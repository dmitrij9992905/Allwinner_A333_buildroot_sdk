# DMX Panel demo для Allwinner A333 HelperBoard

Версия приложения: **1.5.0** (файл [`VERSION`](VERSION), также используется
для RAUC bundle). Нативное приложение без оконной системы на **LVGL 9.5.0**: оно рисует
интерфейс напрямую в Linux framebuffer, получает касания через evdev и
непрерывно передаёт полный DMX512 universe через встроенный RS485 UART. В
автозагрузку приложение не добавляется.

## Что уже реализовано

- LVGL-интерфейс для framebuffer 1280 x 800 с вкладками `Сцена` и `RDM`;
  сенсорная рабочая область 720 x 720 центрируется на широкой панели;
- цветовое колесо, яркость, Blackout и выбор стартового канала сцены;
- одна RGB-сцена: три последовательных канала `R`, `G`, `B`, остальные
  DMX-слоты равны нулю;
- непрерывные кадры DMX512, 512 слотов, 250000 бод, 8N2;
- RDM discovery через `DISC_UNIQUE_BRANCH`, `DISC_MUTE` и `DISC_UN_MUTE`;
- чтение `DEVICE_INFO`, производителя, модели, имени устройства, версии ПО и
  состояния Identify;
- `IDENTIFY_DEVICE` GET/SET;
- `DMX_START_ADDRESS` SET;
- контроль UID, transaction number, command class, PID, длины и checksum
  ответа;
- асинхронная очередь RDM: интерфейс остаётся отзывчивым, а DMX передаётся
  отдельным рабочим потоком;
- режим симуляции с двумя виртуальными RDM-устройствами;
- автоматический поиск Goodix touchscreen по имени, без жёсткой привязки к
  `/dev/input/event0`.

Это рабочий первый этап, а не сертифицированный RDM-контроллер. Сейчас
поддерживаются только корневые устройства (`sub-device 0`), до 64 найденных
UID и обычный ответ `ACK`. Обработка `ACK_TIMER`, `ACK_OVERFLOW`, прокси,
sub-device и полный набор PID E1.20 пока не реализованы.

## LVGL 9.5 и Roboto

Официальный LVGL v9.5.0 включён изолированно в
`vendor/lvgl`. Глобальный LVGL из SDK не используется. Источник,
SHA-256 скачанного архива и лицензии записаны в
[`vendor/lvgl/DMX_PANEL_VENDOR.md`](vendor/lvgl/DMX_PANEL_VENDOR.md).

Приложение собрано с настоящим API 9.5 без слоя совместимости со старыми
именами. Используются software renderer, частичный буфер XRGB8888 и отдельный
RGB565 canvas для цветового колеса. Goodix и framebuffer обслуживает локальный
порт приложения, поэтому встроенные SDL/DRM/fbdev/evdev-драйверы LVGL не
подключаются.

Все видимые надписи используют **Roboto Regular v2.138**. Исходный TTF
`assets/fonts/Roboto-Regular.ttf` встраивается в ELF при сборке, а LVGL 9.5
использует его из памяти через TinyTTF. Внешний файл шрифта на модуле
не нужен.

Roboto включён на условиях Apache-2.0; лицензия и уведомление об авторских
правах хранятся рядом с asset в `assets/fonts/LICENSE.txt` и
`assets/fonts/NOTICE.txt`. SHA-256 TTF:
`797e35f7f5d6020a5c6ea13b42ecd668bcfb3bbc4baa0e74773527e5b6cb3174`.

## Исходники

```text
project/app/dmx_panel/
├── include/                 публичные интерфейсы
├── include/lv_conf.h        локальная конфигурация LVGL 9.5
├── assets/fonts/            встраиваемый Roboto и его лицензия
├── vendor/lvgl/             изолированный upstream LVGL v9.5.0
├── src/main.c               CLI и главный LVGL/controller loop
├── src/lvgl_ui.c            виджеты Сцена/RDM и цветовое колесо
├── src/lvgl_port.c          LVGL display/input поверх fbdev/evdev
├── src/lvgl_fonts.c         Roboto из встроенного TTF через TinyTTF
├── src/panel_*.c            framebuffer, canvas и touchscreen helpers
├── src/dmx_controller.c     DMX universe, очередь и RDM-операции
├── src/dmx_transport.c      UART 250000 8N2, BREAK/MAB и окна ответа
├── src/rdm_protocol.c       пакеты, checksum и discovery response
└── tests/                   protocol/controller regression tests
```

Отдельная инструкция по реально активной заставке находится в
[`BOOT_SPLASH_RU.md`](BOOT_SPLASH_RU.md).

## Сборка и тесты на host

В этом проекте приложение собирается как Buildroot-пакет из корня проекта:

