LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../../..
CUGL_OFFSET := ../..

########################
#
# libjuice static library
#
########################
JUICE_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/libdatachannel/deps/libjuice
JUICE_MAKE := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := juice

LOCAL_C_INCLUDES := $(JUICE_MAKE)/android
LOCAL_C_INCLUDES += $(JUICE_PATH)/include
LOCAL_C_INCLUDES += $(JUICE_PATH)/include/juice
LOCAL_C_INCLUDES += $(JUICE_PATH)/src

# The defines
LOCAL_CFLAGS += -DJUICE_STATIC
LOCAL_CFLAGS += -DUSE_NETTLE=0 
LOCAL_CFLAGS += -DJUICE_HAS_EXPORT_HEADER 
LOCAL_CFLAGS += -Djuice_EXPORTS

# Compilation flags
LOCAL_CFLAGS += -fPIC
LOCAL_CFLAGS += -fno-limit-debug-info
LOCAL_CFLAGS += -fvisibility=hidden
LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -Wextra
LOCAL_CFLAGS += -pthread

# Add your application source files here.
LOCAL_PATH = $(JUICE_PATH)

LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/src/addr.c \
	$(LOCAL_PATH)/src/agent.c \
	$(LOCAL_PATH)/src/crc32.c \
	$(LOCAL_PATH)/src/const_time.c \
	$(LOCAL_PATH)/src/conn.c \
	$(LOCAL_PATH)/src/conn_poll.c \
	$(LOCAL_PATH)/src/conn_thread.c \
	$(LOCAL_PATH)/src/conn_mux.c \
	$(LOCAL_PATH)/src/base64.c \
	$(LOCAL_PATH)/src/hash.c \
	$(LOCAL_PATH)/src/hmac.c \
	$(LOCAL_PATH)/src/ice.c \
	$(LOCAL_PATH)/src/juice.c \
	$(LOCAL_PATH)/src/log.c \
	$(LOCAL_PATH)/src/random.c \
	$(LOCAL_PATH)/src/server.c \
	$(LOCAL_PATH)/src/stun.c \
	$(LOCAL_PATH)/src/timestamp.c \
	$(LOCAL_PATH)/src/turn.c \
	$(LOCAL_PATH)/src/udp.c)

include $(BUILD_STATIC_LIBRARY)