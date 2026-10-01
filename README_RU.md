# Система сборки Allwinner A333 на базе Buildroot

Проект предназначен для независимой сборки Linux-системы для модуля
Allwinner A333 с платой HelperBoard. Есть три профиля: планшет DMX,
планшет-медиаплеер и устройство без дисплея.

В основе используется Buildroot 2025.02.16. Система собирается с `systemd`,
`glibc` и целевым toolchain AArch64. Vendor-ядро Linux 6.6, BSP, DTS и U-Boot
взяты из исходного Allwinner SDK. Оригинальный 32-битный Linaro toolchain
используется только для сборки vendor U-Boot.

Проект разделён на базовую ОС и отдельный раздел `OEM` для бизнес-логики.
Поддерживается A/B-разметка с отдельными слотами rootfs и OEM, а также
подготовка подписанного RAUC bundle для обновления уже установленной системы.

## Структура проекта

```text
buildroot/                  исходный Buildroot
configs/                    BR2_EXTERNAL, defconfig и настройки платы
overlays/a333/rootfs/       общий слой rootfs
overlays/components/        слои дисплея, DMX, медиа и headless
oem/                        приложения и содержимое OEM-раздела
vendor/allwinner-a333/      kernel, BSP, DTS, U-Boot и pack SDK Allwinner
scripts/                    подготовка boot.img, полного образа и RAUC bundle
Dockerfile                  контейнер окружения сборки
docker-build.sh             wrapper для Docker-сборки
build.sh                    точка входа для сборки профилей
profiles/*.conf             defconfig, списки overlay и пути вывода
output/                     результат сборки, игнорируется Git
dl/, ccache/, toolchains/   локальные кэши и toolchain, игнорируются Git
```

## Сборка

Нужен Docker Engine, но не Docker Compose. Команды ниже выполняются из корня
репозитория. 32-битный toolchain для vendor U-Boot **не хранится в Git**:
перед первой сборкой поместите
`toolchains/gcc-linaro-7.2.1-2017.11-x86_64_arm-linux-gnueabi/` так, чтобы
`bin/arm-linux-gnueabi-gcc` внутри него был исполняемым. Каталоги исходников
и упаковщика в `vendor/allwinner-a333/` также должны присутствовать.

### Первая сборка после клонирования

```sh
./docker-build.sh build
./docker-build.sh prepare-sources
./build.sh list
./build.sh dmx configure
./build.sh dmx build
```

Вместо `dmx` можно выбрать `media` или `headless`. Каждый профиль собирается
в собственном `output/profiles/<profile>/`. Если `.config` ещё нет, команда
`build` сама выполнит `configure`; выше шаг показан отдельно для ясности.

### Чистая пересборка одного профиля

Чтобы удалить результаты компиляции, rootfs и образы профиля `dmx`, но
сохранить его `.config` с изменениями из menuconfig:

```sh
./build.sh dmx clean
./build.sh dmx build
```

Чтобы дополнительно вернуться к исходному defconfig профиля:

```sh
./build.sh dmx clean
./build.sh dmx configure
./build.sh dmx build
```

`clean` затрагивает только результаты выбранного профиля. Каталоги `dl/`
(загруженные исходники и подготовленные vendor-архивы), `ccache/`,
неотслеживаемый Git toolchain в `toolchains/` и ключи подписи в `keys/`
сохраняются. При обычной пересборке не нужно пересоздавать Docker-образ и
повторять `prepare-sources`. Не используйте для этого Buildroot `distclean`:
он также удаляет `.config` и другие файлы конфигурации. Если изменился набор
overlay, выполните `clean` перед сборкой в том же каталоге вывода либо
задайте новый `OUTPUT_DIR` в профиле.

Другие полезные команды:

```sh
./build.sh media build
./build.sh headless build
./build.sh media menuconfig
```

