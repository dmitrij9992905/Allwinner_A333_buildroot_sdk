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

## Доступ через ADB

В образ включён `adbd`, запускаемый systemd с ключом `-a` и слушающий TCP-порт
5555. После получения IP-адреса платы на компьютере выполните:

```sh
output/host/bin/adb connect <IP_ПЛАТЫ>:5555
output/host/bin/adb shell
```

На самой плате сервис можно проверить командами:

```sh
systemctl status adbd
systemctl status dmx-panel
journalctl -u dmx-panel -f
```

В текущей конфигурации ADB включён по TCP/IP; доступность порта зависит от
сетевого подключения платы. USB-gadget ADB можно добавить позже после
подтверждения OTG-порта и UDC в финальном DTS.

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

В таком состоянии `sunxi-fel` безопасно использовать для обнаружения платы и
проверки версии FEL, но нельзя применять `sunxi-fel write`, `spl`, `uboot` или
пытаться указывать eMMC-секторы вручную: для A333 отсутствует подтверждённый
upstream FEL-loader и корректная карта адресов. `sys_partition-ab.fex` — это
карта разделов vendor-флешера, а не адресная карта для `sunxi-fel`.

Текущая схема прошивки поэтому такая:

1. первая установка — `a333-helperboard-full.img` через vendor-флешер;
2. последующие обновления — `a333-helperboard.raucb` из работающей системы;
3. `sunxi-fel` — обнаружение и диагностика, пока не появится проверенный
   A333-specific loader.

Не следует прошивать полный `*.img` через `sunxi-fel` или записывать его на
найденное USB-устройство без подтверждения, что это именно eMMC платы.

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
