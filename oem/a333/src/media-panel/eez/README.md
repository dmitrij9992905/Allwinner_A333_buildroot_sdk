# EEZ source of truth

Edit `lvgl-media-player.eez-project` in EEZ Studio **0.29.0** (LVGL 9.5.0,
32-bit color, EEZ Flow enabled). Screen layout, bindings, button actions and
navigation are defined in this project, not in handwritten widget creation.
Native actions/variables are implemented in `../media_ui.cpp`; asynchronous
Wi-Fi operations run in `a333_media_network.py` through NetworkManager.

Generated sources and the Studio-supplied MIT eez-framework amalgamation are
committed in `../ui/`. The runtime identifies upstream commit
`10f714f92621fb0bcb2f70b598fdd624f4bb6d81`. No vendor SDK or host Studio installation
is needed to compile the firmware.

From the SDK repository root:

```sh
EEZ_STUDIO=/path/to/EEZ-Studio-0.29.0.AppImage bash scripts/media-panel-eez.sh generate
# Or edit a separate project, then export its generated output:
bash scripts/media-panel-eez.sh import /path/to/lvgl-media-player.eez-project
bash scripts/media-panel-dev.sh build media
```

Keep `wifi_networks` a **non-native Flow global** (`array:string`): Studio 0.29's
native-array code generator is invalid. All other application values are native.
Do not edit `../ui/` manually; regenerate after project changes.

Navigation uses Android Material Shared Axis X-style motion: a 32 dp slide,
fade-through, fast-out-slow-in easing and 300 ms duration. EEZ Flow defines
direction (`OVER_LEFT` to open, `OVER_RIGHT` to return) and duration;
`../media_transition.cpp` refines the visual animation in the native app.
Studio's preview uses its basic screen-slide effect. Repeated navigation
requests are coalesced until the current transition finishes.

Material Symbols assets and their Apache-2.0 license/source attribution are in
`assets/material-icons/`. Connected icons are off-white; disconnected icons are
light grey. The password is masked and cleared after submission; it is not
included in status files or application logs. NetworkManager owns saved Wi-Fi
credentials on persistent userdata. Enterprise/hidden Wi-Fi profiles can be
configured with `nmcli`; the UI covers scanned open/personal networks.
