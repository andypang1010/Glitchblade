LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
CUGL_OFFSET := __CUGL_PATH__

###########################
#
# WEBP static library
#
###########################
WEBP_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/sdlapp/external/libwebp

WEBP_CFLAGS := -Wall -DANDROID -DHAVE_MALLOC_H -DHAVE_PTHREAD -DWEBP_USE_THREAD
WEBP_CFLAGS += -fvisibility=hidden
WEBP_CFLAGS += -Wno-unused-but-set-variable -Wno-xor-used-as-pow

ifeq ($(APP_OPTIM),release)
  WEBP_CFLAGS += -finline-functions -ffast-math \
                 -ffunction-sections -fdata-sections
  ifeq ($(findstring clang,$(NDK_TOOLCHAIN_VERSION)),)
    WEBP_CFLAGS += -frename-registers -s
  endif
endif

# mips32 fails to build with clang from r14b
# https://bugs.chromium.org/p/webp/issues/detail?id=343
ifeq ($(findstring clang,$(NDK_TOOLCHAIN_VERSION)),clang)
  ifeq ($(TARGET_ARCH),mips)
    clang_version := $(shell $(TARGET_CC) --version)
    ifneq ($(findstring clang version 3,$(clang_version)),)
      WEBP_CFLAGS += -no-integrated-as
    endif
  endif
endif

ifneq ($(findstring armeabi-v7a, $(TARGET_ARCH_ABI)),)
  # Setting LOCAL_ARM_NEON will enable -mfpu=neon which may cause illegal
  # instructions to be generated for armv7a code. Instead target the neon code
  # specifically.
  NEON := c.neon
  USE_CPUFEATURES := yes
  WEBP_CFLAGS += -DHAVE_CPU_FEATURES_H
else
  NEON := c
endif

LOCAL_PATH := $(WEBP_PATH)/src

dec_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/dec/alpha_dec.c \
    $(LOCAL_PATH)/dec/buffer_dec.c \
    $(LOCAL_PATH)/dec/frame_dec.c \
    $(LOCAL_PATH)/dec/idec_dec.c \
    $(LOCAL_PATH)/dec/io_dec.c \
    $(LOCAL_PATH)/dec/quant_dec.c \
    $(LOCAL_PATH)/dec/tree_dec.c \
    $(LOCAL_PATH)/dec/vp8_dec.c \
    $(LOCAL_PATH)/dec/vp8l_dec.c \
    $(LOCAL_PATH)/dec/webp_dec.c)

demux_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/demux/anim_decode.c \
    $(LOCAL_PATH)/demux/demux.c)

dsp_dec_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/dsp/alpha_processing.c \
    $(LOCAL_PATH)/dsp/alpha_processing_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/alpha_processing_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/alpha_processing_sse2.c \
    $(LOCAL_PATH)/dsp/alpha_processing_sse41.c \
    $(LOCAL_PATH)/dsp/cpu.c \
    $(LOCAL_PATH)/dsp/dec.c \
    $(LOCAL_PATH)/dsp/dec_clip_tables.c \
    $(LOCAL_PATH)/dsp/dec_mips32.c \
    $(LOCAL_PATH)/dsp/dec_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/dec_msa.c \
    $(LOCAL_PATH)/dsp/dec_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/dec_sse2.c \
    $(LOCAL_PATH)/dsp/dec_sse41.c \
    $(LOCAL_PATH)/dsp/filters.c \
    $(LOCAL_PATH)/dsp/filters_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/filters_msa.c \
    $(LOCAL_PATH)/dsp/filters_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/filters_sse2.c \
    $(LOCAL_PATH)/dsp/lossless.c \
    $(LOCAL_PATH)/dsp/lossless_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/lossless_msa.c \
    $(LOCAL_PATH)/dsp/lossless_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/lossless_sse2.c \
    $(LOCAL_PATH)/dsp/rescaler.c \
    $(LOCAL_PATH)/dsp/rescaler_mips32.c \
    $(LOCAL_PATH)/dsp/rescaler_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/rescaler_msa.c \
    $(LOCAL_PATH)/dsp/rescaler_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/rescaler_sse2.c \
    $(LOCAL_PATH)/dsp/upsampling.c \
    $(LOCAL_PATH)/dsp/upsampling_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/upsampling_msa.c \
    $(LOCAL_PATH)/dsp/upsampling_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/upsampling_sse2.c \
    $(LOCAL_PATH)/dsp/upsampling_sse41.c \
    $(LOCAL_PATH)/dsp/yuv.c \
    $(LOCAL_PATH)/dsp/yuv_mips32.c \
    $(LOCAL_PATH)/dsp/yuv_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/yuv_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/yuv_sse2.c \
    $(LOCAL_PATH)/dsp/yuv_sse41.c)
    
