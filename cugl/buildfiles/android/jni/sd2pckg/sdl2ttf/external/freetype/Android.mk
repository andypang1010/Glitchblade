LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
TTF_OFFSET  := ../../../sdlapp

###########################
#
# freetype static library
#
###########################
FREETYPE_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(TTF_OFFSET)/external/freetype
HARFBUZZ_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(TTF_OFFSET)/external/harfbuzz

include $(CLEAR_VARS)

LOCAL_PATH := $(FREETYPE_PATH)

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/src/autofit/autofit.c \
	$(LOCAL_PATH)/src/base/ftbase.c \
	$(LOCAL_PATH)/src/base/ftbbox.c \
	$(LOCAL_PATH)/src/base/ftbdf.c \
	$(LOCAL_PATH)/src/base/ftbitmap.c \
	$(LOCAL_PATH)/src/base/ftcid.c \
	$(LOCAL_PATH)/src/base/ftdebug.c \
	$(LOCAL_PATH)/src/base/ftfstype.c \
	$(LOCAL_PATH)/src/base/ftgasp.c \
	$(LOCAL_PATH)/src/base/ftglyph.c \
	$(LOCAL_PATH)/src/base/ftgxval.c \
	$(LOCAL_PATH)/src/base/ftinit.c \
	$(LOCAL_PATH)/src/base/ftmm.c \
	$(LOCAL_PATH)/src/base/ftotval.c \
	$(LOCAL_PATH)/src/base/ftpatent.c \
	$(LOCAL_PATH)/src/base/ftpfr.c \
	$(LOCAL_PATH)/src/base/ftstroke.c \
	$(LOCAL_PATH)/src/base/ftsynth.c \
	$(LOCAL_PATH)/src/base/ftsystem.c \
	$(LOCAL_PATH)/src/base/fttype1.c \
	$(LOCAL_PATH)/src/base/ftwinfnt.c \
	$(LOCAL_PATH)/src/bdf/bdf.c \
	$(LOCAL_PATH)/src/bzip2/ftbzip2.c \
	$(LOCAL_PATH)/src/cache/ftcache.c \
	$(LOCAL_PATH)/src/cff/cff.c \
	$(LOCAL_PATH)/src/cid/type1cid.c \
	$(LOCAL_PATH)/src/gzip/ftgzip.c \
	$(LOCAL_PATH)/src/lzw/ftlzw.c \
	$(LOCAL_PATH)/src/pcf/pcf.c \
	$(LOCAL_PATH)/src/pfr/pfr.c \
	$(LOCAL_PATH)/src/psaux/psaux.c \
	$(LOCAL_PATH)/src/pshinter/pshinter.c \
	$(LOCAL_PATH)/src/psnames/psmodule.c \
	$(LOCAL_PATH)/src/raster/raster.c \
	$(LOCAL_PATH)/src/sdf/sdf.c \
	$(LOCAL_PATH)/src/sfnt/sfnt.c \
	$(LOCAL_PATH)/src/svg/svg.c \
	$(LOCAL_PATH)/src/smooth/smooth.c \
	$(LOCAL_PATH)/src/truetype/truetype.c \
	$(LOCAL_PATH)/src/type1/type1.c \
	$(LOCAL_PATH)/src/type42/type42.c \
	$(LOCAL_PATH)/src/winfonts/winfnt.c)

LOCAL_C_INCLUDES += $(FREETYPE_PATH)/include

SUPPORT_HARFBUZZ ?= true
ifeq ($(SUPPORT_HARFBUZZ),true)
    LOCAL_C_INCLUDES += $(HARFBUZZ_PATH)/src/
    LOCAL_CFLAGS += -DFT_CONFIG_OPTION_USE_HARFBUZZ
endif

LOCAL_CFLAGS += -DFT2_BUILD_LIBRARY -Os

LOCAL_MODULE:= freetype

LOCAL_COPY_HEADERS_TO := $(common_COPY_HEADERS_TO)
LOCAL_COPY_HEADERS := $(common_COPY_HEADERS)

include $(BUILD_STATIC_LIBRARY)

