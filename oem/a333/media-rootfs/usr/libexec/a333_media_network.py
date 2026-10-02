"""Asynchronous NetworkManager/BlueZ adapter for the EEZ media panel.

The public snapshot contains no passwords. Scan indices are revision checked,
so a rescan cannot silently connect to a different network.
"""
import asyncio
import json
import os
import re
import time
from pathlib import Path


def split_nmcli(line):
    fields, field, escape = [], [], False
    for char in line:
        if escape:
            field.append(char)
            escape = False
        elif char == "\\":
            escape = True
        elif char == ":":
            fields.append("".join(field))
            field = []
        else:
            field.append(char)
    if escape:
        field.append("\\")
    fields.append("".join(field))
    return fields


def display_text(value):
    return " ".join(str(value).split())[:180]


class NetworkController:
    def __init__(self, run, bus):
        self.run = Path(run)
        self.bus = bus
        self.networks = []
        # A backend restart must invalidate the UI's old scan indices too.
        self.revision = time.monotonic_ns()
        self.busy = False
        self.message = "Press Scan to find Wi-Fi networks"
        self.wifi = self.ethernet = self.bluetooth = False
        self.operation = None

    def publish(self):
        labels = [display_text(row["ssid"]) + "  [" +
                  display_text(row["security"] or "Open") + ", " +
                  str(row["signal"]) + "%]" for row in self.networks]
        lines = [str(self.revision), str(int(self.wifi)), str(int(self.ethernet)),
                 str(int(self.bluetooth)), str(int(self.busy)), display_text(self.message), *labels]
        temporary = self.run / "network.new"
        temporary.write_text("\n".join(lines) + "\n", encoding="utf-8")
        os.replace(temporary, self.run / "network")

    async def nmcli(self, *arguments, timeout=15):
        process = await asyncio.create_subprocess_exec(
            "/usr/bin/nmcli", "--colors", "no", "--terse", "--escape", "yes",
            *arguments, stdout=asyncio.subprocess.PIPE, stderr=asyncio.subprocess.PIPE,
            env={**os.environ, "LC_ALL": "C"})
        try:
            output, error = await asyncio.wait_for(process.communicate(), timeout)
        except (asyncio.TimeoutError, asyncio.CancelledError):
            process.kill()
            await process.communicate()
            raise
        if process.returncode:
            raise RuntimeError(error.decode("utf-8", "replace").strip() or "NetworkManager command failed")
        return output.decode("utf-8", "replace")

    async def refresh(self):
        try:
            data = await self.nmcli("--fields", "DEVICE,TYPE,STATE", "device", "status")
            devices = [split_nmcli(line) for line in data.splitlines()]
            self.wifi = any(len(row) == 3 and row[1] == "wifi" and row[2] == "connected" for row in devices)
            self.ethernet = any(len(row) == 3 and row[1] == "ethernet" and row[2] == "connected" for row in devices)
        except (OSError, RuntimeError, asyncio.TimeoutError):
            self.wifi = self.ethernet = False
        try:
            from dbus_next import Message, MessageType
            reply = await asyncio.wait_for(self.bus.call(Message(destination="org.bluez", path="/",
                interface="org.freedesktop.DBus.ObjectManager", member="GetManagedObjects")), 3)
            self.bluetooth = reply.message_type == MessageType.METHOD_RETURN and any(
                props.get("org.bluez.Device1", {}).get("Connected") and
                props["org.bluez.Device1"]["Connected"].value for props in reply.body[0].values())
        except Exception:
            self.bluetooth = False
        self.publish()

    async def scan(self):
        await self.nmcli("radio", "wifi", "on")
        result = await self.nmcli("--fields", "SSID,SIGNAL,SECURITY,DEVICE,BSSID",
                                  "device", "wifi", "list", "--rescan", "yes", timeout=25)
        networks = {}
        for line in result.splitlines():
            row = split_nmcli(line)
            if len(row) != 5 or not row[0] or not re.fullmatch(r"(?:[0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}", row[4]):
                continue
            try:
                signal = max(0, min(100, int(row[1])))
            except ValueError:
                continue
            candidate = dict(ssid=row[0], signal=signal,
                             security="" if row[2] == "--" else row[2],
                             device=row[3], bssid=row[4])
            # Prefer wlan0 over the vendor's additional p2p0 station interface.
            previous = networks.get(row[0])
            score = (row[3] == "wlan0", signal)
            if not previous or score > (previous["device"] == "wlan0", previous["signal"]):
                networks[row[0]] = candidate
        self.networks = sorted(networks.values(), key=lambda row: -row["signal"])[:64]
        self.revision += 1
        self.message = f"Found {len(self.networks)} networks. Select a network and enter its password."

    async def connect(self, request):
        index, revision = request.get("index"), request.get("revision")
        password = request.get("password", "")
        if type(index) is not int or type(revision) is not int or revision != self.revision or not 0 <= index < len(self.networks):
            raise ValueError("Network list changed. Scan and select the network again.")
        if not isinstance(password, str) or len(password.encode("utf-8")) > 128 or "\x00" in password:
            raise ValueError("Invalid Wi-Fi password")
        row = self.networks[index]
        if "802.1X" in row["security"]:
            raise ValueError("Enterprise Wi-Fi requires a NetworkManager profile (nmcli).")
        # BSSID is validated; no shell is invoked and no SSID becomes an option.
        args = ["--wait", "30", "device", "wifi", "connect", row["bssid"], "ifname", row["device"]]
        if password:
            args += ["password", password]
        self.message = "Connecting to " + display_text(row["ssid"]) + "..."
        self.publish()
        try:
            await self.nmcli(*args, timeout=35)
        except RuntimeError as error:
            # Do not allow a CLI diagnostic to echo the supplied password.
            raise RuntimeError(str(error).replace(password, "<redacted>") if password else str(error)) from None
        finally:
            request.pop("password", None)
        self.message = "Connected to " + display_text(row["ssid"])

    async def execute(self, request):
        self.busy = True
        self.message = "Scanning Wi-Fi..." if request["command"] == "wifi_scan" else "Connecting..."
        self.publish()
        try:
            if request["command"] == "wifi_scan":
                await self.scan()
            else:
                await self.connect(request)
        except ValueError as error:
            self.message = display_text(error)
        except (OSError, RuntimeError, asyncio.TimeoutError):
            self.message = "Wi-Fi operation failed. Check password, signal and NetworkManager."
        finally:
            request.pop("password", None)
            self.busy = False
            await self.refresh()

    def handle(self, message):
        try:
            request = json.loads(message)
            if not isinstance(request, dict) or request.get("command") not in ("wifi_scan", "wifi_connect"):
                return
        except (ValueError, UnicodeError):
            return
        if self.operation and not self.operation.done():
            return
        self.operation = asyncio.create_task(self.execute(request))

    async def poll(self):
        while True:
            await self.refresh()
            await asyncio.sleep(3)
