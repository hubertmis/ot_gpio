cmake_minimum_required (VERSION 2.6)


project (sh_rmt)

set(LINKER_SCRIPT "${PROJECT_SOURCE_DIR}/third_party/openthread/examples/platforms/nrf52840/nrf52840.ld")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T ${LINKER_SCRIPT}")

include_directories ("${PROJECT_SOURCE_DIR}/third_party/openthread/output/include")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/openthread/examples/platforms")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/nrfx/drivers/include")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/nrfx")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/nrfx/templates")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/nrfx/templates/nRF52840")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/nrfx/mdk")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/nrfx/hal")
include_directories ("${PROJECT_SOURCE_DIR}/third_party/CMSIS_5/CMSIS/Core/Include")

add_compile_definitions(NRF52840_XXAA)

set(NRFX_FILES ${PROJECT_SOURCE_DIR}/third_party/nrfx/drivers/src/nrfx_gpiote.c)
add_executable(sh_rmt src/apps/sh_rmt/main.c src/apps/sh_rmt/sh_rmt_btn.c src/apps/sh_rmt/sh_rmt_led.c ${NRFX_FILES})

add_library(libmbedcrypto STATIC IMPORTED)
set_property(TARGET libmbedcrypto PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libmbedcrypto.a)
add_library(libopenthread-diag STATIC IMPORTED)
set_property(TARGET libopenthread-diag PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-diag.a)
add_library(libopenthread-nrf52840 STATIC IMPORTED)
set_property(TARGET libopenthread-nrf52840 PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-nrf52840.a)
add_library(libopenthread-ftd STATIC IMPORTED)
set_property(TARGET libopenthread-ftd PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-ftd.a)
add_library(libopenthread-platform-utils STATIC IMPORTED)
set_property(TARGET libopenthread-platform-utils PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/openthread/output/nrf52840/lib/libopenthread-platform-utils.a)
add_library(libnrf_cc310 STATIC IMPORTED)
set_property(TARGET libnrf_cc310 PROPERTY IMPORTED_LOCATION ${PROJECT_SOURCE_DIR}/third_party/openthread/third_party/NordicSemiconductor/libraries/nrf_cc310/lib/libnrf_cc310_0.9.10.a)

target_link_libraries (sh_rmt libopenthread-ftd libopenthread-platform-utils libopenthread-nrf52840 libmbedcrypto libopenthread-diag libnrf_cc310)