dsp_enc_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/dsp/cost.c \
    $(LOCAL_PATH)/dsp/cost_mips32.c \
    $(LOCAL_PATH)/dsp/cost_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/cost_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/cost_sse2.c \
    $(LOCAL_PATH)/dsp/enc.c \
    $(LOCAL_PATH)/dsp/enc_mips32.c \
    $(LOCAL_PATH)/dsp/enc_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/enc_msa.c \
    $(LOCAL_PATH)/dsp/enc_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/enc_sse2.c \
    $(LOCAL_PATH)/dsp/enc_sse41.c \
    $(LOCAL_PATH)/dsp/lossless_enc.c \
    $(LOCAL_PATH)/dsp/lossless_enc_mips32.c \
    $(LOCAL_PATH)/dsp/lossless_enc_mips_dsp_r2.c \
    $(LOCAL_PATH)/dsp/lossless_enc_msa.c \
    $(LOCAL_PATH)/dsp/lossless_enc_neon.$(NEON) \
    $(LOCAL_PATH)/dsp/lossless_enc_sse2.c \
    $(LOCAL_PATH)/dsp/lossless_enc_sse41.c \
    $(LOCAL_PATH)/dsp/ssim.c \
    $(LOCAL_PATH)/dsp/ssim_sse2.c)

enc_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/enc/alpha_enc.c \
    $(LOCAL_PATH)/enc/analysis_enc.c \
    $(LOCAL_PATH)/enc/backward_references_cost_enc.c \
    $(LOCAL_PATH)/enc/backward_references_enc.c \
    $(LOCAL_PATH)/enc/config_enc.c \
    $(LOCAL_PATH)/enc/cost_enc.c \
    $(LOCAL_PATH)/enc/filter_enc.c \
    $(LOCAL_PATH)/enc/frame_enc.c \
    $(LOCAL_PATH)/enc/histogram_enc.c \
    $(LOCAL_PATH)/enc/iterator_enc.c \
    $(LOCAL_PATH)/enc/near_lossless_enc.c \
    $(LOCAL_PATH)/enc/picture_enc.c \
    $(LOCAL_PATH)/enc/picture_csp_enc.c \
    $(LOCAL_PATH)/enc/picture_psnr_enc.c \
    $(LOCAL_PATH)/enc/picture_rescale_enc.c \
    $(LOCAL_PATH)/enc/picture_tools_enc.c \
    $(LOCAL_PATH)/enc/predictor_enc.c \
    $(LOCAL_PATH)/enc/quant_enc.c \
    $(LOCAL_PATH)/enc/syntax_enc.c \
    $(LOCAL_PATH)/enc/token_enc.c \
    $(LOCAL_PATH)/enc/tree_enc.c \
    $(LOCAL_PATH)/enc/vp8l_enc.c \
    $(LOCAL_PATH)/enc/webp_enc.c)

mux_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/mux/anim_encode.c \
    $(LOCAL_PATH)/mux/muxedit.c \
    $(LOCAL_PATH)/mux/muxinternal.c \
    $(LOCAL_PATH)/mux/muxread.c)

