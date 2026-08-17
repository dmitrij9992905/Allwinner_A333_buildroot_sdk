# Сборка в Docker

В каталоге `buildroot/` находится Buildroot `2025.02.16` (LTS). Docker-окружение
изолирует версии Ubuntu и host-инструментов от основной системы. Нужные
компоненты vendor-сборки находятся в `vendor/allwinner-a333` и подключаются
в контейнер вместе с проектом.
Основной wrapper использует только Docker Engine CLI, поэтому Docker Compose
для сборки не требуется.

Собрать образ:

```sh
./docker-build.sh build
```

Открыть shell контейнера:

```sh
./docker-build.sh
```

После добавления defconfig платы типовые команды выглядят так:

```sh
./docker-build.sh prepare-sources
./docker-build.sh make O=../output BR2_EXTERNAL=../configs a333_helperboard_defconfig
./docker-build.sh make O=../output BR2_EXTERNAL=../configs menuconfig
./docker-build.sh make O=../output BR2_EXTERNAL=../configs
```

`output/`, `dl/` и `ccache/` находятся в рабочем каталоге и сохраняются между
запусками. `dl/` содержит скачанные исходники, а `ccache/` — кэш компилятора.
Они создаются с UID/GID текущего пользователя хоста. Передача дополнительных
переменных Buildroot выполняется обычным способом, например:

```sh
./docker-build.sh make O=../output BR2_EXTERNAL=../configs BR2_DL_DIR=/workspace/dl
```

Файл `compose.yaml` можно использовать отдельно на системах, где установлен
Docker Compose Plugin.