В `profiles/*.conf` задаются defconfig, каталог вывода, имя RAUC-пакета и
упорядоченные массивы `ROOTFS_OVERLAYS` и `OEM_OVERLAYS`. Пути в них указаны
от корня проекта. Слои rootfs копирует Buildroot, а слои OEM — post-build
скрипт в отдельный A/B-раздел. Более поздний слой перекрывает файлы раннего.
Например:

```sh
ROOTFS_OVERLAYS=(overlays/a333/rootfs overlays/components/display/rootfs overlays/components/media/rootfs)
OEM_OVERLAYS=(oem/a333/common-rootfs oem/a333/media-rootfs)
```

Для другого набора слоёв используйте отдельный `OUTPUT_DIR`: инкрементальный
`target/` Buildroot не удаляет файлы, исчезнувшие из rootfs overlay. Команда
`./build.sh <profile> configure` переносит выбранный список в `.config`.
Дополнительные переменные можно задать в `PROFILE_ENV=("NAME=value")`.
Результаты лежат в `output/profiles/<profile>/`. Старый прямой способ с
`output/` остаётся.
На хосте `build.sh` автоматически вызывает `docker-build.sh`. Его можно
запускать и из `/workspace` внутри интерактивной Docker-сессии.

Последний post-image шаг автоматически создаёт `boot.img`, A/B payload-файлы,
RAUC bundle и полный vendor-образ через скопированный из оригинального SDK
упаковщик `dragon`. Для повторной упаковки без пересборки Buildroot можно
запустить внутри контейнера:

```sh
./docker-build.sh /workspace/scripts/pack-a333-image.sh /workspace/output/images
```

Временные данные упаковщика находятся в `output/images/.a333-pack/` и
игнорируются Git.

Для изменения параметров Buildroot:

```sh
./build.sh dmx menuconfig
```

Скачанные исходники и кэш компилятора сохраняются в `dl/` и `ccache/` внутри
проекта. Это ускоряет повторные сборки и не зависит от версии Ubuntu на хосте.

## Готовые артефакты

После успешной сборки файлы находятся в:

```text
output/profiles/<profile>/images/
```

Основные файлы:

| Файл | Назначение |
|---|---|
| `u-boot.bin` | vendor U-Boot для первичной установки |
| `board.dtb` | device tree платы |
| `Image.A`, `Image.B` | копии kernel Image для слотов A/B |
| `rootfsA.ext4`, `rootfsB.ext4` | базовая ОС для rootfs-слотов A/B |
| `oemA.ext4`, `oemB.ext4` | OEM-разделы A/B |
| `boot.img` | kernel + DTB для vendor packer |
| `a333-helperboard-full.img` | полный vendor-образ для первой прошивки eMMC |
| `a333-helperboard-sys_partition.fex` | A/B-карта, использованная при упаковке |
| `a333-<profile>.raucb` | подписанный пакет обновления RAUC |

У всех профилей отдельные идентификаторы совместимости RAUC, а rootfs/OEM
сохраняют A/B-слоты. OEM формируется отдельно внутри каталога вывода каждого
профиля, чтобы приложения разных устройств не попадали в чужие образы.

Профиль `media` принимает A2DP, включает HFP hands-free через BlueALSA,
MPD для радио, локальных файлов и NFS, а также базовый сенсорный интерфейс
с метаданными AVRCP. Радиостанции записываются в
`/userdata/media/radio.m3u`, музыка — в `/userdata/media/music`.
Кнопка **Music** обновляет базу MPD, наполняет очередь и запускает музыку.
**Pair BT** открывает окно сопряжения на две минуты при наличии `hci0`.
Штатные V+/V- регулируют общую громкость кодека для музыки и Bluetooth-аудио.
Драйвер Linux считывает резистивную цепочку GPADC и выдаёт через evdev
`KEY_VOLUMEUP` (115) и `KEY_VOLUMEDOWN` (114). Медиасервис синхронно меняет
`DACL Volume` и `DACR Volume`, ограничивая максимум уровнем 0 дБ (регистр 159).
Верхняя плашка исчезает через пять секунд после последнего нажатия. GPADC
с настроенными кнопками остаётся включённым, в том числе после простоя.
Шаг регулировки — 3 дБ, диапазон −60…0 дБ и отдельное отключение звука.