```bash
./docker-build.sh make BR2_EXTERNAL=../configs O=../output
```

Вручную пакет можно пересобрать командой `make dmx-panel-rebuild` с теми же
параметрами `O` и `BR2_EXTERNAL`. Исходники приложения находятся в
`oem/a333/src/dmx-panel`, а скомпилированный файл после post-build попадает в
OEM-дерево как `oem/a333/rootfs/usr/bin/dmx-panel`.

Каталог `build/` содержит сгенерированные `.d`-зависимости. Если приложение
переносится в другой путь или контейнер, перед первой сборкой удалите старые
артефакты командой `make -C project/app/dmx_panel clean`, затем запустите сборку
заново. Эта цель не требует установленного cross-toolchain. Если используется
старая копия Makefile, удалите только `build/` вручную и синхронизируйте новый
Makefile перед следующей сборкой. Сам исходный каталог `vendor/lvgl` переносим
вместе с приложением.

Собранный ARM-бинарник в Buildroot:

```text
output/target/usr/bin/dmx-panel
```

Перед упаковкой rootfs post-build переносит его в отдельный OEM-образ.

`make test` проверяет RDM protocol/controller и оставленный regression-test
старого canvas UI. Фактический LVGL 9.5 UI, порт и TinyTTF входят в строгую
ARM-сборку с `-Werror`; compatibility API LVGL намеренно отключён.

В штатном образе systemd запускает `/oem/usr/bin/dmx-panel` с флагами
`--simulate --allow-missing-input`, поэтому демо не открывает RS485.

В текущем checkout несколько ранее созданных generated-каталогов (`config`,
`output`, `project/app/out` и старые результаты `wifi_app`) принадлежат
`nobody:nogroup`. Поэтому общая `./build.sh app` цепочка останавливается на
правах доступа, хотя исходники и ARM target полностью собираются. Ничего
перепрошивать для ручного теста не нужно. При желании проверить стадию copy в
доступный временный каталог:

```bash
make -C project/app/dmx_panel RK_APP_OUTPUT=/tmp/dmx-panel-stage
```

Для восстановления штатной общей сборки следует осознанно вернуть владельца
generated-каталогов или пересоздать их из той же среды, которая создала их
первоначально; исходное дерево SDK для этого менять не требуется.

## Перенос по SSH и запуск вручную

На компьютере:

```bash
cd /home/dmitrij999/Luckfox/sdk
make -C project/app/dmx_panel out/bin/dmx-panel
scp project/app/dmx_panel/out/bin/dmx-panel root@PANEL_IP:/tmp/dmx-panel
```

На A333 через ADB shell, SSH или UART-консоль:

```sh
killall t_s 2>/dev/null || true
killall 86UI_demo 2>/dev/null || true
chmod +x /tmp/dmx-panel
/oem/usr/bin/dmx-panel --simulate --allow-missing-input
```

Режим `--simulate` не открывает UART. В нём можно проверить экран, касания,
Discovery, выбор устройства, Identify и смену адреса до подключения света.
Остановить приложение — `Ctrl-C`.

Запуск с реальной DMX/RDM линией:

```sh
killall t_s 2>/dev/null || true
killall 86UI_demo 2>/dev/null || true
/tmp/dmx-panel \
  --serial /dev/ttyS4 \
  --rs485 auto \
  --break ioctl \
  --controller-uid 7FF0:4C465831
```

Имена framebuffer и touchscreen по умолчанию — `/dev/fb0` и автоматически
найденное устройство с именем `Goodix Capacitive TouchScreen`. Если оси на
конкретной прошивке ориентированы иначе, доступны:

```sh
/tmp/dmx-panel --simulate --swap-xy --invert-x
```

Полная справка не открывает устройства и безопасно работает из консоли:

```sh
/tmp/dmx-panel --help
/tmp/dmx-panel --version
```

Файл в `/tmp` исчезнет после перезагрузки. При сборке firmware приложение
устанавливается в `/oem/usr/bin/dmx-panel`, а init-скрипт `S99lvgl` запускает его
вместо `t_s`/`86UI_demo`. Версия также доступна в
`/oem/usr/share/dmx-panel/VERSION` и командой `/oem/usr/bin/dmx-panel --version`.

Для передачи через одну UART-консоль в rootfs есть ZMODEM `rz/sz`. Например,
в shell выполнить `cd /tmp && rz -y`, затем в minicom выбрать отправку ZMODEM
и локальный файл `dmx-panel`. После передачи остаются те же `chmod` и команды
запуска. Если нужен файл, переживающий reboot и переключение A/B, его можно
положить в `/userdata/dmx-panel`, по-прежнему не добавляя автозагрузку.

Остановить приложение и вручную вернуть штатный UI без reboot:

```sh
killall dmx-panel 2>/dev/null || true
/etc/init.d/S99lvgl start
```

