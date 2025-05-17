LOCAL_PATH := $(call my-dir)
CURR_DEPTH   := ../../..
BOX2D_OFFSET := ../../../external/box2d

###########################
#
# Box2D static library
#
###########################
BOX2D_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(BOX2D_OFFSET)

include $(CLEAR_VARS)

LOCAL_MODULE := box2d

LOCAL_C_INCLUDES := $(BOX2D_PATH)/include
LOCAL_C_INCLUDES += $(BOX2D_PATH)/src
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

# To keep file mangling to a minimum
LOCAL_PATH = $(BOX2D_PATH)/src

# CODEC SOURCE CODE
LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/collision/*.cpp) \
	$(wildcard $(LOCAL_PATH)/common/*.cpp) \
	$(wildcard $(LOCAL_PATH)/dynamics/*.cpp) \
	$(wildcard $(LOCAL_PATH)/rope/*.cpp) \
	)
LOCAL_STATIC_LIBRARIES :=

include $(BUILD_STATIC_LIBRARY)