utils_dec_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/utils/bit_reader_utils.c \
    $(LOCAL_PATH)/utils/color_cache_utils.c \
    $(LOCAL_PATH)/utils/filters_utils.c \
    $(LOCAL_PATH)/utils/huffman_utils.c \
    $(LOCAL_PATH)/utils/quant_levels_dec_utils.c \
    $(LOCAL_PATH)/utils/random_utils.c \
    $(LOCAL_PATH)/utils/rescaler_utils.c \
    $(LOCAL_PATH)/utils/thread_utils.c \
    $(LOCAL_PATH)/utils/utils.c)

utils_enc_srcs := \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/utils/bit_writer_utils.c \
    $(LOCAL_PATH)/utils/huffman_encode_utils.c \
    $(LOCAL_PATH)/utils/quant_levels_utils.c)

################################################################################
# libwebpdecoder

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(dec_srcs) \
    $(dsp_dec_srcs) \
    $(utils_dec_srcs) \

LOCAL_CFLAGS := $(WEBP_CFLAGS)
LOCAL_C_INCLUDES := $(WEBP_PATH)
LOCAL_C_INCLUDES += $(WEBP_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(WEBP_PATH)/src

# prefer arm over thumb mode for performance gains
LOCAL_ARM_MODE := arm

ifeq ($(USE_CPUFEATURES),yes)
  LOCAL_STATIC_LIBRARIES := cpufeatures
endif

LOCAL_MODULE := webpdecoder_static

include $(BUILD_STATIC_LIBRARY)

ifeq ($(ENABLE_SHARED),1)
include $(CLEAR_VARS)

LOCAL_WHOLE_STATIC_LIBRARIES := webpdecoder_static

LOCAL_MODULE := webpdecoder

include $(BUILD_SHARED_LIBRARY)
endif  # ENABLE_SHARED=1

################################################################################
# libwebp

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
    $(dsp_enc_srcs) \
    $(enc_srcs) \
    $(utils_enc_srcs) \

LOCAL_CFLAGS := $(WEBP_CFLAGS)
LOCAL_C_INCLUDES := $(WEBP_PATH)
LOCAL_C_INCLUDES += $(WEBP_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(WEBP_PATH)/src

# prefer arm over thumb mode for performance gains
LOCAL_ARM_MODE := arm

LOCAL_WHOLE_STATIC_LIBRARIES := webpdecoder_static

LOCAL_MODULE := webp

ifeq ($(ENABLE_SHARED),1)
  include $(BUILD_SHARED_LIBRARY)
else
  include $(BUILD_STATIC_LIBRARY)
endif

################################################################################
# libwebpdemux

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(demux_srcs)

LOCAL_CFLAGS := $(WEBP_CFLAGS)
LOCAL_C_INCLUDES := $(WEBP_PATH)
LOCAL_C_INCLUDES += $(WEBP_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(WEBP_PATH)/src

# prefer arm over thumb mode for performance gains
LOCAL_ARM_MODE := arm

LOCAL_MODULE := webpdemux

ifeq ($(ENABLE_SHARED),1)
  LOCAL_SHARED_LIBRARIES := webp
  include $(BUILD_SHARED_LIBRARY)
else
  LOCAL_STATIC_LIBRARIES := webp
  include $(BUILD_STATIC_LIBRARY)
endif

################################################################################
# libwebpmux

include $(CLEAR_VARS)

LOCAL_SRC_FILES := $(mux_srcs)

LOCAL_CFLAGS := $(WEBP_CFLAGS)
LOCAL_C_INCLUDES := $(WEBP_PATH)
LOCAL_C_INCLUDES += $(WEBP_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(WEBP_PATH)/src

# prefer arm over thumb mode for performance gains
LOCAL_ARM_MODE := arm

LOCAL_MODULE := webpmux

ifeq ($(ENABLE_SHARED),1)
  LOCAL_SHARED_LIBRARIES := webp
  include $(BUILD_SHARED_LIBRARY)
else
  LOCAL_STATIC_LIBRARIES := webp
  include $(BUILD_STATIC_LIBRARY)
endif

ifeq ($(USE_CPUFEATURES),yes)
  $(call import-module,android/cpufeatures)
endif
