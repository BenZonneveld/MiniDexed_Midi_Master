
    add_library(hagl INTERFACE)

    target_sources(hagl INTERFACE
        ${CMAKE_CURRENT_LIST_DIR}/src/bitmap.c
        ${CMAKE_CURRENT_LIST_DIR}/src/clip.c
        ${CMAKE_CURRENT_LIST_DIR}/src/fontx.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hagl.c
        ${CMAKE_CURRENT_LIST_DIR}/src/hsl.c
        ${CMAKE_CURRENT_LIST_DIR}/src/rgb565.c
        ${CMAKE_CURRENT_LIST_DIR}/src/rgb888.c
        ${CMAKE_CURRENT_LIST_DIR}/src/tjpgd.c
    )

    target_include_directories(hagl INTERFACE ${CMAKE_CURRENT_LIST_DIR}/include)

    target_link_libraries(hagl INTERFACE hagl_hal)
