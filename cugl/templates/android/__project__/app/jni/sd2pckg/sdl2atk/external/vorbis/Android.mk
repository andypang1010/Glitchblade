LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
CUGL_OFFSET := __CUGL_PATH__

###########################
#
# Vorbis static library
#
###########################
VORB_MAKE := $(LOCAL_PATH)
OGG_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/sdlapp/external/ogg
VORB_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/sdlapp/external/vorbis

include $(CLEAR_VARS)

LOCAL_MODULE := vorbis

LOCAL_C_INCLUDES += $(VORB_PATH)/include
LOCAL_C_INCLUDES += $(VORB_MAKE)/include/$(TARGET_ARCH_ABI)

LOCAL_CFLAGS    += -fPIC

# To keep file mangling to a minimum
LOCAL_PATH = $(VORB_PATH)/lib

# FLAC SOURCE CODE
LOCAL_SRC_FILES += \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/analysis.c \
    $(LOCAL_PATH)/bitrate.c \
    $(LOCAL_PATH)/block.c \
    $(LOCAL_PATH)/codebook.c \
    $(LOCAL_PATH)/envelope.c \
    $(LOCAL_PATH)/floor0.c \
    $(LOCAL_PATH)/floor1.c \
    $(LOCAL_PATH)/info.c \
    $(LOCAL_PATH)/lookup.c \
    $(LOCAL_PATH)/lpc.c \
    $(LOCAL_PATH)/lsp.c \
    $(LOCAL_PATH)/mapping0.c \
    $(LOCAL_PATH)/mdct.c \
    $(LOCAL_PATH)/psy.c \
    $(LOCAL_PATH)/registry.c \
    $(LOCAL_PATH)/res0.c \
    $(LOCAL_PATH)/sharedbook.c \
    $(LOCAL_PATH)/smallft.c \
    $(LOCAL_PATH)/synthesis.c \
    $(LOCAL_PATH)/vorbisenc.c \
    $(LOCAL_PATH)/vorbisfile.c \
    $(LOCAL_PATH)/window.c)

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

LOCAL_STATIC_LIBRARIES += ogg

include $(BUILD_STATIC_LIBRARY)
