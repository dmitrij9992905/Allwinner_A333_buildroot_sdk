# Shared static display/input/font library for Make consumers.
# The parent sets BUILD_DIR, TARGET_CC/AR and CPPFLAGS/CFLAGS.
PANEL_COMMON_DIR ?= ../panel-common
COMMON_CPPFLAGS := -I$(PANEL_COMMON_DIR)/include -I$(PANEL_COMMON_DIR)/vendor/lvgl -I$(PANEL_COMMON_DIR)/vendor
COMMON_LIB := $(BUILD_DIR)/lib/libpanel_common.a
COMMON_SOURCES := $(wildcard $(PANEL_COMMON_DIR)/src/*.c)
COMMON_OBJECTS := $(patsubst $(PANEL_COMMON_DIR)/src/%.c,$(BUILD_DIR)/panel-common/%.o,$(COMMON_SOURCES))
FONT_ASSET := $(abspath $(PANEL_COMMON_DIR)/assets/fonts/Roboto-Regular.ttf)
FONT_ASSET_SHA256 := 797e35f7f5d6020a5c6ea13b42ecd668bcfb3bbc4baa0e74773527e5b6cb3174
FONT_ASSET_OBJECT := $(BUILD_DIR)/panel-common/roboto_font_data.o
COMMON_OBJECTS += $(FONT_ASSET_OBJECT)

# LVGL 9.5.0 is vendored for both applications. Keep this path relative:
# GCC records prerequisite names in the generated .d files, and absolute
# paths would make a copied SDK tree depend on the original build directory.
# The SDK-wide LVGL 8.x library and its generated configuration are
# deliberately not used.
LVGL_ROOT := $(PANEL_COMMON_DIR)/vendor/lvgl
LVGL_SRC_DIR := $(LVGL_ROOT)/src
LVGL_LIB := $(BUILD_DIR)/lib/liblvgl.a
# Compile only the core, software renderer, TinyTTF and widgets used here.
# This avoids pulling the 3D, SDL, DRM, Wayland, codec and GPU backends into
# the build while keeping the complete upstream headers available.
LVGL_SOURCE_DIRS := $(LVGL_SRC_DIR)/core $(LVGL_SRC_DIR)/display \
	$(LVGL_SRC_DIR)/draw/convert $(LVGL_SRC_DIR)/draw/sw \
	$(LVGL_SRC_DIR)/font $(LVGL_SRC_DIR)/indev $(LVGL_SRC_DIR)/misc \
	$(LVGL_SRC_DIR)/osal $(LVGL_SRC_DIR)/stdlib $(LVGL_SRC_DIR)/tick \
	$(LVGL_SRC_DIR)/layouts/flex $(LVGL_SRC_DIR)/libs/bin_decoder \
	$(LVGL_SRC_DIR)/libs/tiny_ttf \
	$(LVGL_SRC_DIR)/themes/default \
	$(LVGL_SRC_DIR)/debugging/sysmon \
	$(LVGL_SRC_DIR)/widgets/bar $(LVGL_SRC_DIR)/widgets/button \
	$(LVGL_SRC_DIR)/widgets/buttonmatrix $(LVGL_SRC_DIR)/widgets/canvas \
	$(LVGL_SRC_DIR)/widgets/image $(LVGL_SRC_DIR)/widgets/label \
	$(LVGL_SRC_DIR)/widgets/list $(LVGL_SRC_DIR)/widgets/slider \
	$(LVGL_SRC_DIR)/widgets/spinbox $(LVGL_SRC_DIR)/widgets/switch \
	$(LVGL_SRC_DIR)/widgets/tabview $(LVGL_SRC_DIR)/widgets/textarea
LVGL_TOPLEVEL_DIRS := $(LVGL_SRC_DIR) $(LVGL_SRC_DIR)/draw \
	$(LVGL_SRC_DIR)/layouts $(LVGL_SRC_DIR)/themes
LVGL_SOURCES := $(shell (find $(LVGL_SOURCE_DIRS) -type f -name '*.c'; \
	find $(LVGL_TOPLEVEL_DIRS) -maxdepth 1 -type f -name '*.c') | \
	LC_ALL=C sort -u)
LVGL_OBJECTS := $(patsubst $(LVGL_SRC_DIR)/%.c,$(BUILD_DIR)/panel-common/lvgl/%.o,$(LVGL_SOURCES))


check-lvgl:
	@test -f $(LVGL_ROOT)/lvgl.h || { \
		echo "Vendored LVGL sources not found in $(LVGL_ROOT)" >&2; \
		exit 1; \
	}
	@grep -Eq '^#define LVGL_VERSION_MAJOR[[:space:]]+9$$' $(LVGL_ROOT)/lv_version.h && \
		grep -Eq '^#define LVGL_VERSION_MINOR[[:space:]]+5$$' $(LVGL_ROOT)/lv_version.h && \
		grep -Eq '^#define LVGL_VERSION_PATCH[[:space:]]+0$$' $(LVGL_ROOT)/lv_version.h || { \
		echo "panel-common requires exactly LVGL 9.5.0" >&2; \
		exit 1; \
	}

check-roboto:
	@test -f $(FONT_ASSET) || { \
		echo "Embedded Roboto asset not found: $(FONT_ASSET)" >&2; \
		exit 1; \
	}
	@echo "$(FONT_ASSET_SHA256)  $(FONT_ASSET)" | sha256sum -c -
	@test -f $(PANEL_COMMON_DIR)/assets/fonts/LICENSE.txt -a -f $(PANEL_COMMON_DIR)/assets/fonts/NOTICE.txt || { \
		echo "Roboto license/notice files are missing" >&2; \
		exit 1; \
	}

$(LVGL_LIB): $(LVGL_OBJECTS)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(TARGET_AR) rcs $@ $^

$(BUILD_DIR)/panel-common/lvgl/%.o: $(LVGL_SRC_DIR)/%.c $(PANEL_COMMON_DIR)/include/lv_conf.h
	@mkdir -p $(dir $@)
	$(TARGET_CC) $(CPPFLAGS) $(LVGL_CFLAGS) -MMD -MP -c $< -o $@

$(LVGL_OBJECTS): | check-lvgl

$(FONT_ASSET_OBJECT): $(PANEL_COMMON_DIR)/src/roboto_font_data.S $(FONT_ASSET) | check-roboto
	@mkdir -p $(dir $@)
	$(TARGET_CC) -DPANEL_ROBOTO_TTF_PATH='"$(FONT_ASSET)"' -c $< -o $@


$(COMMON_LIB): $(COMMON_OBJECTS)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(TARGET_AR) rcs $@ $^

$(BUILD_DIR)/panel-common/%.o: $(PANEL_COMMON_DIR)/src/%.c
	@mkdir -p $(dir $@)
	$(TARGET_CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@
