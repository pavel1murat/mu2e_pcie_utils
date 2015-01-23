include_directories(SYSTEM "${CMAKE_SOURCE_DIR}/linux_driver/mymodule2")

add_library(DTCInterface SHARED
            DTC.cc
            DTC_Types.cc
            mu2edev.cc
)

add_executable(DTCLibTest main.cc)
target_link_libraries(DTCLibTest DTCInterface)

add_custom_command(
    OUTPUT DTC.node
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND V=1 ${CMAKE_CURRENT_SOURCE_DIR}/build.sh
    DEPENDS DTC.cc DTC_Types.cc mu2edev.cc
)
add_custom_target(DTCJSInterface ALL echo DEPENDS DTC.node)
file(INSTALL DTC.node DESTINATION ../bin/web/modules/DTC/server)
add_custom_command(TARGET DTCJSInterface PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/ ../bin/web/
)
