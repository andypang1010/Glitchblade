LOCAL_PATH   := $(call my-dir)
CURR_DEPTH   := ../..
SDL2_OFFSET  := ../../../external/SDL_app/components/SDL
IMAGE_OFFSET := ../../../sdlapp

###########################
#
# SDL_Image shared library
#
###########################
SDL2_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(SDL2_OFFSET)
SDL2_IMAGE_MAKE := $(LOCAL_PATH)
SDL2_IMAGE_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(IMAGE_OFFSET)
SDL2_IMAGE_SOURCE  := $(SDL2_IMAGE_PATH)/src/image
SDL2_IMAGE_INCLUDE := $(SDL2_IMAGE_PATH)/include

# Enable this if you want PNG and JPG support with minimal dependencies
USE_STBIMAGE ?= true

# The additional formats below require downloading third party dependencies,
# using the external/download.sh script.

# Enable this if you want to support loading AVIF images
# The library path should be a relative path to this directory.
SUPPORT_AVIF ?= false
AVIF_LIBRARY_PATH := external/libavif
DAV1D_LIBRARY_PATH := external/dav1d

# Enable this if you want to support loading JPEG images using libjpeg
# The library path should be a relative path to this directory.
SUPPORT_JPG ?= false
SUPPORT_SAVE_JPG ?= true
JPG_LIBRARY_PATH := external/jpeg

# Enable this if you want to support loading JPEG-XL images
# The library path should be a relative path to this directory.
SUPPORT_JXL ?= false
JXL_LIBRARY_PATH := external/libjxl

# Enable this if you want to support loading PNG images using libpng
# The library path should be a relative path to this directory.
SUPPORT_PNG ?= false
SUPPORT_SAVE_PNG ?= true
PNG_LIBRARY_PATH := external/libpng

# Enable this if you want to support loading WebP images
# The library path should be a relative path to this directory.
SUPPORT_WEBP ?= true
WEBP_LIBRARY_PATH := external/libwebp

# NOTE: There is no TIFF support in this build system
# Use CMake for proper TIFF support

# Build the library
ifeq ($(SUPPORT_AVIF),true)
    include $(SDL2_IMAGE_MAKE)/$(AVIF_LIBRARY_PATH)/Android.mk
    include $(SDL2_IMAGE_MAKE)/$(DAV1D_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_JPG),true)
    include $(SDL2_IMAGE_MAKE)/$(JPG_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_JXL),true)
    include $(SDL2_IMAGE_MAKE)/$(JXL_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_PNG),true)
    include $(SDL2_IMAGE_MAKE)/$(PNG_LIBRARY_PATH)/Android.mk
endif

# Build the library
ifeq ($(SUPPORT_WEBP),true)
    include $(SDL2_IMAGE_MAKE)/$(WEBP_LIBRARY_PATH)/Android.mk
endif


# Restore local path
LOCAL_PATH := $(SDL2_IMAGE_SOURCE)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_image

LOCAL_SRC_FILES :=  \
	$(subst $(LOCAL_PATH)/,, \
    $(LOCAL_PATH)/IMG.c           \
    $(LOCAL_PATH)/IMG_avif.c      \
    $(LOCAL_PATH)/IMG_bmp.c       \
    $(LOCAL_PATH)/IMG_gif.c       \
    $(LOCAL_PATH)/IMG_jpg.c       \
    $(LOCAL_PATH)/IMG_jxl.c       \
    $(LOCAL_PATH)/IMG_lbm.c       \
    $(LOCAL_PATH)/IMG_pcx.c       \
    $(LOCAL_PATH)/IMG_png.c       \
    $(LOCAL_PATH)/IMG_pnm.c       \
    $(LOCAL_PATH)/IMG_qoi.c       \
    $(LOCAL_PATH)/IMG_stb.c       \
    $(LOCAL_PATH)/IMG_svg.c       \
    $(LOCAL_PATH)/IMG_tga.c       \
    $(LOCAL_PATH)/IMG_tif.c       \
    $(LOCAL_PATH)/IMG_webp.c      \
    $(LOCAL_PATH)/IMG_WIC.c       \
    $(LOCAL_PATH)/IMG_xcf.c       \
    $(LOCAL_PATH)/IMG_xpm.c.arm   \
    $(LOCAL_PATH)/IMG_xv.c        \
    $(LOCAL_PATH)/IMG_xxx.c)

LOCAL_C_INCLUDES := $(SDL2_IMAGE_INCLUDE)
LOCAL_CFLAGS := -DLOAD_BMP -DLOAD_GIF -DLOAD_LBM -DLOAD_PCX -DLOAD_PNM \
                -DLOAD_SVG -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM -DLOAD_XV  \
                -DLOAD_QOI
LOCAL_LDLIBS :=
LOCAL_STATIC_LIBRARIES :=
LOCAL_SHARED_LIBRARIES := SDL2

ifeq ($(USE_STBIMAGE),true)
    LOCAL_CFLAGS += -DLOAD_JPG -DLOAD_PNG -DUSE_STBIMAGE
endif

ifeq ($(SUPPORT_AVIF),true)
    LOCAL_C_INCLUDES += $(SDL2_IMAGE_PATH)/$(AVIF_LIBRARY_PATH)/include
    LOCAL_CFLAGS += -DLOAD_AVIF
    LOCAL_STATIC_LIBRARIES += avif
    LOCAL_WHOLE_STATIC_LIBRARIES += dav1d dav1d-8bit dav1d-16bit
endif

ifeq ($(SUPPORT_JPG),true)
    LOCAL_C_INCLUDES += $(SDL2_IMAGE_PATH)/$(JPG_LIBRARY_PATH)
    LOCAL_CFLAGS += -DLOAD_JPG
    LOCAL_STATIC_LIBRARIES += jpeg
ifeq ($(SUPPORT_SAVE_JPG),true)
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_JPG=1
else
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_JPG=0
endif
endif

ifeq ($(SUPPORT_JXL),true)
    LOCAL_C_INCLUDES += $(SDL2_IMAGE_PATH)/$(JXL_LIBRARY_PATH)/lib/include \
                        $(SDL2_IMAGE_PATH)/$(JXL_LIBRARY_PATH)/android
    LOCAL_CFLAGS += -DLOAD_JXL
    LOCAL_STATIC_LIBRARIES += jxl
endif

ifeq ($(SUPPORT_PNG),true)
    LOCAL_C_INCLUDES += $(SDL2_IMAGE_PATH)/$(PNG_LIBRARY_PATH)
    LOCAL_CFLAGS += -DLOAD_PNG
    LOCAL_STATIC_LIBRARIES += png
    LOCAL_LDLIBS += -lz
ifeq ($(SUPPORT_SAVE_PNG),true)
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_PNG=1
else
    LOCAL_CFLAGS += -DSDL_IMAGE_SAVE_PNG=0
endif
endif

ifeq ($(SUPPORT_WEBP),true)
    LOCAL_C_INCLUDES += $(SDL2_IMAGE_PATH)/$(WEBP_LIBRARY_PATH)/src
    LOCAL_CFLAGS += -DLOAD_WEBP
    LOCAL_STATIC_LIBRARIES += webp
    LOCAL_STATIC_LIBRARIES += webpdemux
endif

LOCAL_EXPORT_C_INCLUDES += $(SDL2_IMAGE_INCLUDE)

include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL2_image static library
#
###########################

LOCAL_MODULE := SDL2_image_static

LOCAL_MODULE_FILENAME := libSDL2_image

LOCAL_LDLIBS :=
LOCAL_EXPORT_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)

