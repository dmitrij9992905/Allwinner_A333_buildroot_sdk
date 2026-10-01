################################################################################
# A333 media panel, built against the separately copied LVGL sources
################################################################################

MEDIA_PANEL_VERSION = 1.0.0
MEDIA_PANEL_SITE = $(BR2_EXTERNAL_A333_PATH)/../oem/a333/src
MEDIA_PANEL_SITE_METHOD = local
MEDIA_PANEL_SUBDIR = media-panel
MEDIA_PANEL_SUPPORTS_IN_SOURCE_BUILD = NO
MEDIA_PANEL_LICENSE = Apache-2.0, MIT
MEDIA_PANEL_LICENSE_FILES = panel-common/assets/fonts/LICENSE.txt panel-common/vendor/lvgl/LICENCE.txt
MEDIA_PANEL_CONF_OPTS = -DMEDIA_PANEL_DISPLAY_ROTATION=$(BR2_A333_DISPLAY_ROTATION)

$(eval $(cmake-package))

# Migrate already-built Makefile packages too, and reconfigure when the local
# CMake project changes. Sources/objects of unrelated packages stay untouched.
$(MEDIA_PANEL_DIR)/.stamp_rsynced: $(MEDIA_PANEL_SITE)/media-panel/CMakeLists.txt \
	$(MEDIA_PANEL_SITE)/panel-common/CMakeLists.txt \
	$(BR2_EXTERNAL_A333_PATH)/package/media-panel/media-panel.mk
$(MEDIA_PANEL_DIR)/.stamp_configured: $(MEDIA_PANEL_DIR)/.stamp_rsynced
