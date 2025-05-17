LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
TTF_OFFSET  := __SDL2_PATH__

###########################
#
# harfbuzz static library
#
###########################
FREETYPE_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(TTF_OFFSET)/external/freetype
HARFBUZZ_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(TTF_OFFSET)/external/harfbuzz

include $(CLEAR_VARS)

LOCAL_PATH := $(HARFBUZZ_PATH)/src

LOCAL_SRC_FILES = \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/hb-aat-layout.cc \
    $(LOCAL_PATH)/hb-aat-map.cc \
    $(LOCAL_PATH)/hb-blob.cc \
    $(LOCAL_PATH)/hb-buffer-serialize.cc \
    $(LOCAL_PATH)/hb-buffer.cc \
    $(LOCAL_PATH)/hb-common.cc \
    $(LOCAL_PATH)/hb-face.cc \
    $(LOCAL_PATH)/hb-fallback-shape.cc \
    $(LOCAL_PATH)/hb-font.cc \
    $(LOCAL_PATH)/hb-ft.cc \
    $(LOCAL_PATH)/hb-number.cc \
    $(LOCAL_PATH)/hb-ms-feature-ranges.cc \
    $(LOCAL_PATH)/hb-ot-cff1-table.cc \
    $(LOCAL_PATH)/hb-ot-cff2-table.cc \
    $(LOCAL_PATH)/hb-ot-face.cc \
    $(LOCAL_PATH)/hb-ot-font.cc \
    $(LOCAL_PATH)/hb-ot-layout.cc \
    $(LOCAL_PATH)/hb-ot-map.cc \
    $(LOCAL_PATH)/hb-ot-math.cc \
    $(LOCAL_PATH)/hb-ot-metrics.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-arabic.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-default.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-hangul.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-hebrew.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-indic-table.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-indic.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-khmer.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-myanmar.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-syllabic.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-thai.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-use.cc \
    $(LOCAL_PATH)/hb-ot-shape-complex-vowel-constraints.cc \
    $(LOCAL_PATH)/hb-ot-shape-fallback.cc \
    $(LOCAL_PATH)/hb-ot-shape-normalize.cc \
    $(LOCAL_PATH)/hb-ot-shape.cc \
    $(LOCAL_PATH)/hb-ot-tag.cc \
    $(LOCAL_PATH)/hb-ot-var.cc \
    $(LOCAL_PATH)/hb-set.cc \
    $(LOCAL_PATH)/hb-shape-plan.cc \
    $(LOCAL_PATH)/hb-shape.cc \
    $(LOCAL_PATH)/hb-shaper.cc \
    $(LOCAL_PATH)/hb-static.cc \
    $(LOCAL_PATH)/hb-ucd.cc \
    $(LOCAL_PATH)/hb-unicode.cc)

LOCAL_ARM_MODE := arm

LOCAL_CPP_EXTENSION := .cc

LOCAL_C_INCLUDES = \
    $(HARFBUZZ_PATH)/ \
    $(HARFBUZZ_PATH)/src/ \
    $(FREETYPE_PATH)/include/ \

#LOCAL_CFLAGS += -DHB_NO_MT -DHAVE_OT -DHAVE_UCDN -fPIC
LOCAL_CFLAGS += -DHAVE_CONFIG_H -fPIC 

LOCAL_STATIC_LIBRARIES += freetype

LOCAL_EXPORT_C_INCLUDES = $(HARFBUZZ_PATH)/src/

# -DHAVE_ICU -DHAVE_ICU_BUILTIN
LOCAL_MODULE:= harfbuzz

include $(BUILD_STATIC_LIBRARY)