`a333-audio-init.service` один раз при загрузке настраивает аналоговый выход
и умеренную громкость, включая LINEOUT Gain (значения 0/1 отключают звук).
После ручных изменений микшера восстановить звук можно командой
`a333-audio-init --force`. MPD и Bluetooth используют общий ALSA `plug`/`dmix`
на 48 кГц с преобразованием других частот. Патч кодека убирает заявленные,
но не настроенные драйвером частоты, например 88,2 кГц.
Работают только профильные `bluealsad`/`bluealsa-playback`; дублирующие
стандартные службы BlueALSA и активация MPD через сокет замаскированы.

Драйвер AIC8800 собирается с управлением питанием и сканированием SDIO для
Allwinner; прошивки находятся в `/vendor/etc/firmware`.
`a333-bluetooth-uart.service` разблокирует оба Bluetooth rfkill и подключает
`/dev/ttyS1` как H4 на 1500000 бод с RTS/CTS после загрузки прошивки через SDIO.
NetworkManager использует `wpa_supplicant.service` с поддержкой D-Bus и EAP.
Без EAP запрос свойства `EapMethods` завершается ошибкой, и Wi-Fi не стартует.
Проверка после старта: `nmcli device status`,
`nmcli device wifi list ifname wlan0`, `bluetoothctl show` и
`systemctl status a333-bluetooth-uart`.
Если `hci0` нет, сначала проверьте ошибки включения AIC Wi-Fi/SDIO в `dmesg`.
Передача обложки с телефона, маршрутизация микрофона HFP, автомонтирование
USB и обозреватель сетевых ресурсов **пока не реализованы и не проверены на ЦУ**.

Профиль `headless` отключает в Linux DTB дисплей, DSI/LVDS, подсветку и тач.
Ранний заводской загрузчик всё ещё может включать дисплей; перед использованием
контактов разъёма как GPIO надо проверить разводку и допустимые напряжения.

Во всех профилях есть BLE GATT-настройка Wi-Fi. На устройстве администратор
выполняет `a333-ble-pairing on`, сопрягает телефон и записывает JSON
`{"ssid":"...","password":"..."}` в характеристику
`e591cbb7-4d2a-4a04-aa78-209eb9b4ca33`, затем выполняет
`a333-ble-pairing off`. BLE-канал шифруется, но Just Works не защищает от
атаки посредника; первоначальную настройку проводите в доверенном месте.
Окно сопряжения автоматически закрывается через пять минут.

В OEM-раздел сейчас входит демонстрационное приложение `dmx-panel`,
перенесённое из Luckfox Pico Panel86 и адаптированное для framebuffer
1280×800. Исходники находятся в
[`oem/a333/src/dmx-panel`](oem/a333/src/dmx-panel), а после post-build файл
попадает в `/oem/usr/bin/dmx-panel`.

При загрузке systemd автоматически запускает демо с параметрами
`--simulate --allow-missing-input`. Это режим проверки дисплея и интерфейса:
виртуальные RDM-устройства используются из приложения, реальный RS485/UART не
открывается.

## Доступ к устройству

Основная учётная запись:

```text
Пользователь: root
Пароль:       allwinner
```

### SSH, SCP и SFTP

В образ включён сервер OpenSSH с входом root по паролю. После получения
IP-адреса платы:

```sh
ssh root@<IP_ПЛАТЫ>
scp local-file root@<IP_ПЛАТЫ>:/userdata/
sftp root@<IP_ПЛАТЫ>
```

SSH host-ключи создаются при первом запуске в persistent `/var/lib/ssh`.
Они переживают обновления A/B, но пересоздаются при factory reset.

### ADB shell

