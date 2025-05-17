LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
ATK_OFFSET  := ../../../sdlapp

###########################
#
# KissFFT static library
#
###########################
KFFT_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(ATK_OFFSET)/external/kissfft

include $(CLEAR_VARS)

LOCAL_MODULE := kissfft

LOCAL_C_INCLUDES += $(KFFT_PATH)

LOCAL_CFLAGS += -fPIC -ffast-math 
LOCAL_CFLAGS += -fomit-frame-pointer
LOCAL_CFLAGS += -D'kiss_fft_scalar=float'

# To keep file mangling to a minimum
LOCAL_PATH = $(KFFT_PATH)

# FLAC SOURCE CODE
LOCAL_SRC_FILES += \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/kfc.c \
    $(LOCAL_PATH)/kiss_fft.c \
    $(LOCAL_PATH)/kiss_fftnd.c \
    $(LOCAL_PATH)/kiss_fftndr.c \
    $(LOCAL_PATH)/kiss_fftr.c)

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_STATIC_LIBRARY)
