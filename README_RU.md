# Система сборки Allwinner A333 на базе Buildroot

Проект предназначен для независимой сборки Linux-системы для модуля
Allwinner A333 с платой HelperBoard и MIPI-дисплеем 1280×800.

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
overlays/                   файлы базовой rootfs
oem/                        приложения и содержимое OEM-раздела
vendor/allwinner-a333/      kernel, BSP, DTS, U-Boot и pack SDK Allwinner
scripts/                    подготовка boot.img, полного образа и RAUC bundle
Dockerfile                  контейнер окружения сборки
docker-build.sh             wrapper для Docker-сборки
output/                     результат сборки, игнорируется Git
dl/, ccache/, toolchains/   локальные кэши и toolchain, игнорируются Git
```

## Сборка

Требуется Docker Engine; Docker Compose Plugin для основного wrapper не обязателен.
Все host-зависимости устанавливаются внутри Docker-образа.

Сначала собрать Docker-образ:

```sh
./docker-build.sh build
```

Подготовить локальные архивы vendor-исходников и применить конфигурацию платы:

```sh
./docker-build.sh prepare-sources
./docker-build.sh make BR2_EXTERNAL=../configs O=../output a333_helperboard_defconfig
```

Запустить сборку:

```sh
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

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
./docker-build.sh make BR2_EXTERNAL=../configs O=../output menuconfig
```

Скачанные исходники и кэш компилятора сохраняются в `dl/` и `ccache/` внутри
проекта. Это ускоряет повторные сборки и не зависит от версии Ubuntu на хосте.

## Готовые артефакты

После успешной сборки файлы находятся в:

```text
output/images/
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
| `a333-helperboard.raucb` | подписанный пакет обновления RAUC |

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

Bundle содержит два образа:

- `rootfs` — обновление неактивного rootfs-слота;
- `oem` — обновление соответствующего OEM-слота.

На целевой системе обновление запускается командой:

```sh
rauc install /path/to/a333-helperboard.raucb
reboot
```

Custom RAUC backend сопоставляет операции RAUC с переменными Allwinner U-Boot
`systemAB_next`, `systemAB_damage` и `bootcount`. После успешной загрузки
необходимо выполнить health-check приложения и подтвердить новый слот как
рабочий. При ошибке загрузки U-Boot должен вернуть систему на предыдущий слот.

В текущей сборке bundle подписывается автоматически сгенерированным
development-сертификатом из игнорируемой папки `keys/`. Для production нужно
передать контролируемые credentials, доступные внутри контейнера:

```sh
A333_RAUC_KEY=/workspace/keys/prod.key.pem \
A333_RAUC_CERT=/workspace/keys/prod.cert.pem \
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

RAUC bundle обновляет rootfs и OEM. Он не предназначен для изменения таблицы
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