`adbd` одновременно доступен через USB FunctionFS и TCP-порт 5555. Для USB:

```sh
output/host/bin/adb devices
output/host/bin/adb shell
```

Для подключения по Ethernet или Wi-Fi:

```sh
output/host/bin/adb connect <IP_ПЛАТЫ>:5555
output/host/bin/adb shell
```

Эта версия `adbd` запускает root shell без парольной авторизации ADB, поэтому
порт 5555 следует использовать только в доверенной сети.

Состояние сервисов проверяется командами:

```sh
systemctl status sshd adbd NetworkManager bluetooth
journalctl -u sshd -u adbd -u NetworkManager -u bluetooth
ls /sys/class/udc
cat /sys/kernel/config/usb_gadget/a333/functions/ffs.adb/ready
```

## Управление сетью

Ethernet и Wi-Fi управляются NetworkManager. Старые параллельные сервисы
`systemd-networkd` и `wpa_supplicant@wlan0` не запускаются. Основные команды:

```sh
nmcli general status
nmcli device status
nmcli connection show
nmcli device wifi list
nmcli device wifi connect '<SSID>' password '<ПАРОЛЬ>' ifname wlan0
nmcli connection up '<ИМЯ_ПОДКЛЮЧЕНИЯ>'
```

Профили NetworkManager хранятся в persistent `/var/lib/NetworkManager` и
сохраняются при обновлении A/B. Factory reset удаляет их вместе с остальным
содержимым `userdata`.

## Управление Bluetooth

В образ входят `bluetoothd`, `bluetoothctl`, диагностический `btmon`,
дополнительные BlueZ tools и `rfkill`. Пример поиска и подключения:

```sh
rfkill unblock bluetooth
systemctl enable --now bluetooth
bluetoothctl
power on
agent on
default-agent
scan on
pair <MAC>
trust <MAC>
connect <MAC>
```

## Карта памяти и первичная прошивка платы

RAUC bundle не предназначен для первичной прошивки пустой платы. На момент
первой установки ещё нет работающей Linux-системы, RAUC, U-Boot A/B-логики и
настроенных разделов.

В проект скопированы карты из оригинального SDK:

- [`sys_partition-vendor-dragonboard-8G.fex`](configs/boards/a333/helperboard-a333/sys_partition-vendor-dragonboard-8G.fex) — исходная однократная карта для 8G;
- [`sys_partition-vendor-dragonboard-16G.fex`](configs/boards/a333/helperboard-a333/sys_partition-vendor-dragonboard-16G.fex) — исходная однократная карта для 16G;
- [`sys_partition-ab.fex`](configs/boards/a333/helperboard-a333/sys_partition-ab.fex) — карта этого проекта с A/B-разделами.

В A/B-карте разделы имеют номера: `p1` boot-resource, `p2` env, `p3`
env-redund, `p4/p5` bootA/bootB, `p6/p7` rootfsA/rootfsB, `p8/p9` oemA/oemB.
Размеры в `sys_partition-ab.fex` указаны в секторах по 512 байт, кроме
`mbr.size`, который задаётся в килобайтах.

Для первой прошивки используется именно:

```text
output/images/a333-helperboard-full.img
```

Это единый vendor-образ, собранный средствами оригинального Allwinner SDK и
содержащий boot package, карту A/B, env, bootA/bootB, rootfsA/rootfsB и
oemA/oemB. Его нужно передать штатному Allwinner-флешеру, рекомендованному
производителем платы (PhoenixSuit/LiveSuit или совместимому варианту).
Образ перезаписывает начало eMMC и рассчитан на текущую A/B-карту; перед
прошивкой необходимо проверить фактический объём eMMC и сохранить данные.

Отдельные `rootfs*.ext4`, `oem*.ext4`, `boot.img` и `.fex` полезны для
диагностики и ручной прошивки, но не заменяют полный образ при первой
установке. `a333-helperboard.raucb` на пустую плату записывать нельзя.

