LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../../..
CUGL_OFFSET := ../..

########################
#
# crypto static library
#
########################
OSSL_PATH  := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/openssl
OSSL_MAKE  := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := crypto

LOCAL_C_INCLUDES := $(OSSL_PATH)
LOCAL_C_INCLUDES += $(OSSL_PATH)/include
LOCAL_C_INCLUDES += $(OSSL_PATH)/crypto/ec/curve448/arch_32
LOCAL_C_INCLUDES += $(OSSL_PATH)/crypto/ec/curve448
LOCAL_C_INCLUDES += $(OSSL_PATH)/crypto/modes
LOCAL_C_INCLUDES += $(OSSL_MAKE)/android/$(TARGET_ARCH_ABI)/crypto
LOCAL_C_INCLUDES += $(OSSL_MAKE)/android/$(TARGET_ARCH_ABI)/include

# The defines
LOCAL_CFLAGS += -DENGINESDIR=\"/usr/local/engines-1.1\" 
LOCAL_CFLAGS += -DOPENSSLDIR=\"/usr/local/ssl\" 
LOCAL_CFLAGS += -DOPENSSL_NO_ASM 
LOCAL_CFLAGS += -DOPENSSL_NO_STATIC_ENGINE 

# Compilation flags
LOCAL_CFLAGS += -fPIC 
LOCAL_CFLAGS += -fno-limit-debug-info 

