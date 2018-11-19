cmake_minimum_required (VERSION 2.6)

########################################
# Compile OpenThread as an libraries set
########################################

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-diag.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-nrf52840.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-ftd.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-platform-utils.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/third_party/NordicSemiconductor/libraries/nrf_cc310/lib/libnrf_cc310_0.9.10.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread
    COMMAND ./bootstrap
    COMMAND make -f examples/Makefile-nrf52840 DISABLE_CC310=1 JOINER=1
)

add_custom_target(
    openthread
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-diag.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-nrf52840.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-ftd.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-platform-utils.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/third_party/NordicSemiconductor/libraries/nrf_cc310/lib/libnrf_cc310_0.9.10.a
)

add_library(libmbedcrypto STATIC IMPORTED)
set_property(TARGET libmbedcrypto PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a)

add_library(libopenthread-diag STATIC IMPORTED)
set_property(TARGET libopenthread-diag PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-diag.a)

add_library(libopenthread-nrf52840 STATIC IMPORTED)
set_property(TARGET libopenthread-nrf52840 PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-nrf52840.a)

add_library(libopenthread-ftd STATIC IMPORTED)
set_property(TARGET libopenthread-ftd PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-ftd.a)

add_library(libopenthread-platform-utils STATIC IMPORTED)
set_property(TARGET libopenthread-platform-utils PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-platform-utils.a)

add_library(libnrf_cc310 STATIC IMPORTED)
set_property(TARGET libnrf_cc310 PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/third_party/NordicSemiconductor/libraries/nrf_cc310/lib/libnrf_cc310_0.9.10.a)

add_dependencies(libopenthread-ftd openthread)

set(
    OT_LIBS
    libopenthread-ftd
    libopenthread-platform-utils
    libopenthread-nrf52840
    libmbedcrypto
    libopenthread-diag
    libnrf_cc310
)

set(
    OT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/examples/platforms
)

###########################
# Compile nrfx as a library
###########################

add_library(
    nrfx
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/src/nrfx_gpiote.c
)

set(
    NRFX_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/mdk
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/templates
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/templates/nRF52840
)

target_compile_definitions(nrfx
    PUBLIC "NRF52840_XXAA"
)

###############
# Prepare CMSIS
###############

set(CMSIS_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/CMSIS_5/CMSIS/Core/Include)

#########################
# Common for all projects
#########################

set(LINKER_SCRIPT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/examples/platforms/nrf52840/nrf52840.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT}")

macro(CreateHex target)
    ADD_CUSTOM_COMMAND(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:${target}> $<TARGET_FILE:${target}>.hex
    )
endmacro()

macro(PrintSize target)
    ADD_CUSTOM_COMMAND(
        TARGET ${target}
        POST_BUILD
        COMMAND ${CMAKE_SIZE_UTIL} $<TARGET_FILE:${target}>
    )
endmacro()

macro(FinalizeHex target)
    CreateHex(${target})
    PrintSize(${target})
endmacro()

#####################################
# Prepare the OpenThread GPIO project
#####################################

project (ot_gpio)

include_directories (${OT_INCLUDE_DIRS})
include_directories (${NRFX_INCLUDE_DIRS})
include_directories (${CMSIS_INCLUDE_DIRS})

##################################
# Prepare the Shades Remote binary
##################################

add_executable(
    sh_rmt
    src/apps/sh_rmt/main.c
    src/apps/sh_rmt/sh_rmt_btn.c
    src/apps/sh_rmt/sh_rmt_led.c
)

target_link_libraries (sh_rmt ${OT_LIBS} nrfx)

FinalizeHex(sh_rmt)

######################################
# Prepare the Shades Controller binary
######################################

add_executable(
    sh_cnt
    src/apps/sh_cnt/main.c
    src/apps/sh_cnt/sh_cnt_rly.c
)

target_link_libraries (sh_cnt ${OT_LIBS} nrfx)

FinalizeHex(sh_cnt)

############
# Some tests
############

IF (BUILD_TESTING)

    include(CTest)

    add_test(NAME dummy_test
        COMMAND true
    )

ENDIF(BUILD_TESTING)
