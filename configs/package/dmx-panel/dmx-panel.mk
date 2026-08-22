################################################################################
#
# dmx-panel demo application
#
################################################################################

DMX_PANEL_VERSION = $(shell sed -n '1p' $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src/dmx-panel/VERSION)
DMX_PANEL_SITE = $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src/dmx-panel
DMX_PANEL_SITE_METHOD = local
DMX_PANEL_LICENSE = Apache-2.0
DMX_PANEL_LICENSE_FILES = assets/fonts/LICENSE.txt

define DMX_PANEL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) \
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
	$(INSTALL) -D -m 0755 $(@D)/buildroot-out/bin/dmx-panel \
		$(TARGET_DIR)/usr/bin/dmx-panel
	$(INSTALL) -D -m 0644 $(@D)/buildroot-out/share/dmx-panel/VERSION \
		$(TARGET_DIR)/usr/share/dmx-panel/VERSION
endef

$(eval $(generic-package))
