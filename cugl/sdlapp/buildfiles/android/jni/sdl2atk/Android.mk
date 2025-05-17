LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ..
SDL2_OFFSET := ../../..
ATK_OFFSET  := ../../..

###########################
#
# SDL_ATK shared library
#
###########################
SDL2_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(SDL2_OFFSET)
SDL2_ATK_MAKE   := $(LOCAL_PATH)
SDL2_ATK_PATH   := $(LOCAL_PATH)/$(CURR_DEPTH)/$(ATK_OFFSET)
SDL2_ATK_SOURCE := $(SDL2_ATK_PATH)/src/atk

# WAV is built in
SUPPORT_WAV ?= true
SUPPORT_SAVE_WAV ?= true

# You must download kissfft using the external/download.sh script.
KFFT_LIBRARY_PATH := external/kissfft
include $(SDL2_ATK_MAKE)/$(KFFT_LIBRARY_PATH)/Android.mk

# The additional formats below require downloading third party dependencies,
# using the external/download.sh script.

# This will be enabled automatically by FLAC or Vorbis
# The library path should be a relative path to this directory.
SUPPORT_OGG ?= true
OGG_LIBRARY_PATH := external/ogg

# Enable this if you want to support loading FLAC files
# The library path should be a relative path to this directory.
SUPPORT_FLAC ?= true
SUPPORT_SAVE_FLAC ?= true
FLAC_LIBRARY_PATH := external/flac

# Enable this if you want to support loading Vorbis files
# The library path should be a relative path to this directory.
SUPPORT_VORB ?= true
SUPPORT_SAVE_VORB ?= true
VORB_LIBRARY_PATH := external/vorbis

# Enable this if you want to support loading MP3 files
# The library path should be a relative path to this directory.
SUPPORT_MP3 ?= true
MP3_LIBRARY_PATH := external/minimp3

# Build the library
ifeq ($(SUPPORT_FLAC),true)
    include $(SDL2_ATK_MAKE)/$(FLAC_LIBRARY_PATH)/Android.mk
	SUPPORT_OGG = true
endif

# Build the library
ifeq ($(SUPPORT_VORB),true)
    include $(SDL2_ATK_MAKE)/$(VORB_LIBRARY_PATH)/Android.mk
	SUPPORT_OGG = true
endif

# Build the library
ifeq ($(SUPPORT_OGG),true)
    include $(SDL2_ATK_MAKE)/$(OGG_LIBRARY_PATH)/Android.mk
endif

# Keep file mangling at a minimum
LOCAL_PATH := $(SDL2_ATK_SOURCE)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_atk

LOCAL_SRC_FILES :=  \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/rand/ATK_Rand.c          \
    $(LOCAL_PATH)/math/ATK_MathVec.c       \
    $(LOCAL_PATH)/math/ATK_MathComplex.c   \
    $(LOCAL_PATH)/math/ATK_MathPoly.c      \
    $(LOCAL_PATH)/file/ATK_file.c          \
    $(LOCAL_PATH)/dsp/ATK_DSPConvolve.c    \
    $(LOCAL_PATH)/dsp/ATK_DSPFilter.c      \
    $(LOCAL_PATH)/dsp/ATK_DSPTransform.c   \
    $(LOCAL_PATH)/dsp/ATK_DSPWaveform.c    \
    $(LOCAL_PATH)/codec/ATK_Codec.c        \
    $(LOCAL_PATH)/codec/ATK_CodecFLAC.c    \
    $(LOCAL_PATH)/codec/ATK_CodecMP3.c     \
    $(LOCAL_PATH)/codec/ATK_CodecVorbis.c  \
    $(LOCAL_PATH)/codec/ATK_CodecWAV.c     \
    $(LOCAL_PATH)/audio/ATK_AlgoReverb.c   \
    $(LOCAL_PATH)/audio/ATK_AudioCVT.c     \
    $(LOCAL_PATH)/audio/ATK_LatencyAdapter.c)

LOCAL_C_INCLUDES := $(SDL2_ATK_PATH)/include
LOCAL_C_INCLUDES += $(SDL2_ATK_PATH)/$(KFFT_LIBRARY_PATH)
LOCAL_C_INCLUDES += $(SDL2_PATH)/include
LOCAL_CFLAGS 	 := -Wno-format 
LOCAL_CFLAGS 	 += -D'kiss_fft_scalar=float'
LOCAL_CFLAGS     += -O2
LOCAL_STATIC_LIBRARIES := kissfft

ifeq ($(SUPPORT_WAV),true)
    LOCAL_CFLAGS += -DLOAD_WAV
ifeq ($(SUPPORT_SAVE_WAV),true)
    LOCAL_CFLAGS += -DSAVE_WAV
endif
endif

ifeq ($(SUPPORT_FLAC),true)
	LOCAL_C_INCLUDES += $(SDL2_ATK_PATH)/$(FLAC_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DLOAD_FLAC -DFLAC__NO_DLL
    LOCAL_STATIC_LIBRARIES += flac
ifeq ($(SUPPORT_SAVE_FLAC),true)
    LOCAL_CFLAGS += -DSAVE_FLAC
endif
endif

ifeq ($(SUPPORT_VORB),true)
	LOCAL_C_INCLUDES += $(SDL2_ATK_PATH)/$(VORB_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DLOAD_VORB
    LOCAL_STATIC_LIBRARIES += vorbis
ifeq ($(SUPPORT_SAVE_VORB),true)
    LOCAL_CFLAGS += -DSAVE_VORB
endif
endif

ifeq ($(SUPPORT_OGG),true)
    LOCAL_C_INCLUDES += $(SDL2_ATK_PATH)/$(OGG_LIBRARY_PATH)/include
    LOCAL_STATIC_LIBRARIES += ogg
endif

# This is an include only library
ifeq ($(SUPPORT_MP3),true)
	LOCAL_C_INCLUDES += $(SDL2_ATK_PATH)/$(MP3_LIBRARY_PATH)
    LOCAL_CFLAGS += -DLOAD_MP3
endif

LOCAL_EXPORT_C_INCLUDES += $(SDL2_ATK_PATH)

LOCAL_SHARED_LIBRARIES := SDL2

include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL2_atk static library
#
###########################

LOCAL_MODULE := SDL2_atk_static

LOCAL_MODULE_FILENAME := libSDL2_atk

LOCAL_LDLIBS :=
LOCAL_EXPORT_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)