## Проверка FEL и sunxi-fel

На A333 устройство определяется в FEL, например:

```sh
sudo sunxi-fel --list --verbose
sudo sunxi-fel -v ver
```

Для этой платы ожидается SoC ID `0x1919` и строка вида `AWUSBFEX
soc=00001919`. Предупреждение `no 'soc_sram_info' data for your SoC` означает,
что установленная upstream-версия `sunxi-tools` ещё не знает SRAM-карту A333.

Для загрузки FES используется собранный A333-aware `xfel`, а не обычная
upstream-версия `sunxi-fel`:

```sh
sudo ../xfel/xfel version
```

Скрипт FEL-этапа:

```sh
scripts/flash-a333-fel.sh --dry-run
sudo scripts/flash-a333-fel.sh --xfel ../xfel/xfel
```

Он выполняет следующую последовательность:

1. загружает `fes1.fex` по адресу `0x0004c000` и запускает его;
2. ждёт повторного появления A333 в FEL после инициализации DRAM;
3. загружает `u-boot.fex` по `0x4a000000`, `sunxi.fex` по `0x4a200000`,
   `config.fex` по `0x4a300000` и `board.fex` по `0x4a380000`;
4. запускает U-Boot с `work_mode=0x10`, то есть в USB EFEX/product mode.

`sunxi.fex` — обязательный DTB/config blob для vendor U-Boot: он ищется по
`CONFIG_SYS_TEXT_BASE + 2 MiB` (`0x4a200000`). Если его не загрузить, в UART
появляются `FDT ERROR` и U-Boot обычно перезагружается до строк `workmode` и
`run usb efex`.

При подготовке U-Boot скрипт также записывает подтверждённый размер DRAM
`2048 MiB` и пересчитывает checksum заголовка. В ручной процедуре это важно:
поле checksum находится по смещению `0x0c`; без его пересчёта FES2 может быть
отброшен и плата возвращается к загрузке со штатного Flash.

`xfel` умеет только FEL-операции с памятью и запуск кода. Он не реализует
протокол USB EFEX и не умеет записывать eMMC, поэтому после последнего `exec`
нужен LiveSuit/PhoenixSuit или другой EFEX-клиент. Для запуска внешней команды
можно передать её скрипту:

После запуска FES1 устройство может продолжать отображаться как `1f3a:efe8`,
но перестаёт отвечать на FEL-запрос `version`. Поэтому локальная сборка
`xfel` дополнена режимом `--no-version`; скрипт использует его только для
загрузки U-Boot, `sunxi.fex`, `config.fex` и `board.fex` после FES1. Если инструмент был
пересобран или заменён, выполните `(cd ../xfel && make)`.

```sh
scripts/flash-a333-fel.sh \
  --efex-command 'sudo ./LiveSuit'
```

После появления USB EFEX в клиенте нужно выбрать:

```text
output/images/a333-helperboard-full.img
```

Если `--efex-command` не задан, скрипт останавливается после handoff и ничего
не записывает в eMMC. В частности, полный `*.img` нельзя передавать в
`xfel write`: это запись в оперативную память, а не в карту памяти. Файлы
`sys_partition-ab.fex` и `sys_partition-vendor-*.fex` являются входными данными
vendor-флешера, а не картой адресов для FEL.

Текущая схема прошивки:

1. первая установка — `a333-helperboard-full.img` через USB EFEX/vendor-флешер;
2. последующие обновления — `a333-helperboard.raucb` из работающей системы;
3. `sunxi-fel`/`xfel` — диагностика и загрузка промежуточных компонентов FEL.

В Ubuntu 26.04 не следует устанавливать старый LiveSuit непосредственно в
систему: его `awusb.ko` и `libpng12` рассчитаны на старую версию ядра и
пользовательского пространства. Документация Allwinner допускает отдельную
Ubuntu 20.04/22.04 среду и сборку `awusb.ko`, но это не устраняет ограничение
`xfel` по EFEX-протоколу.