# Add your application source files here.
LOCAL_PATH = $(OSSL_PATH)/crypto
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/aes/aes_cbc.c \
	$(LOCAL_PATH)/aes/aes_cfb.c \
	$(LOCAL_PATH)/aes/aes_core.c \
	$(LOCAL_PATH)/aes/aes_ecb.c \
	$(LOCAL_PATH)/aes/aes_ige.c \
	$(LOCAL_PATH)/aes/aes_misc.c \
	$(LOCAL_PATH)/aes/aes_ofb.c \
	$(LOCAL_PATH)/aes/aes_wrap.c \
	$(wildcard $(LOCAL_PATH)/aria/*.c) \
	$(wildcard $(LOCAL_PATH)/asn1/*.c) \
	$(wildcard $(LOCAL_PATH)/async/arch/*.c) \
	$(wildcard $(LOCAL_PATH)/async/*.c) \
	$(wildcard $(LOCAL_PATH)/bf/*.c) \
	$(LOCAL_PATH)/bio/b_addr.c \
	$(LOCAL_PATH)/bio/b_dump.c \
	$(LOCAL_PATH)/bio/b_print.c \
	$(LOCAL_PATH)/bio/b_sock.c \
	$(LOCAL_PATH)/bio/b_sock2.c \
	$(LOCAL_PATH)/bio/bf_buff.c \
	$(LOCAL_PATH)/bio/bf_nbio.c \
	$(LOCAL_PATH)/bio/bf_null.c \
	$(LOCAL_PATH)/bio/bio_cb.c \
	$(LOCAL_PATH)/bio/bio_err.c \
	$(LOCAL_PATH)/bio/bio_lib.c \
	$(LOCAL_PATH)/bio/bio_meth.c \
	$(LOCAL_PATH)/bio/bss_acpt.c \
	$(LOCAL_PATH)/bio/bss_bio.c \
	$(LOCAL_PATH)/bio/bss_conn.c \
	$(LOCAL_PATH)/bio/bss_dgram.c \
	$(LOCAL_PATH)/bio/bss_fd.c \
	$(LOCAL_PATH)/bio/bss_file.c \
	$(LOCAL_PATH)/bio/bss_log.c \
	$(LOCAL_PATH)/bio/bss_mem.c \
	$(LOCAL_PATH)/bio/bss_null.c \
	$(LOCAL_PATH)/bio/bss_sock.c \
	$(wildcard $(LOCAL_PATH)/blake2/*.c) \
	$(wildcard $(LOCAL_PATH)/bn/*.c) \
	$(wildcard $(LOCAL_PATH)/buffer/*.c) \
	$(wildcard $(LOCAL_PATH)/camellia/*.c) \
	$(wildcard $(LOCAL_PATH)/cast/*.c) \
	$(wildcard $(LOCAL_PATH)/chacha/*.c) \
	$(wildcard $(LOCAL_PATH)/cmac/*.c) \
	$(wildcard $(LOCAL_PATH)/cms/*.c) \
	$(wildcard $(LOCAL_PATH)/comp/*.c) \
	$(wildcard $(LOCAL_PATH)/conf/*.c) \
	$(wildcard $(LOCAL_PATH)/ct/*.c) \
	$(LOCAL_PATH)/des/cbc_cksm.c \
	$(LOCAL_PATH)/des/cbc_enc.c \
	$(LOCAL_PATH)/des/cfb64ede.c \
	$(LOCAL_PATH)/des/cfb64enc.c \
	$(LOCAL_PATH)/des/cfb_enc.c \
	$(LOCAL_PATH)/des/des_enc.c \
	$(LOCAL_PATH)/des/ecb3_enc.c \
	$(LOCAL_PATH)/des/ecb_enc.c \
	$(LOCAL_PATH)/des/fcrypt.c \
	$(LOCAL_PATH)/des/fcrypt_b.c \
	$(LOCAL_PATH)/des/ofb64ede.c \
	$(LOCAL_PATH)/des/ofb64enc.c \
	$(LOCAL_PATH)/des/ofb_enc.c \
	$(LOCAL_PATH)/des/pcbc_enc.c \
	$(LOCAL_PATH)/des/qud_cksm.c \
	$(LOCAL_PATH)/des/rand_key.c \
	$(LOCAL_PATH)/des/set_key.c \
	$(LOCAL_PATH)/des/str2key.c \
	$(LOCAL_PATH)/des/xcbc_enc.c \
	$(wildcard $(LOCAL_PATH)/dh/*.c) \
	$(wildcard $(LOCAL_PATH)/dsa/*.c) \
	$(wildcard $(LOCAL_PATH)/dso/*.c) \
	$(wildcard $(LOCAL_PATH)/ec/curve448/arch_32/*.c) \
	$(wildcard $(LOCAL_PATH)/ec/curve448/*.c) \
	$(LOCAL_PATH)/ec/curve25519.c \
	$(LOCAL_PATH)/ec/ec2_oct.c \
	$(LOCAL_PATH)/ec/ec2_smpl.c \
	$(LOCAL_PATH)/ec/ec_ameth.c \
	$(LOCAL_PATH)/ec/ec_asn1.c \
	$(LOCAL_PATH)/ec/ec_check.c \
	$(LOCAL_PATH)/ec/ec_curve.c \
	$(LOCAL_PATH)/ec/ec_cvt.c \
	$(LOCAL_PATH)/ec/ec_err.c \
	$(LOCAL_PATH)/ec/ec_key.c \
	$(LOCAL_PATH)/ec/ec_kmeth.c \
	$(LOCAL_PATH)/ec/ec_lib.c \
	$(LOCAL_PATH)/ec/ec_mult.c \
	$(LOCAL_PATH)/ec/ec_oct.c \
	$(LOCAL_PATH)/ec/ec_pmeth.c \
	$(LOCAL_PATH)/ec/ec_print.c \
	$(LOCAL_PATH)/ec/ecdh_kdf.c \
	$(LOCAL_PATH)/ec/ecdh_ossl.c \
	$(LOCAL_PATH)/ec/ecdsa_ossl.c \
	$(LOCAL_PATH)/ec/ecdsa_sign.c \
	$(LOCAL_PATH)/ec/ecdsa_vrf.c \
	$(LOCAL_PATH)/ec/eck_prn.c \
	$(LOCAL_PATH)/ec/ecp_mont.c \
	$(LOCAL_PATH)/ec/ecp_nist.c \
	$(LOCAL_PATH)/ec/ecp_nistp224.c \
	$(LOCAL_PATH)/ec/ecp_nistp256.c \
	$(LOCAL_PATH)/ec/ecp_nistp521.c \
	$(LOCAL_PATH)/ec/ecp_nistputil.c \
	$(LOCAL_PATH)/ec/ecp_oct.c \
	$(LOCAL_PATH)/ec/ecp_smpl.c \
	$(LOCAL_PATH)/ec/ecx_meth.c \
	$(LOCAL_PATH)/engine/eng_all.c \
	$(LOCAL_PATH)/engine/eng_cnf.c \
	$(LOCAL_PATH)/engine/eng_ctrl.c \
	$(LOCAL_PATH)/engine/eng_dyn.c \
	$(LOCAL_PATH)/engine/eng_err.c \
	$(LOCAL_PATH)/engine/eng_fat.c \
	$(LOCAL_PATH)/engine/eng_init.c \
	$(LOCAL_PATH)/engine/eng_lib.c \
	$(LOCAL_PATH)/engine/eng_list.c \
	$(LOCAL_PATH)/engine/eng_openssl.c \
	$(LOCAL_PATH)/engine/eng_pkey.c \
	$(LOCAL_PATH)/engine/eng_rdrand.c \
	$(LOCAL_PATH)/engine/eng_table.c \
	$(LOCAL_PATH)/engine/tb_asnmth.c \
	$(LOCAL_PATH)/engine/tb_cipher.c \
	$(LOCAL_PATH)/engine/tb_dh.c \
	$(LOCAL_PATH)/engine/tb_digest.c \
	$(LOCAL_PATH)/engine/tb_dsa.c \
	$(LOCAL_PATH)/engine/tb_eckey.c \
	$(LOCAL_PATH)/engine/tb_pkmeth.c \
	$(LOCAL_PATH)/engine/tb_rand.c \
	$(LOCAL_PATH)/engine/tb_rsa.c \
	$(wildcard $(LOCAL_PATH)/err/*.c) \
	$(wildcard $(LOCAL_PATH)/evp/*.c) \
	$(wildcard $(LOCAL_PATH)/hmac/*.c) \
	$(wildcard $(LOCAL_PATH)/idea/*.c) \
	$(wildcard $(LOCAL_PATH)/kdf/*.c) \
	$(wildcard $(LOCAL_PATH)/lhash/*.c) \
	$(wildcard $(LOCAL_PATH)/md4/*.c) \
	$(wildcard $(LOCAL_PATH)/md5/*.c) \
	$(wildcard $(LOCAL_PATH)/mdc2/*.c) \
	$(wildcard $(LOCAL_PATH)/modes/*.c) \
	$(wildcard $(LOCAL_PATH)/objects/*.c) \
	$(wildcard $(LOCAL_PATH)/ocsp/*.c) \
	$(wildcard $(LOCAL_PATH)/pem/*.c) \
	$(wildcard $(LOCAL_PATH)/pkcs12/*.c) \
	$(wildcard $(LOCAL_PATH)/pkcs7/*.c) \
	$(LOCAL_PATH)/poly1305/poly1305.c \
	$(LOCAL_PATH)/poly1305/poly1305_ameth.c \
	$(LOCAL_PATH)/poly1305/poly1305_pmeth.c \
	$(wildcard $(LOCAL_PATH)/rand/*.c) \
	$(wildcard $(LOCAL_PATH)/rc2/*.c) \
	$(wildcard $(LOCAL_PATH)/rc4/*.c) \
	$(wildcard $(LOCAL_PATH)/ripemd/*.c) \
	$(wildcard $(LOCAL_PATH)/rsa/*.c) \
	$(wildcard $(LOCAL_PATH)/seed/*.c) \
	$(wildcard $(LOCAL_PATH)/sha/*.c) \
	$(wildcard $(LOCAL_PATH)/siphash/*.c) \
	$(wildcard $(LOCAL_PATH)/sm2/*.c) \
	$(wildcard $(LOCAL_PATH)/sm3/*.c) \
	$(wildcard $(LOCAL_PATH)/sm4/*.c) \
	$(wildcard $(LOCAL_PATH)/srp/*.c) \
	$(wildcard $(LOCAL_PATH)/stack/*.c) \
	$(wildcard $(LOCAL_PATH)/store/*.c) \
	$(wildcard $(LOCAL_PATH)/ts/*.c) \
	$(wildcard $(LOCAL_PATH)/txt_db/*.c) \
	$(wildcard $(LOCAL_PATH)/ui/*.c) \
	$(wildcard $(LOCAL_PATH)/whrlpool/*.c) \
	$(wildcard $(LOCAL_PATH)/x509/*.c) \
	$(wildcard $(LOCAL_PATH)/x509v3/*.c) \
	$(LOCAL_PATH)/cpt_err.c \
	$(LOCAL_PATH)/cryptlib.c \
	$(LOCAL_PATH)/ctype.c \
	$(LOCAL_PATH)/cversion.c \
	$(LOCAL_PATH)/ebcdic.c \
	$(LOCAL_PATH)/ex_data.c \
	$(LOCAL_PATH)/init.c \
	$(LOCAL_PATH)/mem.c \
	$(LOCAL_PATH)/mem_clr.c \
	$(LOCAL_PATH)/mem_dbg.c \
	$(LOCAL_PATH)/mem_sec.c \
	$(LOCAL_PATH)/o_dir.c \
	$(LOCAL_PATH)/o_fips.c \
	$(LOCAL_PATH)/o_fopen.c \
	$(LOCAL_PATH)/o_init.c \
	$(LOCAL_PATH)/o_str.c \
	$(LOCAL_PATH)/o_time.c \
	$(LOCAL_PATH)/uid.c \
	$(LOCAL_PATH)/getenv.c \
	$(LOCAL_PATH)/threads_pthread.c)

include $(BUILD_STATIC_LIBRARY)

########################
#
# ssl static library
#
########################

include $(CLEAR_VARS)

LOCAL_MODULE := ssl

LOCAL_C_INCLUDES := $(OSSL_PATH)
LOCAL_C_INCLUDES += $(OSSL_PATH)/include
LOCAL_C_INCLUDES += $(OSSL_MAKE)/android/$(TARGET_ARCH_ABI)/include

# The defines
LOCAL_CFLAGS += -DENGINESDIR=\"/usr/local/engines-1.1\" 
LOCAL_CFLAGS += -DOPENSSLDIR=\"/usr/local/ssl\" 
LOCAL_CFLAGS += -DOPENSSL_NO_ASM 
LOCAL_CFLAGS += -DOPENSSL_NO_STATIC_ENGINE 

# Compilation flags
LOCAL_CFLAGS += -fPIC 
LOCAL_CFLAGS += -fno-limit-debug-info 

LOCAL_PATH = $(OSSL_PATH)/ssl
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/record/*.c) \
	$(wildcard $(LOCAL_PATH)/statem/*.c) \
	$(wildcard $(LOCAL_PATH)/*.c) )

include $(BUILD_STATIC_LIBRARY)