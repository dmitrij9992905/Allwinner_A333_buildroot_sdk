# Allwinner A333 vendor components

Эти каталоги скопированы из исходной Android/Ubuntu-сборки HelperBoard A333
для независимого использования в Buildroot:

- `kernel/linux-6.6` — vendor-ядро Linux 6.6;
- `bsp` — независимый Allwinner BSP и драйверы;
- `device/a333` — A333-конфигурации, DTS/DTBO, boot-параметры и бинарные
  boot-артефакты;
- `boot/u-boot-2018` — исходники vendor U-Boot.

Git-метаданные исходных Android-repo проектов намеренно не копировались.
Исходным ориентиром были каталоги `source/src/longan` из соседнего vendor
дерева. В дальнейшем эти компоненты можно обновлять отдельно от Android SDK.
