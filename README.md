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
vendor/allwinner-a333/      kernel, BSP, DTS и U-Boot из Allwinner SDK
scripts/                    подготовка vendor-архивов и RAUC bundle
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
| `a333-helperboard.raucb` | подписанный пакет обновления RAUC |

## Первичная прошивка платы

RAUC bundle не предназначен для первичной прошивки пустой платы. На момент
первой установки ещё нет работающей Linux-системы, RAUC, U-Boot A/B-логики и
настроенных разделов.

Для первой прошивки используется vendor-инструмент Allwinner и комплект
первичных артефактов:

1. создать разметку по
   [`sys_partition-ab.fex`](configs/boards/a333/helperboard-a333/sys_partition-ab.fex);
2. установить `u-boot.bin` и необходимые boot-resource/env-артефакты через
   штатный Allwinner packer/flasher;
3. записать `rootfsA.ext4` в `rootfsA` и `rootfsB.ext4` в `rootfsB`;
4. записать `oemA.ext4` в `oemA` и `oemB.ext4` в `oemB`;
5. подготовить bootA/bootB согласно формату, который ожидает конкретный
   vendor-flasher и текущий U-Boot.

Текущая сборка создаёт отдельные partition payloads, но ещё не создаёт единый
vendor-файл `*.img`/`sys_config.fex` для конкретной прошивочной утилиты. Поэтому
точный формат `bootA`/`bootB`, boot-resource и итогового Allwinner image должен
быть согласован с фактическим flasher платы. Не следует записывать
`a333-helperboard.raucb` напрямую на пустую eMMC.

Разметка в проекте является исходным вариантом для eMMC 16 GiB. Перед первой
прошивкой нужно проверить размеры и номера разделов на реальном модуле.

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

Сборка Buildroot, vendor kernel/BSP, U-Boot, systemd/glibc rootfs и создание
RAUC bundle проверены в Docker. До использования на серийных платах требуется
проверить на реальном железе MIPI-панель 1280×800, номера eMMC-разделов,
первичную прошивку, переключение A/B и rollback после неуспешной загрузки.

Дополнительные детали по плате находятся в
[`configs/boards/a333/helperboard-a333/README.md`](configs/boards/a333/helperboard-a333/README.md),
а описание Docker-окружения — в [`DOCKER.md`](DOCKER.md).
