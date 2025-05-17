option(WITH_ASM "Use any assembly optimization routines" ON)

check_include_file("cpuid.h" HAVE_CPUID_H)
check_include_file("sys/param.h" HAVE_SYS_PARAM_H)

set(CMAKE_REQUIRED_LIBRARIES m)
check_function_exists(lround HAVE_LROUND)

include(CheckCSourceCompiles)
include(CheckCPUArch)
include(CheckA64NEON)

check_cpu_arch_x64(FLAC__CPU_X86_64)
if(NOT FLAC__CPU_X86_64)
    check_cpu_arch_x86(FLAC__CPU_IA32)
endif()

if(FLAC__CPU_X86_64 OR FLAC__CPU_IA32)
    set(FLAC__ALIGN_MALLOC_DATA 1)
    option(WITH_AVX "Enable AVX, AVX2 optimizations (with runtime detection, resulting binary does not require AVX2, so only necessary when a compiler doesn't know about AVX)" ON)
    if(WITH_AVX AND MSVC)
        set_source_files_properties(fixed_intrin_avx2.c lpc_intrin_avx2.c stream_encoder_intrin_avx2.c lpc_intrin_fma.c PROPERTIES COMPILE_FLAGS /arch:AVX2)
    endif()
else()
    check_cpu_arch_arm64(FLAC__CPU_ARM64)
    if(FLAC__CPU_ARM64)
         check_a64neon(FLAC__HAS_A64NEONINTRIN)
    endif()
endif()

if(NOT WITH_ASM)
    add_definitions(-DFLAC__NO_ASM)
endif()

set(LIBFLAC_DIR "${FLAC_DIR}/src/libFLAC")

include_directories("${LIBFLAC_DIR}/include")

add_library(FLAC
    ${LIBFLAC_DIR}/bitmath.c
    ${LIBFLAC_DIR}/bitreader.c
    ${LIBFLAC_DIR}/bitwriter.c
    ${LIBFLAC_DIR}/cpu.c
    ${LIBFLAC_DIR}/crc.c
    ${LIBFLAC_DIR}/fixed.c
    ${LIBFLAC_DIR}/fixed_intrin_sse2.c
    ${LIBFLAC_DIR}/fixed_intrin_ssse3.c
    ${LIBFLAC_DIR}/fixed_intrin_sse42.c
    ${LIBFLAC_DIR}/fixed_intrin_avx2.c
    ${LIBFLAC_DIR}/float.c
    ${LIBFLAC_DIR}/format.c
    ${LIBFLAC_DIR}/lpc.c
    ${LIBFLAC_DIR}/lpc_intrin_neon.c
    ${LIBFLAC_DIR}/lpc_intrin_sse2.c
    ${LIBFLAC_DIR}/lpc_intrin_sse41.c
    ${LIBFLAC_DIR}/lpc_intrin_avx2.c
    ${LIBFLAC_DIR}/lpc_intrin_fma.c
    ${LIBFLAC_DIR}/md5.c
    ${LIBFLAC_DIR}/memory.c
    ${LIBFLAC_DIR}/metadata_iterators.c
    ${LIBFLAC_DIR}/metadata_object.c
    ${LIBFLAC_DIR}/stream_decoder.c
    ${LIBFLAC_DIR}/stream_encoder.c
    ${LIBFLAC_DIR}/stream_encoder_intrin_sse2.c
    ${LIBFLAC_DIR}/stream_encoder_intrin_ssse3.c
    ${LIBFLAC_DIR}/stream_encoder_intrin_avx2.c
    ${LIBFLAC_DIR}/stream_encoder_framing.c
    ${LIBFLAC_DIR}/version.rc
    ${LIBFLAC_DIR}/window.c
    $<$<BOOL:${WIN32}>:${FLAC_DIR}/include/share/win_utf8_io.h>
    $<$<BOOL:${WIN32}>:${FLAC_DIR}/src/share/win_utf8_io/win_utf8_io.c>
    $<$<BOOL:${OGG_FOUND}>:${LIBFLAC_DIR}/ogg_decoder_aspect.c>
    $<$<BOOL:${OGG_FOUND}>:${LIBFLAC_DIR}/ogg_encoder_aspect.c>
    $<$<BOOL:${OGG_FOUND}>:${LIBFLAC_DIR}/ogg_helper.c>
    $<$<BOOL:${OGG_FOUND}>:${LIBFLAC_DIR}/ogg_mapping.c>)
set_property(TARGET FLAC PROPERTY PROJECT_LABEL "libFLAC")
if(TARGET FLAC-asm)
    target_sources(FLAC PRIVATE $<TARGET_OBJECTS:FLAC-asm>)
endif()

target_compile_definitions(FLAC
    PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:FLAC_API_EXPORTS>
    PUBLIC $<$<NOT:$<BOOL:${BUILD_SHARED_LIBS}>>:FLAC__NO_DLL>)
if(NOT WIN32)
    target_compile_definitions(FLAC PRIVATE $<$<BOOL:${BUILD_SHARED_LIBS}>:FLAC__USE_VISIBILITY_ATTR>)
endif()
target_include_directories(FLAC INTERFACE
    "$<BUILD_INTERFACE:${FLAC_DIR}/include>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>")
target_link_libraries(FLAC PUBLIC $<$<BOOL:${HAVE_LROUND}>:m>)
if(TARGET Ogg::ogg)
    target_link_libraries(FLAC PUBLIC Ogg::ogg)
endif()
if(BUILD_SHARED_LIBS)
    set_target_properties(FLAC PROPERTIES
        VERSION 12.2.0
        SOVERSION 12)
    if(NOT WIN32)
        set_target_properties(FLAC PROPERTIES C_VISIBILITY_PRESET hidden)
    endif()
endif()

check_c_compiler_flag("-fassociative-math -fno-signed-zeros -fno-trapping-math -freciprocal-math" HAVE_ASSOC_MATH)

if(MSVC)
        target_compile_options(FLAC BEFORE PRIVATE "/fp:fast")
else()
    if(HAVE_ASSOC_MATH)
        target_compile_options(FLAC BEFORE PRIVATE -fassociative-math -fno-signed-zeros -fno-trapping-math -freciprocal-math)
    endif()
endif()

if(HAVE_PTHREAD)
    target_link_libraries(FLAC PUBLIC Threads::Threads)
endif()

add_library(FLAC::FLAC ALIAS FLAC)

install(TARGETS FLAC EXPORT targets
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}/"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}/")

if(INSTALL_PKGCONFIG_MODULES)
    set(prefix "${CMAKE_INSTALL_PREFIX}")
    set(exec_prefix "${CMAKE_INSTALL_PREFIX}")
    set(libdir "${CMAKE_INSTALL_FULL_LIBDIR}")
    set(includedir "${CMAKE_INSTALL_FULL_INCLUDEDIR}")
    configure_file(${LIBFLAC_DIR}/flac.pc.in flac.pc @ONLY)
    install(FILES "${CMAKE_CURRENT_BINARY_DIR}/flac.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")
endif()
