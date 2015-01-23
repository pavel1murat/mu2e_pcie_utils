include_directories("${CMAKE_SOURCE_DIR}/linux_driver/develmodule")

add_executable( mcs mcs.cc )

install_headers()
install_source()
