LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ..
CUGL_OFFSET := __CUGL_PATH__

###########################
#
# CUGL static library
#
###########################
CUGL_MAKE := $(LOCAL_PATH)
CUGL_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)
SDL2_PATH := $(CUGL_PATH)/sdlapp

CUGL_INCLUDES := $(CUGL_PATH)/include
CUGL_INCLUDES += $(SDL2_PATH)/include
CUGL_INCLUDES += $(CUGL_PATH)/external/poly2tri
CUGL_INCLUDES += $(CUGL_PATH)/external/box2d/include
CUGL_INCLUDES += $(CUGL_PATH)/external/libdatachannel/include


# CUGL Core
include $(CUGL_MAKE)/external/poly2tri/Android.mk

include $(CLEAR_VARS)

LOCAL_MODULE := CUGL-Core
LOCAL_C_INCLUDES := $(CUGL_INCLUDES)

LOCAL_PATH := $(CUGL_PATH)/source

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/core/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/actions/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/assets/*.c) \
	$(wildcard $(LOCAL_PATH)/core/assets/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/input/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/io/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/math/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/math/polygon/*.cpp) \
	$(wildcard $(LOCAL_PATH)/core/util/*.cpp))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_CFLAGS += \
	-Wno-missing-prototypes \
	-Wno-implicit-const-int-float-conversion \

LOCAL_STATIC_LIBRARIES := poly2tri

LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_app
LOCAL_SHARED_LIBRARIES += SDL2_atk
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += SDL2_ttf

include $(BUILD_STATIC_LIBRARY)


# CUGL GRAPHICS
include $(CLEAR_VARS)

LOCAL_MODULE := CUGL-Graphics
LOCAL_C_INCLUDES := $(CUGL_INCLUDES)

LOCAL_PATH := $(CUGL_PATH)/source

LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/graphics/*.cpp) \
	$(wildcard $(LOCAL_PATH)/graphics/loaders/*.cpp))

LOCAL_CFLAGS += -DGL_GLEXT_PROTOTYPES
LOCAL_CFLAGS += \
	-Wno-missing-prototypes \
	-Wno-implicit-const-int-float-conversion \
	
LOCAL_SHARED_LIBRARIES := SDL2
LOCAL_SHARED_LIBRARIES += SDL2_app
LOCAL_SHARED_LIBRARIES += SDL2_atk
LOCAL_SHARED_LIBRARIES += SDL2_image
LOCAL_SHARED_LIBRARIES += SDL2_ttf

include $(BUILD_STATIC_LIBRARY)


# CUGL Audio
SUPPORT_AUDIO ?= true
ifeq ($(SUPPORT_AUDIO),true)
	include $(CLEAR_VARS)
	LOCAL_MODULE := CUGL-Audio
	LOCAL_C_INCLUDES := $(CUGL_INCLUDES)
	
	LOCAL_PATH := $(CUGL_PATH)/source

	LOCAL_SRC_FILES := \
		$(subst $(LOCAL_PATH)/,, \
		$(wildcard $(LOCAL_PATH)/audio/*.cpp) \
		$(wildcard $(LOCAL_PATH)/audio/graph/*.cpp))

	LOCAL_CFLAGS += \
		-Wno-missing-prototypes \
		-Wno-implicit-const-int-float-conversion \

	LOCAL_SHARED_LIBRARIES := SDL2
	LOCAL_SHARED_LIBRARIES += SDL2_app
	LOCAL_SHARED_LIBRARIES += SDL2_atk
	LOCAL_SHARED_LIBRARIES += SDL2_image
	LOCAL_SHARED_LIBRARIES += SDL2_ttf

	include $(BUILD_STATIC_LIBRARY)
endif


# CUGL SCENE2
SUPPORT_SCENE2 ?= true
ifeq ($(SUPPORT_SCENE2),true)
	include $(CLEAR_VARS)
	LOCAL_MODULE := CUGL-Scene2
	LOCAL_C_INCLUDES := $(CUGL_INCLUDES)
	
	LOCAL_PATH := $(CUGL_PATH)/source

	LOCAL_SRC_FILES := \
		$(subst $(LOCAL_PATH)/,, \
		$(wildcard $(LOCAL_PATH)/scene2/*.cpp) \
		$(wildcard $(LOCAL_PATH)/scene2/layout/*.cpp))

	LOCAL_CFLAGS += \
		-Wno-missing-prototypes \
		-Wno-implicit-const-int-float-conversion \

	LOCAL_SHARED_LIBRARIES := SDL2
	LOCAL_SHARED_LIBRARIES += SDL2_app
	LOCAL_SHARED_LIBRARIES += SDL2_atk
	LOCAL_SHARED_LIBRARIES += SDL2_image
	LOCAL_SHARED_LIBRARIES += SDL2_ttf

	include $(BUILD_STATIC_LIBRARY)
endif


# CUGL SCENE3
SUPPORT_SCENE3 ?= true
ifeq ($(SUPPORT_SCENE3),true)
	include $(CLEAR_VARS)
	LOCAL_MODULE := CUGL-Scene3
	LOCAL_C_INCLUDES := $(CUGL_INCLUDES)
	
	LOCAL_PATH := $(CUGL_PATH)/source

	LOCAL_SRC_FILES := \
		$(subst $(LOCAL_PATH)/,, \
		$(wildcard $(LOCAL_PATH)/scene3/*.cpp))

	LOCAL_CFLAGS += \
		-Wno-missing-prototypes \
		-Wno-implicit-const-int-float-conversion \

	LOCAL_SHARED_LIBRARIES := SDL2
	LOCAL_SHARED_LIBRARIES += SDL2_app
	LOCAL_SHARED_LIBRARIES += SDL2_atk
	LOCAL_SHARED_LIBRARIES += SDL2_image
	LOCAL_SHARED_LIBRARIES += SDL2_ttf

	include $(BUILD_STATIC_LIBRARY)
endif


# CUGL PHYSICS2
SUPPORT_PHYSICS2 ?= true
ifeq ($(SUPPORT_PHYSICS2),true)
	include $(CUGL_MAKE)/external/box2d/Android.mk

	include $(CLEAR_VARS)
	
	LOCAL_MODULE := CUGL-Physics2
	LOCAL_C_INCLUDES := $(CUGL_INCLUDES)
	
	LOCAL_PATH := $(CUGL_PATH)/source

	LOCAL_SRC_FILES := \
		$(subst $(LOCAL_PATH)/,, \
		$(wildcard $(LOCAL_PATH)/physics2/*.cpp))

	LOCAL_CFLAGS += \
		-Wno-missing-prototypes \
		-Wno-implicit-const-int-float-conversion \

	LOCAL_SHARED_LIBRARIES := SDL2
	LOCAL_SHARED_LIBRARIES += SDL2_app
	LOCAL_SHARED_LIBRARIES += SDL2_atk
	LOCAL_SHARED_LIBRARIES += SDL2_image
	LOCAL_SHARED_LIBRARIES += SDL2_ttf

	LOCAL_STATIC_LIBRARIES := box2d
	include $(BUILD_STATIC_LIBRARY)
endif


# CUGL NETCODE
SUPPORT_NETCODE ?= true
ifeq ($(SUPPORT_NETCODE),true)
	include $(CUGL_MAKE)/external/libdatachannel/Android.mk

	include $(CLEAR_VARS)
	
	LOCAL_MODULE := CUGL-Netcode
	LOCAL_C_INCLUDES := $(CUGL_INCLUDES)
	
	LOCAL_PATH := $(CUGL_PATH)/source

	LOCAL_SRC_FILES := \
		$(subst $(LOCAL_PATH)/,, \
		$(wildcard $(LOCAL_PATH)/netcode/*.cpp))

	LOCAL_CFLAGS += \
		-Wno-missing-prototypes \
		-Wno-implicit-const-int-float-conversion \

	LOCAL_SHARED_LIBRARIES := SDL2
	LOCAL_SHARED_LIBRARIES += SDL2_app
	LOCAL_SHARED_LIBRARIES += SDL2_atk
	LOCAL_SHARED_LIBRARIES += SDL2_image
	LOCAL_SHARED_LIBRARIES += SDL2_ttf

	LOCAL_STATIC_LIBRARIES := datachannel
	include $(BUILD_STATIC_LIBRARY)
endif


# CUGL DISTRIB PHYSICS2
SUPPORT_DISTRIB_PHYSICS2 ?= true
ifeq ($(SUPPORT_DISTRIB_PHYSICS2),true)
	include $(CLEAR_VARS)
	LOCAL_MODULE := CUGL-Distrib-Physics2
	LOCAL_C_INCLUDES := $(CUGL_INCLUDES)
	
	LOCAL_PATH := $(CUGL_PATH)/source

	LOCAL_SRC_FILES := \
		$(subst $(LOCAL_PATH)/,, \
		$(wildcard $(LOCAL_PATH)/physics2/distrib/*.cpp))

	LOCAL_CFLAGS += \
		-Wno-missing-prototypes \
		-Wno-implicit-const-int-float-conversion \

	LOCAL_SHARED_LIBRARIES := SDL2
	LOCAL_SHARED_LIBRARIES += SDL2_app
	LOCAL_SHARED_LIBRARIES += SDL2_atk
	LOCAL_SHARED_LIBRARIES += SDL2_image
	LOCAL_SHARED_LIBRARIES += SDL2_ttf

	include $(BUILD_STATIC_LIBRARY)
endif


# CUGL
include $(CLEAR_VARS)

LOCAL_MODULE := CUGL

LOCAL_STATIC_LIBRARIES := CUGL-Core
LOCAL_STATIC_LIBRARIES += CUGL-Graphics
ifeq ($(SUPPORT_AUDIO),true)
	LOCAL_STATIC_LIBRARIES += CUGL-Audio
endif
ifeq ($(SUPPORT_SCENE2),true)
	LOCAL_STATIC_LIBRARIES += CUGL-Scene2
endif
ifeq ($(SUPPORT_SCENE3),true)
	LOCAL_STATIC_LIBRARIES += CUGL-Scene3
endif
ifeq ($(SUPPORT_PHYSICS2),true)
	LOCAL_STATIC_LIBRARIES += CUGL-Physics2
endif
ifeq ($(SUPPORT_NETCODE),true)
	LOCAL_STATIC_LIBRARIES += CUGL-Netcode
endif
ifeq ($(SUPPORT_DISTRIB_PHYSICS2),true)
	LOCAL_STATIC_LIBRARIES += CUGL-Distrib-Physics2
endif

include $(BUILD_STATIC_LIBRARY)
