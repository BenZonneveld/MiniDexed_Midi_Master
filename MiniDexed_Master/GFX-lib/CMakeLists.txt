add_library(GFX-lib INTERFACE)
target_sources(GFX-lib INTERFACE
	${CMAKE_CURRENT_LIST_DIR}/Adafruit_GFX.cpp
	${CMAKE_CURRENT_LIST_DIR}/Adafruit_SPITFT.cpp
	${CMAKE_CURRENT_LIST_DIR}/glcdfont.c
	${CMAKE_CURRENT_LIST_DIR}/Print.cpp
)

target_include_directories(GFX-lib INTERFACE
	${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(GFX-lib INTERFACE
        hardware_spi
        hardware_dma
		hardware_pwm
		hardware_gpio
        pico_stdlib
)
