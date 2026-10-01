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

### Варианты логирования ядра

| Обычный профиль | Профиль с подробной отладкой ядра |
|---|---|
| `dmx` | `dmx-kernel-debug-logs` |
| `media` | `media-kernel-debug-logs` |
| `headless` | `headless-kernel-debug-logs` |

В обычных профилях отключены отладочные сообщения vendor BSP и трассировка
initcall. Из параметров ядра убраны `earlyprintk`, `ignore_loglevel` и
`keep_bootcon`; `loglevel=5` оставляет предупреждения и ошибки на UART.
`printk`/`dmesg`, вход через UART и диагностические интерфейсы сохраняются.
Debug-варианты сохраняют прежнюю подробную конфигурацию и параметры загрузки.
Это уменьшает затраты на вывод в UART, но ускорение нужно измерять на плате:
задержки пробинга драйверов сами по себе этим не устраняются.

```sh
./build.sh media build                         # обычное логирование
./build.sh media-kernel-debug-logs build       # подробная отладка ядра
```

У каждого варианта собственные defconfig, каталог вывода и имя `.raucb`.
Debug-профили наследуют набор rootfs/OEM overlay своего продукта. Поле
`KERNEL_LOGGING=quiet` или `debug` выбирает фрагменты ядра и U-Boot environment.
`build.sh` синхронизирует также существующие `.config` и при изменении политики
сбрасывает только конфигурационные/сборочные stamp-файлы ядра и U-Boot, чтобы
инкрементальная сборка не оставляла старую конфигурацию ядра.

Обычная/debug-пара имеет одинаковый RAUC compatible ID и A/B-разметку.
Подписанный bundle-hook меняет только параметры логирования U-Boot после
записи всех трёх образов слота, не меняя выбор A/B и разметку. Поэтому переход
возможен через OTA без полной прошивки. Политика логирования глобальна для
U-Boot, а не хранится отдельно для слотов: ручное переключение слота не
восстанавливает старую политику. Полный образ также содержит выбранный
environment. Для старых образов сначала выполните миграцию environment,
описанную в разделе RAUC ниже.

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

## Разработка media-panel в VSCode

`oem/a333/src/media-panel/CMakeLists.txt` — CMake-проект приложения. Buildroot
собирает его через `cmake-package`. Оба приложения линкуются с общей статической
библиотекой `panel-common` в `oem/a333/src/panel-common/`: framebuffer/G2D,
evdev, порт LVGL и работа со шрифтами. Копия LVGL 9.5.0 и Roboto с лицензиями
теперь также лежат там. DMX сохраняет Makefile и подключает `common.mk`, media
подключает CMake-цель библиотеки. Приложения не берут общий код друг из друга
или из внешнего SDK поставщика/Luckfox.

```text
oem/a333/src/
├── panel-common/  # include/, src/, vendor/lvgl/, assets/fonts/
├── dmx-panel/     # DMX/RDM и специфичный DMX-интерфейс
└── media-panel/   # интерфейс магнитолы и CMake-проект
```

1. Один раз соберите прошивку `media` либо `media-kernel-debug-logs`, соответствующую
   устройству: нужны её AArch64/glibc toolchain и sysroot. Если они уже есть,
   ради разработки UI всю прошивку пересобирать не нужно.
2. Откройте **корень репозитория** в VSCode (`code .`). Установите рекомендуемое
   расширение C/C++. CMake Tools необязателен: готовые задачи используют Docker,
   установленный на хосте CMake не нужен. Нужны Python 3 и OpenSSH `ssh`/`scp`.
3. **Ctrl+Shift+B** собирает только приложение. Выберите профиль; бинарник
   появится в `output/dev/media-panel/<profile>/media-panel`.
4. **Terminal → Run Task → media-panel: run (build + deploy + logs)** запускает
   полный цикл. Выберите профиль и адрес SSH (по умолчанию `root@192.168.0.144`).
   Пароль root — `allwinner`, если не настроены SSH-ключи.

