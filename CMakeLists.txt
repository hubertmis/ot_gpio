cmake_minimum_required (VERSION 3.2)

cmake_policy(SET CMP0012 NEW)

########################################
# Compile OpenThread as an libraries set
########################################

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libnordicsemi-nrf52840-radio-driver.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libnordicsemi-nrf52840-sdk.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-nrf52840.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-ftd.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-platform-utils.a
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/third_party/NordicSemiconductor/libraries/nrf_cc310/lib/libnrf_cc310_0.9.10.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread
    COMMAND ./bootstrap
    COMMAND make -f examples/Makefile-nrf52840 JOINER=1 COAP=1 COAPS=1
)

add_custom_target(
    openthread
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libnordicsemi-nrf52840-radio-driver.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libnordicsemi-nrf52840-sdk.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-nrf52840.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-ftd.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-platform-utils.a
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/third_party/NordicSemiconductor/libraries/nrf_cc310/lib/libnrf_cc310_0.9.10.a
)

add_library(libmbedcrypto STATIC IMPORTED)
set_property(TARGET libmbedcrypto PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a)

add_library(libnordicsemi-nrf52840-radio-driver STATIC IMPORTED)
set_property(TARGET libnordicsemi-nrf52840-radio-driver PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libnordicsemi-nrf52840-radio-driver.a)

add_library(libnordicsemi-nrf52840-sdk STATIC IMPORTED)
set_property(TARGET libnordicsemi-nrf52840-sdk PROPERTY IMPORTED_LOCATION ${CMAKE_CURRENT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libnordicsemi-nrf52840-sdk.a)

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
    libopenthread-platform-utils
    libopenthread-ftd
    libmbedcrypto
    libopenthread-nrf52840
    libopenthread-ftd
    libnordicsemi-nrf52840-radio-driver
    libnordicsemi-nrf52840-sdk
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
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/src/nrfx_saadc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/src/nrfx_gpiote.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/src/nrfx_rtc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/src/nrfx_spim.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/src/prs/nrfx_prs.c
)

set(
    NRFX_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/config
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/drivers/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/hal
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nrfx/mdk
)

