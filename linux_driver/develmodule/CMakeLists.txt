add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko
    COMMAND make CC=/usr/bin/gcc
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/pci_devel_main.ko ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko
    COMMAND make clean
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pci_devel_main.c
)

add_custom_target(pci_devel_module ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/../../bin/pci_devel_main.ko DESTINATION ${${product}_bin_dir})

install_headers()
install_source()
