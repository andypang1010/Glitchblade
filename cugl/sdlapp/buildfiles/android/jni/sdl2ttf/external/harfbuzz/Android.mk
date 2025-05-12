LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../..
TTF_OFFSET  := ../../..

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
    $(LOCAL_PATH)/harfbuzz.cc)

LOCAL_ARM_MODE := arm

LOCAL_CPP_EXTENSION := .cc

LOCAL_C_INCLUDES = \
    $(HARFBUZZ_PATH)/ \
    $(HB_PATCH_PATH)/src/ \
    $(FREETYPE_PATH)/include/ \

#LOCAL_CFLAGS += -DHB_NO_MT -DHAVE_OT -DHAVE_UCDN -fPIC
LOCAL_CFLAGS += -DHAVE_CONFIG_H -fPIC 

LOCAL_STATIC_LIBRARIES += freetype

LOCAL_EXPORT_C_INCLUDES = $(HB_PATCH_PATH)/src/

# -DHAVE_ICU -DHAVE_ICU_BUILTIN
LOCAL_MODULE:= harfbuzz

include $(BUILD_STATIC_LIBRARY)
