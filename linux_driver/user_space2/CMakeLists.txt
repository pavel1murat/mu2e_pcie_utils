include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/linux_driver/mymodule2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-missing-field-initializers")
add_executable(my_cntl my_cntl.cc mu2edev.cc)
add_executable(sizeof_buffdesc sizeof_buffdesc.cc)
