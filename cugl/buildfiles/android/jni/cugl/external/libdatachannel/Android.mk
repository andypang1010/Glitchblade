LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../..
CUGL_OFFSET := ../..

########################
#
# libdatachannel shared library
#
########################
DATA_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/libdatachannel
OSSL_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/openssl
DATA_MAKE := $(LOCAL_PATH)
OSSL_MAKE := $(LOCAL_PATH)/openssl

# Build the support libraries first
include $(DATA_MAKE)/libjuice/Android.mk
include $(DATA_MAKE)/libsrtp/Android.mk
include $(DATA_MAKE)/openssl/Android.mk
include $(DATA_MAKE)/usrsctp/Android.mk

include $(CLEAR_VARS)

LOCAL_MODULE := datachannel

LOCAL_C_INCLUDES := $(DATA_PATH)/include
LOCAL_C_INCLUDES += $(DATA_PATH)/include/rtc
LOCAL_C_INCLUDES += $(DATA_PATH)/src
LOCAL_C_INCLUDES += $(DATA_PATH)/deps/usrsctp/usrsctplib
LOCAL_C_INCLUDES += $(DATA_PATH)/deps/plog/include
LOCAL_C_INCLUDES += $(DATA_PATH)/deps/libsrtp/crypto/include
LOCAL_C_INCLUDES += $(DATA_PATH)/deps/libsrtp/include
LOCAL_C_INCLUDES += $(DATA_PATH)/deps/libjuice/include
LOCAL_C_INCLUDES += $(OSSL_PATH)
LOCAL_C_INCLUDES += $(OSSL_PATH)/include
LOCAL_C_INCLUDES += $(DATA_MAKE)/android/$(TARGET_ARCH_ABI)/include
LOCAL_C_INCLUDES += $(OSSL_MAKE)/android/$(TARGET_ARCH_ABI)/include

# The defines
LOCAL_CFLAGS += -DRTC_EXPORTS
LOCAL_CFLAGS += -DRTC_STATIC
LOCAL_CFLAGS += -DRTC_ENABLE_WEBSOCKET=1
LOCAL_CFLAGS += -DRTC_ENABLE_MEDIA=1
LOCAL_CFLAGS += -DRTC_SYSTEM_SRTP=0
LOCAL_CFLAGS += -DUSE_GNUTLS=0
LOCAL_CFLAGS += -DUSE_NICE=0
LOCAL_CFLAGS += -DRTC_SYSTEM_JUICE=0
LOCAL_CFLAGS += -DJUICE_STATIC

# Compilation flags
LOCAL_CFLAGS += -fPIC
LOCAL_CFLAGS += -fno-limit-debug-info
LOCAL_CFLAGS += -fvisibility=default
LOCAL_CFLAGS += -Wall
LOCAL_CFLAGS += -Wextra
LOCAL_CFLAGS += -pthread
LOCAL_CFLAGS += -std=gnu++17

# Add your application source files here.
LOCAL_PATH = $(DATA_PATH)/src
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/av1rtppacketizer.cpp \
	$(LOCAL_PATH)/candidate.cpp \
	$(LOCAL_PATH)/capi.cpp \
	$(LOCAL_PATH)/channel.cpp \
	$(LOCAL_PATH)/configuration.cpp \
	$(LOCAL_PATH)/datachannel.cpp \
	$(LOCAL_PATH)/description.cpp \
	$(LOCAL_PATH)/global.cpp \
	$(LOCAL_PATH)/h264rtpdepacketizer.cpp \
	$(LOCAL_PATH)/h264rtppacketizer.cpp \
	$(LOCAL_PATH)/h265nalunit.cpp \
	$(LOCAL_PATH)/h265rtppacketizer.cpp \
	$(LOCAL_PATH)/mediahandler.cpp \
	$(LOCAL_PATH)/message.cpp \
	$(LOCAL_PATH)/nalunit.cpp \
	$(LOCAL_PATH)/pacinghandler.cpp \
	$(LOCAL_PATH)/peerconnection.cpp \
	$(LOCAL_PATH)/rembhandler.cpp \
	$(LOCAL_PATH)/rtcpnackresponder.cpp \
	$(LOCAL_PATH)/rtcpreceivingsession.cpp \
	$(LOCAL_PATH)/rtcpsrreporter.cpp \
	$(LOCAL_PATH)/rtppacketizationconfig.cpp \
	$(LOCAL_PATH)/rtpdepacketizer.cpp \
	$(LOCAL_PATH)/rtppacketizer.cpp \
	$(LOCAL_PATH)/rtp.cpp \
	$(LOCAL_PATH)/track.cpp \
	$(LOCAL_PATH)/websocket.cpp \
	$(LOCAL_PATH)/websocketserver.cpp \
	$(LOCAL_PATH)/impl/certificate.cpp \
	$(LOCAL_PATH)/impl/channel.cpp \
	$(LOCAL_PATH)/impl/datachannel.cpp \
	$(LOCAL_PATH)/impl/dtlssrtptransport.cpp \
	$(LOCAL_PATH)/impl/dtlstransport.cpp \
	$(LOCAL_PATH)/impl/http.cpp \
	$(LOCAL_PATH)/impl/httpproxytransport.cpp \
	$(LOCAL_PATH)/impl/icetransport.cpp \
	$(LOCAL_PATH)/impl/init.cpp \
	$(LOCAL_PATH)/impl/logcounter.cpp \
	$(LOCAL_PATH)/impl/peerconnection.cpp \
	$(LOCAL_PATH)/impl/pollinterrupter.cpp \
	$(LOCAL_PATH)/impl/pollservice.cpp \
	$(LOCAL_PATH)/impl/processor.cpp \
	$(LOCAL_PATH)/impl/sctptransport.cpp \
	$(LOCAL_PATH)/impl/sha.cpp \
	$(LOCAL_PATH)/impl/tcpserver.cpp \
	$(LOCAL_PATH)/impl/tcptransport.cpp \
	$(LOCAL_PATH)/impl/threadpool.cpp \
	$(LOCAL_PATH)/impl/tls.cpp \
	$(LOCAL_PATH)/impl/tlstransport.cpp \
	$(LOCAL_PATH)/impl/track.cpp \
	$(LOCAL_PATH)/impl/transport.cpp \
	$(LOCAL_PATH)/impl/utils.cpp \
	$(LOCAL_PATH)/impl/verifiedtlstransport.cpp \
	$(LOCAL_PATH)/impl/websocket.cpp \
	$(LOCAL_PATH)/impl/websocketserver.cpp \
	$(LOCAL_PATH)/impl/wstransport.cpp \
	$(LOCAL_PATH)/impl/wshandshake.cpp)

# Pull in all the dependencies
LOCAL_STATIC_LIBRARIES := crypto
LOCAL_STATIC_LIBRARIES += ssl
LOCAL_STATIC_LIBRARIES += usrsctp
LOCAL_STATIC_LIBRARIES += srtp2
LOCAL_STATIC_LIBRARIES += juice

include $(BUILD_STATIC_LIBRARY)