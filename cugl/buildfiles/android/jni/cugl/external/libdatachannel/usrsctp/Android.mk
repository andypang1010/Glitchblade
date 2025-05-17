LOCAL_PATH  := $(call my-dir)
CURR_DEPTH  := ../../../../..
CUGL_OFFSET := ../..

########################
#
# usrsctp static library
#
########################
USCTP_PATH := $(LOCAL_PATH)/$(CURR_DEPTH)/$(CUGL_OFFSET)/external/libdatachannel/deps/usrsctp
USCTP_MAKE := $(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := usrsctp

LOCAL_C_INCLUDES := $(USCTP_PATH)/usrsctplib
LOCAL_C_INCLUDES += $(USCTP_PATH)/usrsctplib/netinet
LOCAL_C_INCLUDES += $(USCTP_PATH)/usrsctplib/netinet6

# The defines
LOCAL_CFLAGS += -DHAVE_LINUX_IF_ADDR_H 
LOCAL_CFLAGS += -DHAVE_LINUX_RTNETLINK_H 
LOCAL_CFLAGS += -DHAVE_NETINET_IP_ICMP_H 
LOCAL_CFLAGS += -DHAVE_NET_ROUTE_H 
LOCAL_CFLAGS += -DHAVE_STDATOMIC_H 
LOCAL_CFLAGS += -DHAVE_SYS_QUEUE_H 
LOCAL_CFLAGS += -DINET 
LOCAL_CFLAGS += -DINET6 
LOCAL_CFLAGS += -DSCTP_DEBUG 
LOCAL_CFLAGS += -DSCTP_PROCESS_LEVEL_LOCKS 
LOCAL_CFLAGS += -DSCTP_SIMPLE_ALLOCATOR 
LOCAL_CFLAGS += -DANDROID 
LOCAL_CFLAGS += -D__Userspace__ 
LOCAL_CFLAGS += -D_FORTIFY_SOURCE=2

# Compilation flags
LOCAL_CFLAGS += -std=c99 
LOCAL_CFLAGS += -fdata-sections 
LOCAL_CFLAGS += -ffunction-sections 
LOCAL_CFLAGS += -funwind-tables 
LOCAL_CFLAGS += -fstack-protector-strong 
LOCAL_CFLAGS += -no-canonical-prefixes 
LOCAL_CFLAGS += -Wformat 
LOCAL_CFLAGS += -Werror=format-security  
LOCAL_CFLAGS += -pedantic 
LOCAL_CFLAGS += -Wall 
LOCAL_CFLAGS += -Wextra 
LOCAL_CFLAGS += -Wfloat-equal 
LOCAL_CFLAGS += -Wshadow 
LOCAL_CFLAGS += -Wpointer-arith 
LOCAL_CFLAGS += -Wunreachable-code 
LOCAL_CFLAGS += -Winit-self 
LOCAL_CFLAGS += -Wno-unused-function 
LOCAL_CFLAGS += -Wno-unused-parameter 
LOCAL_CFLAGS += -Wno-unreachable-code 
LOCAL_CFLAGS += -Wstrict-prototypes 
LOCAL_CFLAGS += -Werror 
LOCAL_CFLAGS += -Wno-address-of-packed-member 
LOCAL_CFLAGS += -Wno-deprecated-declarations 
LOCAL_CFLAGS += -fno-limit-debug-info  
LOCAL_CFLAGS += -fPIC 


# Add your application source files here.
LOCAL_PATH = $(USCTP_PATH)/usrsctplib
LOCAL_SRC_FILES := $(subst $(LOCAL_PATH)/,, \
	$(LOCAL_PATH)/netinet/sctp_asconf.c \
	$(LOCAL_PATH)/netinet/sctp_auth.c \
	$(LOCAL_PATH)/netinet/sctp_bsd_addr.c \
	$(LOCAL_PATH)/netinet/sctp_callout.c \
	$(LOCAL_PATH)/netinet/sctp_cc_functions.c \
	$(LOCAL_PATH)/netinet/sctp_crc32.c \
	$(LOCAL_PATH)/netinet/sctp_indata.c \
	$(LOCAL_PATH)/netinet/sctp_input.c \
	$(LOCAL_PATH)/netinet/sctp_output.c \
	$(LOCAL_PATH)/netinet/sctp_pcb.c \
	$(LOCAL_PATH)/netinet/sctp_peeloff.c \
	$(LOCAL_PATH)/netinet/sctp_sha1.c \
	$(LOCAL_PATH)/netinet/sctp_ss_functions.c \
	$(LOCAL_PATH)/netinet/sctp_sysctl.c \
	$(LOCAL_PATH)/netinet/sctp_timer.c \
	$(LOCAL_PATH)/netinet/sctp_userspace.c \
	$(LOCAL_PATH)/netinet/sctp_usrreq.c \
	$(LOCAL_PATH)/netinet/sctputil.c \
	$(LOCAL_PATH)/netinet6/sctp6_usrreq.c \
	$(LOCAL_PATH)/user_environment.c \
	$(LOCAL_PATH)/user_mbuf.c \
	$(LOCAL_PATH)/user_recv_thread.c \
	$(LOCAL_PATH)/user_socket.c)

include $(BUILD_STATIC_LIBRARY)