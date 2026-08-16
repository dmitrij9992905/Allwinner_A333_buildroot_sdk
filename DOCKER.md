# Сборка в Docker

В корне проекта находится Buildroot `2025.02.16` (LTS). Docker-окружение
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
./docker-build.sh make O=output a333_defconfig
./docker-build.sh make O=output menuconfig
./docker-build.sh make O=output
```

`output/` и `dl/` находятся в рабочем каталоге и сохраняются между запусками.
Они создаются с UID/GID текущего пользователя хоста. Передача дополнительных
переменных Buildroot выполняется обычным способом, например:

```sh
./docker-build.sh make O=output BR2_DL_DIR=/workspace/dl
```

Файл `compose.yaml` можно использовать отдельно на системах, где установлен
Docker Compose Plugin.