target_compile_definitions(nrfx
    PUBLIC "NRF52840_XXAA"
    PUBLIC "CONFIG_NFCT_PINS_AS_GPIOS"
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

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Os")

execute_process(
  COMMAND git log -1 --format=%h
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
  OUTPUT_VARIABLE GIT_COMMIT_HASH
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

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
    src/apps/sh_rmt/sh_rmt_anim.c
    src/apps/sh_rmt/sh_rmt_anim.h
    src/apps/sh_rmt/sh_rmt_btn.c
    src/apps/sh_rmt/sh_rmt_btn.h
    src/apps/sh_rmt/sh_rmt_conn.c
    src/apps/sh_rmt/sh_rmt_conn.h
    src/apps/sh_rmt/sh_rmt_led.c
    src/apps/sh_rmt/sh_rmt_led.h
    src/apps/sh_rmt/sh_rmt_prg.c
    src/apps/sh_rmt/sh_rmt_prg.h
    src/lib/btn/humi_btn.c
    src/lib/btn/humi_btn.h
    src/lib/conn/humi_conn.c
    src/lib/conn/humi_conn.h
    src/lib/led/humi_led.c
    src/lib/led/humi_led.h
    src/lib/lite_log/lite_log.c
    src/lib/lite_log/lite_log.h
    src/lib/lite_log/log_data.h
    src/lib/mcbor/mcbor_const.h
    src/lib/mcbor/mcbor_dec.c
    src/lib/mcbor/mcbor_dec.h
    src/lib/mcbor/mcbor_enc.c
    src/lib/mcbor/mcbor_enc.h
    src/lib/mcbor/mcbor_types.h
    src/lib/pwr/humi_pwr.c
    src/lib/pwr/humi_pwr.h
    src/lib/timer/humi_timer.c
    src/lib/timer/humi_timer.h
)

target_compile_definitions(sh_rmt
        PUBLIC "VENDOR_MODEL=\"sh_rmt\""
        PUBLIC "SW_VERSION=\"${GIT_COMMIT_HASH}\""
        )

if (${PSKD})
    target_compile_definitions(sh_rmt PUBLIC "PSKD=\"${PSKD}\"")
endif(${PSKD})

#if (DEFINED CACHE{COAP_PSK})
#    target_compile_definitions(sh_rmt PUBLIC "COAP_PSK=\"${COAP_PSK}\"")
#endif (DEFINED CACHE{COAP_PSK})

target_link_libraries (sh_rmt ${OT_LIBS} nrfx)

FinalizeHex(sh_rmt)

######################################
# Prepare the Shades Controller binary
######################################

set(SH_CNT_SRCS
        src/apps/sh_cnt/main.c
        src/apps/sh_cnt/sh_cnt_btn.c
        src/apps/sh_cnt/sh_cnt_btn.h
        src/apps/sh_cnt/sh_cnt_conn.c
        src/apps/sh_cnt/sh_cnt_conn.h
        src/apps/sh_cnt/sh_cnt_display.c
        src/apps/sh_cnt/sh_cnt_display.h
        src/apps/sh_cnt/sh_cnt_led.c
        src/apps/sh_cnt/sh_cnt_led.h
        src/apps/sh_cnt/sh_cnt_mot.c
        src/apps/sh_cnt/sh_cnt_mot.h
        src/apps/sh_cnt/sh_cnt_pos.c
        src/apps/sh_cnt/sh_cnt_pos.h
        src/apps/sh_cnt/sh_cnt_rly.c
        src/apps/sh_cnt/sh_cnt_rly.h
        src/lib/btn/humi_btn.c
        src/lib/btn/humi_btn.h
        src/lib/conn/humi_conn.c
        src/lib/conn/humi_conn.h
        src/lib/led/humi_led.c
        src/lib/led/humi_led.h
        src/lib/lite_log/lite_log.c
        src/lib/lite_log/lite_log.h
        src/lib/lite_log/log_data.h
        src/lib/mcbor/mcbor_const.h
        src/lib/mcbor/mcbor_dec.c
        src/lib/mcbor/mcbor_dec.h
        src/lib/mcbor/mcbor_enc.c
        src/lib/mcbor/mcbor_enc.h
        src/lib/mcbor/mcbor_types.h
        src/lib/pwr/humi_pwr.c
        src/lib/pwr/humi_pwr.h
        src/lib/rly/humi_rly.h
        src/lib/rly/humi_rly.c
        src/lib/timer/humi_timer.c
        src/lib/timer/humi_timer.h
    )

foreach(SH_CNT_VAR dr k lr br test)
    set(BIN_NAME sh_cnt_${SH_CNT_VAR})
    add_executable(
            ${BIN_NAME}
            ${SH_CNT_SRCS}
    )

    target_compile_definitions(${BIN_NAME}
            PUBLIC "SH_CNT_LOC_${SH_CNT_VAR}=1"
            PUBLIC "VENDOR_MODEL=\"sh_cnt\""
            PUBLIC "SW_VERSION=\"${GIT_COMMIT_HASH}\""
            )

    if (${PSKD})
        target_compile_definitions(${BIN_NAME} PUBLIC "PSKD=\"${PSKD}\"")
    endif(${PSKD})

    if (DEFINED CACHE{COAP_PSK})
        target_compile_definitions(${BIN_NAME} PUBLIC "COAP_PSK=\"${COAP_PSK}\"")
    endif(DEFINED CACHE{COAP_PSK})

    target_link_libraries (${BIN_NAME} ${OT_LIBS} nrfx)

    FinalizeHex(${BIN_NAME})
endforeach(SH_CNT_VAR)

#####################################
# Prepare the Temp Touchscreen binary
#####################################

add_executable(
        temp_tscrn
        src/apps/temp_tscrn/ctrl.c
        src/apps/temp_tscrn/ctrl.h
        src/apps/temp_tscrn/display.c
        src/apps/temp_tscrn/display.h
        src/apps/temp_tscrn/main.c
        src/apps/temp_tscrn/output.c
        src/apps/temp_tscrn/output.h
        src/apps/temp_tscrn/temp_scrn.h
        src/apps/temp_tscrn/temp_scrn.c
        src/apps/temp_tscrn/temp_tscrn_data.c
        src/apps/temp_tscrn/temp_tscrn_data.h
        src/lib/conn/humi_conn.c
        src/lib/conn/humi_conn.h
        src/lib/ft8xx/ft8xx.c
        src/lib/ft8xx/ft8xx.h
        src/lib/ft8xx/ft8xx_copro.h
        src/lib/ft8xx/ft8xx_copro.c
        src/lib/ft8xx/ft8xx_dl.h
        src/lib/ft8xx/ft8xx_drv.c
        src/lib/ft8xx/ft8xx_drv.h
        src/lib/ft8xx/ft8xx_memory.h
        src/lib/lite_log/lite_log.c
        src/lib/lite_log/lite_log.h
        src/lib/lite_log/log_data.h
        src/lib/mcbor/mcbor_const.h
        src/lib/mcbor/mcbor_dec.c
        src/lib/mcbor/mcbor_dec.h
        src/lib/mcbor/mcbor_enc.c
        src/lib/mcbor/mcbor_enc.h
        src/lib/mcbor/mcbor_types.h
        src/lib/ntc/humi_ntc.c
        src/lib/ntc/humi_ntc.h
        src/lib/pwr/humi_pwr.c
        src/lib/pwr/humi_pwr.h
        src/lib/rly/humi_rly.c
        src/lib/rly/humi_rly.h
        src/lib/timer/humi_timer.c
        src/lib/timer/humi_timer.h
)

target_compile_definitions(temp_tscrn
        PUBLIC "VENDOR_MODEL=\"temp_tscrn\""
        PUBLIC "SW_VERSION=\"${GIT_COMMIT_HASH}\""
        )

if (${PSKD})
    target_compile_definitions(temp_tscrn PUBLIC "PSKD=\"${PSKD}\"")
endif(${PSKD})

if (DEFINED CACHE{COAP_PSK})
    target_compile_definitions(temp_tscrn PUBLIC "COAP_PSK=\"${COAP_PSK}\"")
endif (DEFINED CACHE{COAP_PSK})

target_link_libraries (temp_tscrn ${OT_LIBS} nrfx m)

FinalizeHex(temp_tscrn)

############
# Some tests
############

IF (BUILD_TESTING)

    include(CTest)

    add_test(NAME dummy_test
        COMMAND true
    )

ENDIF(BUILD_TESTING)
