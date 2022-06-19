cmake_minimum_required(VERSION 3.5)

include(${PICO_SDK_PATH}/lib/tinyusb/hw/bsp/family_support.cmake)

# gets PROJECT name for the example (e.g. <BOARD>-<DIR_NAME>)
family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})

project(${PROJECT})

# Checks this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

add_subdirectory(GFX-lib)
add_subdirectory(FatFS build)

target_sources(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/MDMA.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/MidiCore.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/usb_descriptors.c
        ${CMAKE_CURRENT_SOURCE_DIR}/tud_audio_callbacks.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tud_callbacks.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/fatfs_config.c
        ${CMAKE_CURRENT_SOURCE_DIR}/utils/tools.c
        ${CMAKE_CURRENT_SOURCE_DIR}/Pots/Pots.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Buttons/Buttons.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Menu/MainMenu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Menu/card_menu.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Dexed/TG.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/i2s/I2S.cpp
)


pico_enable_stdio_usb(${PROJECT} 0)
pico_enable_stdio_uart(${PROJECT} 1)

# Example include
target_include_directories(${PROJECT} PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/utils
        ${CMAKE_CURRENT_SOURCE_DIR}/Pots
        ${CMAKE_CURRENT_SOURCE_DIR}/Buttons
        ${CMAKE_CURRENT_SOURCE_DIR}/Menu
        ${CMAKE_CURRENT_SOURCE_DIR}/Dexed
        ${CMAKE_CURRENT_SOURCE_DIR}/GFX-lib
        ${CMAKE_CURRENT_SOURCE_DIR}/FatFs/include
        ${CMAKE_CURRENT_SOURCE_DIR}/i2s
)

target_link_libraries(${PROJECT} PUBLIC
    pico_stdlib
    pico_multicore
    hardware_adc
    hardware_spi
    hardware_gpio
    hardware_pio
    hardware_dma
    pico_bootsel_via_double_reset
    GFX-lib
    FatFs_SPI
)

target_compile_definitions(${PROJECT} PRIVATE
  USE_USB_AUDIO
  HAGL_HAL_USE_SINGLE_BUFFER
  #HAGL_HAL_USE_DOUBLE_BUFFER
  #HAGL_HAL_USE_TRIPLE_BUFFER
  #HAGL_HAL_USE_DMA
  HAGL_HAL_DEBUG
  MIPI_DISPLAY_SPI_CLOCK_SPEED_HZ=24000000
  MIPI_DISPLAY_PIN_CS=13
  MIPI_DISPLAY_PIN_DC=7 # A0
  MIPI_DISPLAY_PIN_RST=6
  MIPI_DISPLAY_PIN_BL=-1
  MIPI_DISPLAY_PIN_CLK=10
  MIPI_DISPLAY_PIN_MOSI=11 # SDA
  MIPI_DISPLAY_PIN_MISO=12
  MIPI_DISPLAY_PIXEL_FORMAT=MIPI_DCS_PIXEL_FORMAT_16BIT
  MIPI_DISPLAY_ADDRESS_MODE=MIPI_DCS_ADDRESS_MODE_RGB|MIPI_DCS_ADDRESS_MODE_SWAP_XY|MIPI_DCS_ADDRESS_MODE_MIRROR_Y 
  MIPI_DISPLAY_WIDTH=160
  MIPI_DISPLAY_HEIGHT=128
  MIPI_DISPLAY_OFFSET_X=0
  MIPI_DISPLAY_OFFSET_Y=0
  MIPI_DISPLAY_INVERT=0
)

pico_set_program_name(${PROJECT} "Dexed Master")
pico_set_program_version(${PROJECT} "1.0" )

# Configure compilation flags and libraries for the example... see the corresponding function
# in hw/bsp/FAMILY/family.cmake for details.
family_configure_device_example(${PROJECT})

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND picotool reboot -f -u 
    VERBATIM
    COMMAND ping -n 2 localhost 2> nul
    VERBATIM
    COMMAND	picotool load -x ${PROJECT}.uf2
    VERBATIM
)