Задача инкрементально собирает приложение, загружает его в
`/userdata/dev/media-panel/`, останавливает только `a333-media.service` и
запускает тестовый UI с stdout/stderr в терминале SSH. MPD, Bluetooth и
`a333-media-backend.service` продолжают работать. **Ctrl+C** завершает тест
и возвращает штатный UI, если он был активен. Возврат также предусмотрен при
обрабатываемом разрыве SSH; после аварийного завершения можно выполнить
`systemctl start a333-media.service` на устройстве.
Штатный `/oem/usr/bin/media-panel`, A/B-слоты и автозапуск не изменяются.
Тестовые бинарники остаются в userdata.

Те же действия из терминала без VSCode:

```sh
./scripts/media-panel-dev.sh build media
./scripts/media-panel-dev.sh deploy media root@192.168.0.144
./scripts/media-panel-dev.sh run media root@192.168.0.144
# Все три шага одной командой:
./scripts/media-panel-dev.sh cycle media root@192.168.0.144
# Логи установленного UI и backend:
./scripts/media-panel-dev.sh logs media root@192.168.0.144
```

SSH alias, порт и ключ задаются в хостовом `~/.ssh/config`. Чтобы не вводить
пароль несколько раз, используйте `ssh-copy-id root@192.168.0.144`.
Пароль не сохраняется в настройках VSCode. Для SCP используется `-O`, поэтому
SFTP-сервер на устройстве не требуется.

По умолчанию используется `RelWithDebInfo`, включены сообщения приложения
о старте, дисплее/таче и отправке команд backend; GDB не требуется.
При необходимости задайте `A333_DEV_BUILD_TYPE=Debug` или
`A333_DEV_LVGL_LOGGING=ON` перед запуском VSCode/скрипта. Параметры CMake:
`MEDIA_PANEL_DISPLAY_ROTATION` (0/90/180/270), `MEDIA_PANEL_LOGGING` и
`MEDIA_PANEL_LVGL_LOGGING` и `MEDIA_PANEL_G2D` (по умолчанию ON).
Скрипт берёт поворот из `.config` профиля либо
из defconfig. Логи приложения не зависят от debug-варианта ядра.

Для IntelliSense выберите `media` либо `media-kernel-debug-logs` через
**C/C++: Select a Configuration**. CMake создаёт `compile_commands.json`, а
скрипт — `compile_commands.host.json` с путями хоста, включая пути с пробелами
и исходники общей библиотеки. Создать базу без компиляции можно командой
`./scripts/media-panel-dev.sh configure media`.
Явные пути include/defines также покрывают заголовки и DMX-файлы, отсутствующие
в базе. Для DMX добавлены конфигурации редактора `dmx` и
`dmx-kernel-debug-logs` с компиляторами соответствующих профилей. Запасная
конфигурация предполагает поворот 90°; записи media в compilation database
содержат реальные параметры сборки. После изменения поворота пересоздайте
базу; для DMX поправьте запасной define `PANEL_DISPLAY_ROTATION` под свой профиль.
Если после переноса остались красные подчёркивания, выполните **C/C++: Reset
IntelliSense Database**, затем **Developer: Reload Window**. Нужно расширение
`ms-vscode.cpptools` из рекомендаций проекта. Для `.h` выбран язык C,
семантическая подсветка включена.
Исходники редактируйте в `oem/a333/src/`, не в временных копиях `output/.../build/`.

Linux-заголовки не берутся с хоста. Dev-скрипт экспортирует ARM64 UAPI
командой `headers_install` из **собранного ядра выбранного профиля** в
`output/dev/kernel/<профиль>/headers/include`. Эти заголовки входят и в
команды dev-компиляции, и в IntelliSense; libc берётся из целевого sysroot.
Внутренние `include/linux` ядра не подмешиваются в приложения.

Для работы с самим ядром выполните после его сборки:

```bash
bash scripts/kernel-vscode.sh media
```

