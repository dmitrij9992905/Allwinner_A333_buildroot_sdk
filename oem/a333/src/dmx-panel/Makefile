BUILDROOT ?= 0

ifeq ($(BUILDROOT),1)
# Buildroot supplies the compiler, archiver and stripper.  Do not include the
# Luckfox SDK makefile: the copied application is intentionally standalone.
SDK_ROOT :=
APP_PARAM :=
else
SDK_ROOT := $(abspath ../../..)
-include $(SDK_ROOT)/.BoardConfig.mk
ifneq ($(RK_TOOLCHAIN_CROSS),)
export PATH := $(SDK_ROOT)/tools/linux/toolchain/$(RK_TOOLCHAIN_CROSS)/bin:$(PATH)
endif

ifeq ($(APP_PARAM),)
APP_PARAM := ../Makefile.param
endif
endif

# Cleaning is intentionally usable before the cross-toolchain is installed.
# Makefile.param performs a compiler probe while it is parsed, which otherwise
# prevents removal of stale dependency files after copying the app tree.
ifneq ($(filter clean distclean,$(MAKECMDGOALS)),)
SKIP_APP_PARAM := 1
endif
ifneq ($(SKIP_APP_PARAM),1)
ifneq ($(BUILDROOT),1)
include $(APP_PARAM)
endif
endif

SHELL := /bin/bash

PKG_NAME := dmx-panel
PKG_BIN ?= out
BUILD_DIR ?= build/target
TARGET := $(PKG_BIN)/bin/dmx-panel
VERSION_FILE := VERSION
DMX_PANEL_VERSION := $(strip $(shell cat $(VERSION_FILE) 2>/dev/null))
VERSION_OUTPUT := $(PKG_BIN)/share/dmx-panel/VERSION

ifeq ($(DMX_PANEL_VERSION),)
$(error dmx-panel VERSION file is missing or empty)
endif

# LVGL 9.5.0 is vendored for this application.  Keep this path relative:
# GCC records prerequisite names in the generated .d files, and absolute
# paths would make a copied SDK tree depend on the original build directory.
# The SDK-wide LVGL 8.x library and its generated configuration are
# deliberately not used.
LVGL_ROOT := vendor/lvgl
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
LVGL_OBJECTS := $(patsubst $(LVGL_SRC_DIR)/%.c,$(BUILD_DIR)/lvgl/%.o,$(LVGL_SOURCES))

ifeq ($(BUILDROOT),1)
TARGET_CC ?= $(CC)
TARGET_AR ?= $(AR)
TARGET_STRIP ?= $(STRIP)
else
TARGET_CC := $(RK_APP_CROSS)-gcc
TARGET_AR := $(RK_APP_CROSS)-ar
TARGET_STRIP := $(RK_APP_CROSS)-strip
endif
HOST_CC ?= cc
DISPLAY_ROTATION ?= 90

ifneq ($(filter 0 90 180 270,$(DISPLAY_ROTATION)),$(DISPLAY_ROTATION))
$(error DISPLAY_ROTATION must be one of 0, 90, 180 or 270)
endif

# Keep both upstream-supported include spellings usable by application code:
# <lvgl.h> and <lvgl/lvgl.h>.
CPPFLAGS := -Iinclude -I$(LVGL_ROOT) -I$(dir $(LVGL_ROOT)) \
	-D_GNU_SOURCE -DLV_CONF_INCLUDE_SIMPLE=1 -DLV_DISABLE_API_MAPPING \
	-DDMX_PANEL_VERSION=\"$(DMX_PANEL_VERSION)\" \
	-DDMX_PANEL_DISPLAY_ROTATION=$(DISPLAY_ROTATION)
CFLAGS := -std=c11 -Os -Wall -Wextra -Werror $(RK_APP_CROSS_CFLAGS) \
	-ffunction-sections -fdata-sections
LVGL_CFLAGS := $(filter-out -Werror,$(CFLAGS)) \
	-Wno-unused-function -Wno-unused-parameter
LDLIBS := -pthread -lm
LDFLAGS := -Wl,--gc-sections