## Обновление через RAUC

Для обновления уже загруженной системы используется:

```text
output/images/a333-helperboard.raucb
```

Bundle содержит три образа для одной неактивной группы A/B:

- `boot` — ядро и DTB (`boot.img`), bootA/bootB (`p4/p5`);
- `rootfs` — обновление неактивного rootfs-слота;
- `oem` — обновление соответствующего OEM-слота.

Статус RAUC хранится в `/userdata/var/lib/rauc`, вне обновляемых слотов.
Boot0, BL31, SCP, U-Boot и userdata при OTA не заменяются.

На целевой системе обновление запускается командой:

```sh
rauc install /path/to/a333-helperboard.raucb
reboot
```

Custom RAUC backend сопоставляет операции RAUC с переменными Allwinner U-Boot
`systemAB_next`, `systemAB_damage` и `bootcount`. После успешной загрузки
необходимо выполнить health-check приложения и подтвердить новый слот как
рабочий (`rauc status mark-good booted`). Backend использует единственный
vendor-маркер здоровья, а не независимые флаги каждого слота. Автоматический
откат требует отдельно проверенной интеграции bootcount в U-Boot: успешная
перезагрузка после OTA сама по себе не доказывает восстановление после сбоя ядра.

Для старых образов перед первым обновлением новым bundle нужно добавить boot-
слоты в `/etc/rauc/system.conf` и оставить в `/etc/fw_env.config` единственную
строку `/dev/mmcblk0p2 0x0 0x20000`: этот U-Boot не использует redundant-заголовок.
Некоторые старые vendor env также содержат начальный NUL, скрывающий переменные
от `fw_printenv`. Сначала сохраните первые 128 КиБ обоих env-разделов.
`scripts/normalize-a333-env.py INPUT_BACKUP OUTPUT_FILE` проверяет CRC, сохраняет
переменные и создаёт исправленный файл, не записывая его на устройство. Перед
установкой проверьте файл через `fw_printenv`. Не запускайте `fw_setenv` для
невалидного/пустого environment: это может удалить параметры загрузки.
Новые полные образы автоматически создают environment нужного формата.

Проверено на media-плате 2026-10-01: подписанный verity-bundle
`2026.10.01-adb-ota1` установлен через RAUC в bootB/rootfsB/oemB; после
перезагрузки запущен слот B с ядром №9. Контрольная сумма bootA и UUID userdata
не изменились. Проверены служба ADB, конфигурация USB UDC, `adb shell` по TCP и
службы media/Wi-Fi/Bluetooth; слот B подтверждён как good. Искусственный сбой
загрузки и автоматический откат этим тестом не проверялись.

В текущей сборке bundle подписывается автоматически сгенерированным
development-сертификатом из игнорируемой папки `keys/`. Для production нужно
передать контролируемые credentials, доступные внутри контейнера:

```sh
A333_RAUC_KEY=/workspace/keys/prod.key.pem \
A333_RAUC_CERT=/workspace/keys/prod.cert.pem \
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

RAUC bundle обновляет boot (ядро/DTB), rootfs и OEM. Он не изменяет таблицу
разделов, boot-resource или самого U-Boot. Эти компоненты обновляются отдельной
процедурой первичной/vendor-прошивки.

## Текущее состояние

Сборка Buildroot, vendor kernel/BSP, U-Boot, systemd/glibc rootfs, RAUC bundle
и полный `a333-helperboard-full.img` проверены в Docker. До использования на
реальном железе требуется проверить MIPI-панель 1280×800, фактический объём
eMMC, первичную прошивку, переключение A/B и rollback после неуспешной
загрузки.

Дополнительные детали по плате находятся в
[`configs/boards/a333/helperboard-a333/README.md`](configs/boards/a333/helperboard-a333/README.md),
а описание Docker-окружения — в [`DOCKER.md`](DOCKER.md).
