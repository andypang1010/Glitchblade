LOCAL_PATH := $(call my-dir)
CURR_DEPTH   := ../../..
POLY2_OFFSET := ../../../external/poly2tri

###########################
#
# poly2Tri static library
#
###########################
POLY2_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(POLY2_OFFSET)

include $(CLEAR_VARS)

LOCAL_MODULE := poly2tri

LOCAL_C_INCLUDES := $(POLY2_PATH)
LOCAL_EXPORT_C_INCLUDES += $(LOCAL_C_INCLUDES)

# To keep file mangling to a minimum
LOCAL_PATH = $(POLY2_PATH)/poly2tri

# CODEC SOURCE CODE
LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/common/*.cc) \
	$(wildcard $(LOCAL_PATH)/sweep/*.cc) \
	)
LOCAL_STATIC_LIBRARIES :=

include $(BUILD_STATIC_LIBRARY)
