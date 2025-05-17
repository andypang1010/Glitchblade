LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
ATK_OFFSET  := ../../../sdlapp

###########################
#
# OGG static library
#
###########################
OGG_MAKE  := $(LOCAL_PATH)
OGG_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(ATK_OFFSET)/external/ogg

include $(CLEAR_VARS)

LOCAL_MODULE := ogg

LOCAL_C_INCLUDES += $(OGG_PATH)/include
LOCAL_C_INCLUDES += $(OGG_MAKE)/include/$(TARGET_ARCH_ABI)

LOCAL_CFLAGS    += -fPIC

# To keep file mangling to a minimum
LOCAL_PATH = $(OGG_PATH)/src

# FLAC SOURCE CODE
LOCAL_SRC_FILES += \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/bitwise.c \
	$(LOCAL_PATH)/framing.c)

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_STATIC_LIBRARY)
