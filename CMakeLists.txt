cmake_minimum_required(VERSION 3.12)
set(PICO_BOARD pico_w)
include(pico_sdk_import.cmake)
include(pico_extras_import.cmake)
pico_sdk_init()

project(homeassistant-eink-display)

# add a compilation subdirectory
add_subdirectory(externals/Pico_ePaper_Code/c/lib/Config)
add_subdirectory(externals/Pico_ePaper_Code/c/lib/e-Paper)
add_subdirectory(externals/Pico_ePaper_Code/c/lib/Fonts)
add_subdirectory(externals/Pico_ePaper_Code/c/lib/GUI)


# add a header directory
include_directories(./externals/Pico_ePaper_Code/c/lib/Config)
include_directories(./externals/Pico_ePaper_Code/c/lib/GUI)
include_directories(./externals/Pico_ePaper_Code/c/lib/e-Paper)
include_directories(./externals/tiny-json)

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.h"
        "${PROJECT_SOURCE_DIR}/src/*.c"
        )

# generate an executable file
add_executable(hass-display
${all_SRCS}
externals/tiny-json/tiny-json.c
)

# enable usb output, disable uart output
pico_enable_stdio_usb(hass-display 0)
pico_enable_stdio_uart(hass-display 1)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(hass-display)

target_include_directories(hass-display PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_link_libraries(hass-display pico_lwip_core pico_cyw43_arch_lwip_poll ePaper GUI Fonts Config pico_stdlib hardware_spi hardware_sleep hardware_rosc hardware_rtc )
#target_link_libraries(hass-display pico_lwip_core pico_lwip_mbedtls pico_mbedtls pico_cyw43_arch_lwip_poll ePaper GUI Fonts Config pico_stdlib hardware_spi)

