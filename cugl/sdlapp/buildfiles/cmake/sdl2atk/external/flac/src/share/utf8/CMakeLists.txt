cmake_minimum_required(VERSION 3.12)

add_library(utf8 STATIC
    ${FLAC_DIR}/src/share/utf8/charset.c
    ${FLAC_DIR}/src/share/utf8/iconvert.c
    ${FLAC_DIR}/src/share/utf8/utf8.c)

target_link_libraries(utf8 PUBLIC grabbag $<TARGET_NAME_IF_EXISTS:Iconv::Iconv>)