В VSCode выберите **C/C++: Select a Configuration → kernel-media**.
Аналогичные `kernel-<профиль>` доступны для всех шести профилей.
Используются реальные команды Kbuild, `.config`, сгенерированные ARM64
заголовки и `bsp/include` именно `output/profiles/<профиль>/build/linux-custom`.
Системные заголовки хоста отключены через `-nostdinc`. После пересборки или
смены ядра обновите базу этой командой либо задачей
**SDK kernel: refresh headers and IntelliSense**. Несобранные драйверы могут
потребовать включения в конфигурацию ядра, чтобы получить точную команду Kbuild.

### EEZ-Studio: цвет и FPS

Используется **LVGL 9.5.0**, `LV_COLOR_DEPTH=32`, буфер дисплея
`LV_COLOR_FORMAT_XRGB8888` (4 байта на пиксель, RGB по 8 бит; X не используется).
Для проекта LVGL в EEZ-Studio выбирайте ветку **9.x**, глубину цвета **32 bit**,
если такая настройка доступна; изображения — **XRGB8888** без прозрачности
или **ARGB8888** с прозрачностью. RGB565 нужен только существующему canvas
цветового колеса DMX, а не всему интерфейсу. Логический экран приложения —
1280×800 при штатном повороте 90°/270° (800×1280 при 0°/180°);
поворот физического framebuffer выполняет общий порт SDK.
Это параметры рендеринга, не настройка числа линий/формата MIPI.

В обоих приложениях включён встроенный FPS/CPU monitor LVGL в правом нижнем
углу. Он считает обновления LVGL, а не физическую частоту развёртки дисплея;
на статическом экране FPS может быть низким. CPU — оценка занятости обработчика
LVGL, не общая загрузка всех ядер Linux. Для отключения в dev-сборке:
`A333_DEV_PERF_MONITOR=OFF ./scripts/media-panel-dev.sh build media`.
Для CMake — `-DMEDIA_PANEL_PERF_MONITOR=OFF`, для DMX Make — `PERF_MONITOR=0`.

Для отдельного SDK выполните `./build.sh media build sdk`, распакуйте
`*_sdk-buildroot.tar.gz` из `output/profiles/media/images/` в другое место и
запустите там `relocate-sdk.sh`. Команда
`A333_SDK_DIR=/абсолютный/путь/к/sdk ./scripts/media-panel-dev.sh build media`
использует CMake/компилятор SDK на хосте вместо Docker. При смене Docker на
нативный SDK нужен **свежий каталог dev-сборки**: например, сначала переименуйте
`output/dev/media-panel/media`, поскольку CMake кэширует абсолютные пути.
Если исходного toolchain профиля больше нет, укажите компилятор экспортированного
SDK в настройке C/C++ compiler path. Можно также вызвать CMake напрямую:

```sh
cmake -S oem/a333/src/media-panel -B output/dev/media-panel-native \
  -DCMAKE_TOOLCHAIN_FILE=/absolute/path/to/sdk/share/buildroot/toolchainfile.cmake \
  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMEDIA_PANEL_DISPLAY_ROTATION=90
cmake --build output/dev/media-panel-native --parallel
```

Чтобы изменения попали в штатный OEM-образ и RAUC-bundle:

```sh
./build.sh media build media-panel-rebuild
./build.sh media build
```

Старый Makefile-пакет автоматически синхронизируется и переконфигурируется
при первой сборке после перехода на CMake. Dev-сборка и загрузка бинарника
сами по себе не создают и не устанавливают обновление прошивки.

### Аппаратный вывод через Allwinner G2D (DMX и media)

Оба приложения используют общий G2D-адаптер вывода через DMA-BUF. Виджеты LVGL
пока рисует CPU, а аппаратный **ротатор G2D RCQ** заменяет дорогой программный
поворот/копирование готового кадра в `/dev/fb0`. Это не NXP `LV_USE_G2D`.
Mali, Xorg, закрытые библиотеки поставщика и `/dev/mem` для этого не нужны.

