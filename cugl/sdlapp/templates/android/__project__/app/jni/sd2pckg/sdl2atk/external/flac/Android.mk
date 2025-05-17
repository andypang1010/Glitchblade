LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
ATK_OFFSET  := __SDL2_PATH__

###########################
#
# FLAC static library
#
###########################
FLAC_MAKE := $(LOCAL_PATH)
FLAC_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(ATK_OFFSET)/external/flac
OGG_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(ATK_OFFSET)/external/ogg

include $(CLEAR_VARS)

LOCAL_MODULE := flac

LOCAL_C_INCLUDES += $(OGG_PATH)/include
LOCAL_C_INCLUDES += $(FLAC_PATH)/include
LOCAL_C_INCLUDES += $(FLAC_PATH)/src/libFLAC/include
LOCAL_C_INCLUDES += $(FLAC_MAKE)/include/$(TARGET_ARCH_ABI)

LOCAL_CFLAGS += -fPIC -fassociative-math 
LOCAL_CFLAGS += -fkeep-inline-functions
LOCAL_CFLAGS += -fno-signed-zeros
LOCAL_CFLAGS += -fno-trapping-math
LOCAL_CFLAGS += -freciprocal-math
LOCAL_CFLAGS += -DHAVE_CONFIG_H
LOCAL_CFLAGS += -DFLAC__NO_DLL
LOCAL_CFLAGS += -D_POSIX_PTHREAD_SEMANTICS 
LOCAL_CFLAGS += -D__STDC_WANT_IEC_60559_BFP_EXT__
LOCAL_CFLAGS += -D__STDC_WANT_IEC_60559_FUNCS_EXT__
LOCAL_CFLAGS += -D__STDC_WANT_IEC_60559_TYPES_EXT__
LOCAL_CFLAGS += -D__STDC_WANT_LIB_EXT2__ 
LOCAL_CFLAGS += -D__STDC_WANT_MATH_SPEC_FUNCS__
LOCAL_CFLAGS += -D'_FORTIFY_SOURCE=2'

ifeq ($(TARGET_ARCH_ABI),armeabi-v7a)
	LOCAL_CFLAGS += -D_ARM_ASSEM_
endif
ifeq ($(TARGET_ARCH_ABI),arm64-v8a)
	LOCAL_CFLAGS += -D_ARM_ASSEM_
endif

# To keep file mangling to a minimum
LOCAL_PATH = $(FLAC_PATH)/src/libFLAC

# FLAC SOURCE CODE
LOCAL_SRC_FILES += \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/bitmath.c \
	$(LOCAL_PATH)/bitreader.c \
	$(LOCAL_PATH)/bitwriter.c \
	$(LOCAL_PATH)/cpu.c \
	$(LOCAL_PATH)/crc.c \
	$(LOCAL_PATH)/fixed.c \
	$(LOCAL_PATH)/fixed_intrin_avx2.c \
	$(LOCAL_PATH)/fixed_intrin_sse2.c \
	$(LOCAL_PATH)/fixed_intrin_sse42.c \
	$(LOCAL_PATH)/fixed_intrin_ssse3.c \
	$(LOCAL_PATH)/float.c \
	$(LOCAL_PATH)/format.c \
	$(LOCAL_PATH)/lpc.c \
	$(LOCAL_PATH)/lpc_intrin_avx2.c \
	$(LOCAL_PATH)/lpc_intrin_fma.c \
	$(LOCAL_PATH)/lpc_intrin_neon.c \
	$(LOCAL_PATH)/lpc_intrin_sse2.c \
	$(LOCAL_PATH)/lpc_intrin_sse41.c \
	$(LOCAL_PATH)/md5.c \
	$(LOCAL_PATH)/memory.c \
	$(LOCAL_PATH)/metadata_iterators.c \
	$(LOCAL_PATH)/metadata_object.c \
	$(LOCAL_PATH)/ogg_decoder_aspect.c \
	$(LOCAL_PATH)/ogg_encoder_aspect.c \
	$(LOCAL_PATH)/ogg_helper.c \
	$(LOCAL_PATH)/ogg_mapping.c \
	$(LOCAL_PATH)/stream_decoder.c \
	$(LOCAL_PATH)/stream_encoder.c \
	$(LOCAL_PATH)/stream_encoder_framing.c \
	$(LOCAL_PATH)/stream_encoder_intrin_avx2.c \
	$(LOCAL_PATH)/stream_encoder_intrin_sse2.c \
	$(LOCAL_PATH)/stream_encoder_intrin_ssse3.c \
	$(LOCAL_PATH)/window.c)


LOCAL_STATIC_LIBRARIES += ogg

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_STATIC_LIBRARY)
