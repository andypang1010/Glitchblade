LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../..
SDL2_OFFSET := ../../../sdlapp
APP_OFFSET  := ../../../sdlapp

###########################
#
# SDL_app shared library
#
###########################
SDL2_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(SDL2_OFFSET)
SDL2_APP_MAKE   := $(LOCAL_PATH)
SDL2_APP_PATH   := $(LOCAL_PATH)/$(CURR_DEPTH)/$(APP_OFFSET)
SDL2_APP_SOURCE := $(SDL2_APP_PATH)/src/app

# Keep file mangling at a minimum
LOCAL_PATH := $(SDL2_APP_SOURCE)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_app

LOCAL_C_INCLUDES := $(SDL2_PATH)/include
LOCAL_C_INCLUDES += $(SDL2_APP_PATH)/include

LOCAL_SRC_FILES :=  \
	$(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/APP_display.c \
	$(LOCAL_PATH)/APP_version.c \
	$(LOCAL_PATH)/android/APP_sysdisplay.c)

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

include $(BUILD_SHARED_LIBRARY)

###########################
#
# SDL2_app static library
#
###########################

LOCAL_MODULE := SDL2_app_static

LOCAL_MODULE_FILENAME := libSDL2_app

LOCAL_LDLIBS :=
LOCAL_EXPORT_LDLIBS :=

include $(BUILD_STATIC_LIBRARY)

