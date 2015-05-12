include_directories(SYSTEM "${CMAKE_CURRENT_SOURCE_DIR}/../mymodule2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
cet_make_exec(my_cntl SOURCE my_cntl.cc mu2edev.cc)
cet_make_exec(sizeof_buffdesc SOURCE sizeof_buffdesc.cc)

install_source()
install_headers()
