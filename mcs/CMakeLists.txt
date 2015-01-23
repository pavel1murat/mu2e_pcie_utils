include_directories("${CMAKE_SOURCE_DIR}/linux_driver/develmodule")

cet_make_exec( mcs SOURCE mcs.cc )

install_headers()
install_source()
