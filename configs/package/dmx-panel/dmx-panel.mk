################################################################################
#
# dmx-panel demo application
#
################################################################################

DMX_PANEL_VERSION = $(shell sed -n '1p' $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src/dmx-panel/VERSION)
DMX_PANEL_SITE = $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src
DMX_PANEL_SITE_METHOD = local
DMX_PANEL_LICENSE = Apache-2.0, MIT
DMX_PANEL_LICENSE_FILES = panel-common/assets/fonts/LICENSE.txt panel-common/vendor/lvgl/LICENCE.txt

define DMX_PANEL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/dmx-panel \
		BUILDROOT=1 \
		PKG_BIN=buildroot-out \
		BUILD_DIR=buildroot-build \
		TARGET_CC="$(TARGET_CC)" \
		TARGET_AR="$(TARGET_AR)" \
		TARGET_STRIP="$(TARGET_STRIP)" \
		DISPLAY_ROTATION="$(BR2_A333_DISPLAY_ROTATION)" \
		all
endef

define DMX_PANEL_INSTALL_TARGET_CMDS
	$(INSTALL) -D -m 0755 $(@D)/dmx-panel/buildroot-out/bin/dmx-panel \
		$(TARGET_DIR)/usr/bin/dmx-panel
	$(INSTALL) -D -m 0644 $(@D)/dmx-panel/buildroot-out/share/dmx-panel/VERSION \
		$(TARGET_DIR)/usr/share/dmx-panel/VERSION
endef

$(eval $(generic-package))

# Resynchronize packages cached before the shared-library layout migration.
$(DMX_PANEL_DIR)/.stamp_rsynced: $(DMX_PANEL_SITE)/dmx-panel/Makefile \
	$(DMX_PANEL_SITE)/panel-common/common.mk \
	$(BR2_EXTERNAL_A333_PATH)/package/dmx-panel/dmx-panel.mk
