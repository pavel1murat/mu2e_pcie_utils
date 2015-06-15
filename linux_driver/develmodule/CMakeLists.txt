add_custom_target(
    pci_devel_module ALL
    COMMAND make CC=/usr/bin/gcc
    COMMAND mv pci_devel_main.ko ..
    COMMAND make clean
    COMMAND mv ../pci_devel_main.ko .
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET pci_devel_module POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/pci_devel_main.ko ../bin/pci_devel_main.ko
)
install(FILES pci_devel_main.ko DESTINATION ${${product}_bin_dir})

install_headers()
install_source()
