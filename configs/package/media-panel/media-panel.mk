################################################################################
# A333 media panel, built against the separately copied LVGL sources
################################################################################

MEDIA_PANEL_VERSION = 1.0.0
MEDIA_PANEL_SITE = $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src/dmx-panel
MEDIA_PANEL_SITE_METHOD = local
MEDIA_PANEL_LICENSE = Apache-2.0
MEDIA_PANEL_LICENSE_FILES = assets/fonts/LICENSE.txt
MEDIA_PANEL_SRC = $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src/media-panel/media_panel.c

define MEDIA_PANEL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) BUILDROOT=1 \
		BUILD_DIR=buildroot-build TARGET_CC="$(TARGET_CC)" \
		TARGET_AR="$(TARGET_AR)" TARGET_STRIP="$(TARGET_STRIP)" \
		DISPLAY_ROTATION="$(BR2_A333_DISPLAY_ROTATION)" \
		lvgl buildroot-build/roboto_font_data.o
	$(TARGET_CC) -std=c11 -Os -D_GNU_SOURCE \
		-DDMX_PANEL_DISPLAY_ROTATION=$(BR2_A333_DISPLAY_ROTATION) \
		-DLV_CONF_INCLUDE_SIMPLE=1 -DLV_DISABLE_API_MAPPING \
		-I$(@D)/include -I$(@D)/vendor/lvgl -I$(@D)/vendor \
		$(MEDIA_PANEL_SRC) $(@D)/src/lvgl_port.c \
		$(@D)/src/panel_fbdev.c $(@D)/src/panel_input.c \
		$(@D)/src/panel_canvas.c $(@D)/src/lvgl_fonts.c \
		$(@D)/buildroot-build/roboto_font_data.o \
		$(@D)/buildroot-build/lib/liblvgl.a \
		-pthread -lm -o $(@D)/media-panel
endef

define MEDIA_PANEL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/media-panel $(TARGET_DIR)/usr/bin/media-panel
endef

$(eval $(generic-package))
