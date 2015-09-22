add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../../bin/mu2e.ko
    COMMAND make CC=/usr/bin/gcc
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/mu2e.ko ${CMAKE_CURRENT_BINARY_DIR}/../../bin/mu2e.ko
    COMMAND make clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/mu2e_event.c
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2e_fs.c
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2e_main.c
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2e_pci.c
)

add_custom_target(mymodule2 ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../../bin/mu2e.ko)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../../bin/mu2e.ko DESTINATION ${${product}_bin_dir})

install_source()
install_headers()
