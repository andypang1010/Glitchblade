add_library(grabbag STATIC
    ${FLAC_DIR}/src/share/grabbag/alloc.c
    ${FLAC_DIR}/src/share/grabbag/cuesheet.c
    ${FLAC_DIR}/src/share/grabbag/file.c
    ${FLAC_DIR}/src/share/grabbag/picture.c
    ${FLAC_DIR}/src/share/grabbag/replaygain.c
    ${FLAC_DIR}/src/share/grabbag/seektable.c
    ${FLAC_DIR}/src/share/grabbag/snprintf.c)
target_link_libraries(grabbag PUBLIC
    FLAC
    replaygain_analysis)
if(TARGET win_utf8_io)
    target_link_libraries(grabbag PUBLIC win_utf8_io)
endif()
