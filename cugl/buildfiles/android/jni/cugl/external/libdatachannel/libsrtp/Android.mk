LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../../..
CUGL_OFFSET := ../..

########################
#
# libsrtp static library
#
########################
SRTP_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/libdatachannel/deps/libsrtp
OSSL_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/openssl
SRTP_MAKE := $(LOCAL_PATH)
OSSL_MAKE := $(LOCAL_PATH)/../openssl

include $(CLEAR_VARS)

LOCAL_MODULE := srtp2

LOCAL_C_INCLUDES := $(SRTP_MAKE)/android/$(TARGET_ARCH_ABI)
LOCAL_C_INCLUDES += $(OSSL_MAKE)/android/$(TARGET_ARCH_ABI)/crypto
LOCAL_C_INCLUDES += $(OSSL_MAKE)/android/$(TARGET_ARCH_ABI)/include
LOCAL_C_INCLUDES += $(SRTP_PATH)/include
LOCAL_C_INCLUDES += $(SRTP_PATH)/crypto/include
LOCAL_C_INCLUDES += $(OSSL_PATH)
LOCAL_C_INCLUDES += $(OSSL_PATH)/include
LOCAL_C_INCLUDES += $(OSSL_PATH)/crypto/ec/curve448/arch_32
LOCAL_C_INCLUDES += $(OSSL_PATH)/crypto/ec/curve448
LOCAL_C_INCLUDES += $(OSSL_PATH)/crypto/modes

# The defines
LOCAL_CFLAGS += -DHAVE_CONFIG_H 
LOCAL_CFLAGS += -DPACKAGE_VERSION=\"2.4.2\" 

# Compilation flags
LOCAL_CFLAGS += -fPIC
LOCAL_CFLAGS += -fno-limit-debug-info

# Add your application source files here.
LOCAL_PATH = $(SRTP_PATH)
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/srtp/srtp.c \
	$(LOCAL_PATH)/crypto/cipher/cipher.c \
	$(LOCAL_PATH)/crypto/cipher/cipher_test_cases.c \
	$(LOCAL_PATH)/crypto/cipher/null_cipher.c \
	$(LOCAL_PATH)/crypto/cipher/aes_icm_ossl.c \
	$(LOCAL_PATH)/crypto/cipher/aes_gcm_ossl.c \
	$(LOCAL_PATH)/crypto/hash/auth.c \
	$(LOCAL_PATH)/crypto/hash/auth_test_cases.c \
	$(LOCAL_PATH)/crypto/hash/null_auth.c \
	$(LOCAL_PATH)/crypto/hash/hmac_ossl.c \
	$(LOCAL_PATH)/crypto/kernel/alloc.c \
	$(LOCAL_PATH)/crypto/kernel/crypto_kernel.c \
	$(LOCAL_PATH)/crypto/kernel/err.c \
	$(LOCAL_PATH)/crypto/kernel/key.c \
	$(LOCAL_PATH)/crypto/math/datatypes.c \
	$(LOCAL_PATH)/crypto/replay/rdb.c \
	$(LOCAL_PATH)/crypto/replay/rdbx.c)

include $(BUILD_STATIC_LIBRARY)