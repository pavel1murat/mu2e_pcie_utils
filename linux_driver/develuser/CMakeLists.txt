include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/linux_driver/develmodule")
cet_make_exec(devl SOURCE devl.cc)

install_source()
