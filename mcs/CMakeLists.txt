include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../linux_driver/develmodule")

cet_make_exec( mcs SOURCE mcs.cc )

cet_script( mcs.sh )

install_headers()
install_source()
