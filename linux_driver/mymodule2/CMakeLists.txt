add_custom_target(
    mymodule2 ALL
    COMMAND make CC=/usr/bin/gcc
    COMMAND mv mu2e.ko ..
    COMMAND make clean
    COMMAND mv ../mu2e.ko .
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_command(TARGET mymodule2 POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/mu2e.ko ../bin/mu2e.ko
)
install(FILES mu2e.ko DESTINATION ${${product}_bin_dir})

install_source()
install_headers()
