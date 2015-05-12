include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../develmodule")
cet_make_exec(devl SOURCE devl.cc)

install_source()
