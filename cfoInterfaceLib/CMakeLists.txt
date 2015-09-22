cet_make_library(LIBRARY_NAME CFOInterface SOURCE
            CFO.cc
            CFOLibTest.cc
            CFO_Types.cc
            mu2edev.cc
	    mu2esim.cc
)

cet_make_exec(CFOLibTest SOURCE main.cc LIBRARIES CFOInterface)
target_link_libraries(CFOLibTest pthread)

cet_make_exec(mu2eUtil_CFO SOURCE util_main.cc LIBRARIES CFOInterface)

cet_make_exec(CFORegDump SOURCE cfoRegDump.cc LIBRARIES CFOInterface)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/CFO/server/CFO.node
           ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/CFO/server/CFOLibTest.node
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND V=1 ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/ ${CMAKE_CURRENT_BINARY_DIR}/bin/web/
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/CFO.node ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/CFO/server/CFO.node
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/CFOLibTest.node  ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/CFO/server/CFOLibTest.node
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/CFO.cc 
            ${CMAKE_CURRENT_SOURCE_DIR}/CFOLibTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/CFO_Types.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2edev.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2esim.cc
)
add_custom_target(CFOJSInterface ALL 
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/CFO/server/CFO.node
            ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/CFO/server/CFOLibTest.node
)
add_dependencies(CFOJSInterface CFOInterface)


install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/web DESTINATION ${${product}_bin_dir})