Общий фрагмент ядра включает `CONFIG_AW_G2D=y`, RCQ, ротатор и DMA heaps.
Приложение один раз экспортирует framebuffer через `FBIOGET_DMABUF`, выделяет
повторно используемый DMA-буфер источника, синхронизирует CPU-записи через
`DMA_BUF_IOCTL_SYNC` и ждёт завершения G2D до подтверждения flush в LVGL.
Одно последовательное копирование canvas остаётся: это ещё не zero-copy.

Переменные среды для обоих приложений:

- `A333_PANEL_RENDERER=auto` — по умолчанию. Использовать G2D, если возможно;
  иначе вывести причину и перейти на программный вывод. После аппаратной
  ошибки повторных попыток в этом процессе нет, чтобы избежать зависаний
  на повторяющихся таймаутах и потока одинаковых сообщений.
- `A333_PANEL_RENDERER=software` — старый программный вывод для сравнения.
- `A333_PANEL_RENDERER=g2d` — строгая диагностика: ошибка G2D не маскируется
  программным выводом. Для штатного запуска рекомендуется `auto`.
- `A333_PANEL_PROFILE=1` — раз в пять секунд **при поступлении новых кадров**
  печатать число кадров, число аппаратных кадров, среднее/максимальное время
  вывода. Замер включает копирование источника, но не отрисовку LVGL и не
  всю задержку реакции на кнопку. Первый максимум может включать инициализацию.

В dev-скрипте/VSCode им соответствуют `A333_DEV_RENDERER`,
`A333_DEV_PANEL_PROFILE`, а при сборке — `A333_DEV_G2D=ON|OFF`:

```sh
./scripts/media-panel-dev.sh build media
./scripts/media-panel-dev.sh deploy media root@192.168.0.144
A333_DEV_RENDERER=software A333_DEV_PANEL_PROFILE=1 \
  ./scripts/media-panel-dev.sh run media root@192.168.0.144
# Выйти через Ctrl-C, затем сравнить ту же анимацию на аппаратном выводе:
A333_DEV_RENDERER=g2d A333_DEV_PANEL_PROFILE=1 \
  ./scripts/media-panel-dev.sh run media root@192.168.0.144
```

На ЦУ требуется новое ядро с `/dev/g2d` и `/dev/dma_heap/*`. Локальный патч BSP
также исправляет инициализацию необязательного списка power domains G2D.
В существующей сборке нужно очистить пакет ядра, чтобы применился новый патч:

```sh
./build.sh media build linux-dirclean
./build.sh media build media-panel-rebuild
./build.sh media build
```

Для DMX заменить `media` на `dmx`, а `media-panel-rebuild` на
`dmx-panel-rebuild`. Через RAUC нужно обновить также boot-образ с ядром:
одной загрузки нового бинарника приложения недостаточно. В headless нет
приложения дисплея и обращений к G2D.

Ротатор RCQ сохраняет формат и не масштабирует. Сейчас поддерживаются
непрозрачные ARGB/ABGR 32-битные кадры, stride назначения, кратный 8 байтам,
и масштаб 1:1 после поворота 0/90/180/270. Другие варианты используют CPU.
На первом этапе запись идёт в существующий видимый framebuffer: переключение
страниц и VSync **не добавлены**, поэтому G2D сам по себе не гарантирует
отсутствие разрывов и 60 FPS. Linux-опрос V+/V− и управление ALSA не меняются;
быстрее может стать отображение результата на экране.

Изучение Ubuntu поставщика (только справочная информация, не зависимость
сборки): в `allwinner-a333/source/src/longan/test/dragonboard/baijie_extra`
файл `extra-xfce/etc/X11/xorg.conf` выбирает DRM `modesetting`/`glamor`,
а Qt-вариант выбирает `eglfs_mali` и содержит Valhall r32p0 `libmali`.
Это отдельный GPU-путь, не G2D; конфигурация XFCE ещё не доказывает работу
аппаратного ускорения без логов запуска. Нельзя без проверки смешивать эти
библиотеки с другой версией драйвера Mali. UAPI и ротатор G2D в Longan совпадают
с нашими копиями BSP. Нужная часть ABI скопирована в приложение с исходным
уведомлением о лицензии Linux-syscall и проверяется тестами.

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
