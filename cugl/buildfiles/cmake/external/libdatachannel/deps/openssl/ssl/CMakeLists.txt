# Based on original work by David Manura
# Copyright (C) 2007-2012 LuaDist.
# Copyright (C) 2013 Brian Sidebotham

# Redistribution and use of this file is allowed according to the terms of the
# MIT license.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

if( BUILD_SHARED_LIBS )
  add_definitions( -DOPENSSL_BUILD_SHLIBSSL )
endif()

# Define the relative position of this file in the CUGL project
# This is to make this file easily relocatable (for Android, etc.)
set(CMAKE_CURRENT_DEPTH "${PROJECT_SOURCE_DIR}/../../../..")

if(NOT DEFINED PROJECT_DIR_OFFSET)
	set(PROJECT_DIR_OFFSET "../..")
endif()

if(NOT DEFINED OPENSSL_PATH)
	set(OPENSSL_PATH "${CMAKE_CURRENT_DEPTH}/${PROJECT_DIR_OFFSET}/external/openssl")
endif()
set(SSL_PATH "${OPENSSL_PATH}/ssl" )

set( LIBSRC
  	${SSL_PATH}/bio_ssl.c 
  	${SSL_PATH}/d1_lib.c 
  	${SSL_PATH}/d1_msg.c 
  	${SSL_PATH}/d1_srtp.c 
  	${SSL_PATH}/methods.c 
  	${SSL_PATH}/packet.c 
  	${SSL_PATH}/pqueue.c 
  	${SSL_PATH}/s3_cbc.c 
  	${SSL_PATH}/s3_enc.c 
  	${SSL_PATH}/s3_lib.c
  	${SSL_PATH}/s3_msg.c 
  	${SSL_PATH}/ssl_asn1.c 
  	${SSL_PATH}/ssl_cert.c 
  	${SSL_PATH}/ssl_ciph.c 
  	${SSL_PATH}/ssl_conf.c 
  	${SSL_PATH}/ssl_err.c 
  	${SSL_PATH}/ssl_init.c 
  	${SSL_PATH}/ssl_lib.c
  	${SSL_PATH}/ssl_mcnf.c 
  	${SSL_PATH}/ssl_rsa.c 
  	${SSL_PATH}/ssl_sess.c 
  	${SSL_PATH}/ssl_stat.c 
  	${SSL_PATH}/ssl_txt.c 
  	${SSL_PATH}/ssl_utst.c 
  	${SSL_PATH}/t1_enc.c 
  	${SSL_PATH}/t1_lib.c
  	${SSL_PATH}/t1_trce.c 
  	${SSL_PATH}/tls13_enc.c 
  	${SSL_PATH}/tls_srp.c
  	${SSL_PATH}/record/dtls1_bitmap.c 
  	${SSL_PATH}/record/rec_layer_d1.c 
  	${SSL_PATH}/record/rec_layer_s3.c 
  	${SSL_PATH}/record/ssl3_buffer.c
  	${SSL_PATH}/record/ssl3_record.c 
  	${SSL_PATH}/record/ssl3_record_tls13.c
  	${SSL_PATH}/statem/extensions.c 
  	${SSL_PATH}/statem/extensions_clnt.c 
  	${SSL_PATH}/statem/extensions_cust.c
  	${SSL_PATH}/statem/extensions_srvr.c 
  	${SSL_PATH}/statem/statem.c 
  	${SSL_PATH}/statem/statem_clnt.c 
  	${SSL_PATH}/statem/statem_dtls.c
  	${SSL_PATH}/statem/statem_lib.c 
  	${SSL_PATH}/statem/statem_srvr.c
)

include_directories( BEFORE SYSTEM
  ${OPENSSL_PATH}/ # e_os.h
)

add_library( ssl ${LIBSRC} )
add_library(OpenSSL::SSL ALIAS ssl)

target_include_directories( ssl PUBLIC ${PROJECT_BINARY_DIR}/include )

target_link_libraries( ssl crypto )

set_target_properties( ssl PROPERTIES
  VERSION "${LIB_VERSION}" SOVERSION "${LIB_SOVERSION}" )

if( ANDROID )
  set_target_properties( ssl PROPERTIES OUTPUT_NAME "ssl_1_1" )
elseif( MSVC )
  if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    set_target_properties( ssl PROPERTIES OUTPUT_NAME "ssl-1_1-x64" )
  elseif( CMAKE_SIZEOF_VOID_P EQUAL 4 )
    set_target_properties( ssl PROPERTIES OUTPUT_NAME "ssl-1_1" )
  endif()
else()
  set_target_properties( ssl PROPERTIES OUTPUT_NAME "ssl" )
endif()

