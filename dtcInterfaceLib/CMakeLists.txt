cet_make_library(LIBRARY_NAME DTCInterface SOURCE
            DTC.cc
            DTCLibTest.cc
            DTC_Types.cc
            mu2edev.cc
	    mu2esim.cc
)

cet_make_exec(DTCLibTest SOURCE main.cc LIBRARIES DTCInterface)

add_custom_command(
    OUTPUT DTC.node
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND V=1 ${CMAKE_CURRENT_SOURCE_DIR}/build.sh
    DEPENDS DTC.cc DTC_Types.cc mu2edev.cc mu2esim.cc
)
add_custom_target(DTCJSInterface ALL echo DEPENDS DTC.node)
add_custom_command(TARGET DTCJSInterface PRE_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/ ../bin/web/
)
add_custom_command(TARGET DTCJSInterface POST_BUILD
   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/DTC.node ../bin/web/modules/DTC/server/DTC.node
   COMMAND ${CMAKE_COMMAND} -E copy
${CMAKE_CURRENT_SOURCE_DIR}/DTCLibTest.node
../bin/web/modules/DTC/server/DTCLibTest.node
   COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_SOURCE_DIR}/DTC.node ${CMAKE_CURRENT_SOURCE_DIR}/web/modules/DTC/server/DTC.node
   COMMAND ${CMAKE_COMMAND} -E rename
${CMAKE_CURRENT_SOURCE_DIR}/DTCLibTest.node
${CMAKE_CURRENT_SOURCE_DIR}/web/modules/DTC/server/DTCLibTest.node
)
install(DIRECTORY web DESTINATION ${${product}_bin_dir})

cet_script(updateMu2e.sh)

install_source()
install_headers()
