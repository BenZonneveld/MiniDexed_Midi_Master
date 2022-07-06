cmake_minimum_required(VERSION 3.5)

include(${PICO_SDK_PATH}/lib/tinyusb/hw/bsp/family_support.cmake)

# gets PROJECT name for the example (e.g. <BOARD>-<DIR_NAME>)
family_get_project_name(PROJECT ${CMAKE_CURRENT_LIST_DIR})

project(${PROJECT})

# Checks this example is valid for the family and initializes the project
family_initialize_project(${PROJECT} ${CMAKE_CURRENT_LIST_DIR})

add_executable(${PROJECT})

pico_generate_pio_header(${PROJECT} ${CMAKE_CURRENT_LIST_DIR}/UAC2/i2s_reader.pio)

# Example source
target_sources(${PROJECT} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/UAC2/i2s.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/UAC2/usb_audio.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/usb_descriptors.c
)

# Example include
target_include_directories(${PROJECT} PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/
)

pico_enable_stdio_uart(${PROJECT} 1)

target_link_libraries(${PROJECT} PUBLIC
	pico_stdlib
	hardware_pio
	hardware_gpio
	hardware_dma
	hardware_irq
	tinyusb_device
	tinyusb_board
)

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