Штатный UI больше не включён в автозапуск этой конфигурации; команда выше
запустит уже `dmx-panel`.

## Работа с интерфейсом

На вкладке `Сцена` касание цветового круга сразу меняет RGB. Справа находятся
яркость, стартовый адрес RGB-тройки и Blackout. Допустимый адрес сцены —
1...510, потому что используются три соседних DMX-слота.

На вкладке `RDM`:

1. Нажать `Discovery`.
2. Выбрать найденный UID слева.
3. Нажать `Обновить info`, чтобы выполнить GET-запросы информации.
4. Переключатель `Identify` включает или выключает Identify.
5. Кнопками `−`/`+` выбрать новый DMX start address и нажать `Задать`.

RDM start address допускается в диапазоне 1...512. Само устройство проверяет,
помещается ли его footprint в universe, и может ответить NACK на неподходящий
адрес.

## RS485 и ограничения первого запуска

Для встроенного порта используется `/dev/ttyS4`. В DTS этой платы у UART4 нет
RTS/DE pin и RS485 properties, поэтому режим по умолчанию — `--rs485 auto`,
рассчитанный на аппаратное автоматическое переключение направления штатного
трансивера. `--rs485 rts` оставлен для изменённого железа/DTS и завершится с
явной ошибкой, если `TIOCSRS485` не поддержан.

Предоставленная схема подтверждает принцип схемы auto-flow. У `U5 SP485EEN`
выводы `/RE` и `DE` соединены, подтянуты вверх через `R23 4.7K` и стягиваются
транзистором `S1` от `TXD'`; `DI` постоянно подключён к `SGND`. Поэтому при
`TXD=0` передатчик включён и активно формирует SPACE, а при `TXD=1`
передатчик выключен, приёмник включён и MARK создаётся внешним bias
`R22/R26` (по 4.7K). Отдельный RTS/DE здесь не нужен и физически к U5 не
подведён. UART BREAK через `TIOCSBRK` проходит через ту же схему корректно.

У этого решения есть важное электрическое ограничение именно для DMX/RDM на
250 kbit/s: логическая единица передаётся не активным драйвером, а пассивной
подтяжкой 4.7K. На предоставленном фрагменте второй вывод `R27 120R` не показан
подключённым к противоположной линии, поэтому считать его встроенным
терминатором нельзя — фактическую комплектацию нужно проверить омметром. При
обычном внешнем терминаторе 120 Ом слабый bias может дать недостаточную
дифференциальную амплитуду: грубая оценка только по этим номиналам —
`5 V * 120 / (4700 + 120 + 4700) ≈ 63 mV`. Без терминатора на длинном кабеле,
наоборот, возможен слишком медленный фронт MARK. Это свойство железа,
программно его исправить нельзя.

Изолированная RS485-часть Panel 86 требует питания на широковольтном входе
даже тогда, когда сам Linux-модуль запитан от USB Type-C. Перед тестом также
нужно проверить A/B/GND и не подключать к одной DMX-линии два активных
контроллера. Стандартный дальний терминатор 120 Ом нужен для итоговой DMX-линии,
но сначала необходимо убедиться осциллографом, что именно эта auto-flow схема
с ним сохраняет нормальные уровни. Если нет, потребуется доработка bias/
трансивера либо внешний DMX/RDM-интерфейс с активной передачей обеих полярностей.

Перед эксплуатацией с разными приборами обязательно проверьте осциллографом
BREAK, MAB, момент отпускания передатчика и turnaround ответа. Приложение
пытается получить `SCHED_FIFO` и показывает timing warning при недоступном
real-time scheduling или обнаруженном превышении BREAK. Это не заменяет
измерения: userspace не способен заметить все задержки до фактического первого
бита UART. Режим `--break baud` — аварийный fallback: он всегда помечается
предупреждением, поскольку после смены baud нельзя гарантировать верхнюю
границу RDM MAB.

UID `7FF0:4C465831` находится в диапазоне, зарезервированном ESTA для
прототипов. Для выпускаемого контроллера передайте UID с официально выданным
Manufacturer ID; каждый физический controller должен иметь уникальную
32-битную device-часть UID.

## Полезные официальные ссылки

- Luckfox: [Modbus/RS485 tutorial для Panel 86](https://wiki.luckfox.com/Luckfox-Pico-86-Panel/QT/Modbus-Communication-Tutorial/)
- Luckfox: [страница Pico Panel 86](https://www.luckfox.com/Luckfox-Pico-86-Panel)
- ESTA: [опубликованные стандарты, включая актуальную E1.20](https://tsp.esta.org/tsp/documents/published_docs.php)
- ESTA: [реестр Manufacturer ID](https://tsp.esta.org/tsp/working_groups/CP/mfctrIDs.php)