# panel_ui.c is the retired hand-rendered UI.  Its public action types are
# still shared with the controller, but the target application uses lvgl_ui.c.
SOURCES := $(filter-out src/panel_ui.c,$(wildcard src/*.c))
FONT_ASSET := $(abspath assets/fonts/Roboto-Regular.ttf)
FONT_ASSET_SHA256 := 797e35f7f5d6020a5c6ea13b42ecd668bcfb3bbc4baa0e74773527e5b6cb3174
FONT_ASSET_OBJECT := $(BUILD_DIR)/roboto_font_data.o
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCES)) \
	$(FONT_ASSET_OBJECT)

.PHONY: all dmx-panel-build lvgl check-lvgl check-roboto clean distclean test preview info

ifeq ($(BUILDROOT),1)
all: $(TARGET) $(VERSION_OUTPUT)
else
all: dmx-panel-build
	@echo "build $(PKG_NAME) done"
endif

dmx-panel-build: $(TARGET) $(VERSION_OUTPUT)
	$(call MAROC_COPY_PKG_TO_APP_OUTPUT,$(RK_APP_OUTPUT),$(PKG_BIN))

$(VERSION_OUTPUT): $(VERSION_FILE)
	@mkdir -p $(dir $@)
	cp -f $< $@

$(TARGET): $(OBJECTS) $(LVGL_LIB)
	@mkdir -p $(dir $@)
	$(TARGET_CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(TARGET_STRIP) --strip-unneeded $@

lvgl: $(LVGL_LIB)

check-lvgl:
	@test -f $(LVGL_ROOT)/lvgl.h || { \
		echo "Vendored LVGL sources not found in $(LVGL_ROOT)" >&2; \
		exit 1; \
	}
	@grep -Eq '^#define LVGL_VERSION_MAJOR[[:space:]]+9$$' $(LVGL_ROOT)/lv_version.h && \
		grep -Eq '^#define LVGL_VERSION_MINOR[[:space:]]+5$$' $(LVGL_ROOT)/lv_version.h && \
		grep -Eq '^#define LVGL_VERSION_PATCH[[:space:]]+0$$' $(LVGL_ROOT)/lv_version.h || { \
		echo "dmx-panel requires exactly LVGL 9.5.0" >&2; \
		exit 1; \
	}

check-roboto:
	@test -f $(FONT_ASSET) || { \
		echo "Embedded Roboto asset not found: $(FONT_ASSET)" >&2; \
		exit 1; \
	}
	@echo "$(FONT_ASSET_SHA256)  $(FONT_ASSET)" | sha256sum -c -
	@test -f assets/fonts/LICENSE.txt -a -f assets/fonts/NOTICE.txt || { \
		echo "Roboto license/notice files are missing" >&2; \
		exit 1; \
	}

$(LVGL_LIB): $(LVGL_OBJECTS)
	@mkdir -p $(dir $@)
	@rm -f $@
	$(TARGET_AR) rcs $@ $^

$(BUILD_DIR)/lvgl/%.o: $(LVGL_SRC_DIR)/%.c include/lv_conf.h
	@mkdir -p $(dir $@)
	$(TARGET_CC) $(CPPFLAGS) $(LVGL_CFLAGS) -MMD -MP -c $< -o $@

$(LVGL_OBJECTS): | check-lvgl

$(FONT_ASSET_OBJECT): src/roboto_font_data.S $(FONT_ASSET) | check-roboto
	@mkdir -p $(dir $@)
	$(TARGET_CC) -DDMX_ROBOTO_TTF_PATH='"$(FONT_ASSET)"' -c $< -o $@

$(BUILD_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(TARGET_CC) $(CPPFLAGS) $(CFLAGS) -MMD -MP -c $< -o $@

ifneq ($(SKIP_APP_PARAM),1)
-include $(OBJECTS:.o=.d) $(LVGL_OBJECTS:.o=.d)
endif

test:
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror \
		-Iinclude src/rdm_protocol.c tests/test_rdm_protocol.c \
		-o tests/test_rdm_protocol
	./tests/test_rdm_protocol
	@echo "Running legacy raw-canvas UI regression (not an LVGL test)"
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror \
		-Iinclude src/panel_canvas.c src/panel_ui.c src/rdm_protocol.c \
		tests/test_panel_ui.c -lm -o tests/test_panel_ui
	./tests/test_panel_ui
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror \
		-Iinclude src/rdm_protocol.c src/dmx_transport.c \
		src/dmx_controller.c tests/test_dmx_controller.c \
		-pthread -lm -o tests/test_dmx_controller
	./tests/test_dmx_controller

preview:
	@echo "Rendering legacy raw-canvas preview (not the LVGL 9.5 UI)"
	@if [[ ! -f tools/render_preview.c ]]; then \
		echo "tools/render_preview.c is not available" >&2; exit 1; \
	fi
	$(HOST_CC) -std=c11 -O2 -Wall -Wextra -Werror \
		-Iinclude src/panel_canvas.c src/panel_ui.c src/rdm_protocol.c \
		tools/render_preview.c -lm -o tools/render_preview
	./tools/render_preview scene-preview.ppm rdm-preview.ppm

clean:
	rm -rf build $(PKG_BIN) tests/test_rdm_protocol tests/test_panel_ui \
		tests/test_dmx_controller \
		tools/render_preview scene-preview.ppm rdm-preview.ppm

distclean: clean

info:
	@echo "$(PKG_NAME): framebuffer/evdev DMX512 + RDM controller for Allwinner A333 1280x800"
