cet_make_library(LIBRARY_NAME DTCInterface SOURCE
            DTC.cc
            DTCLibTest.cc
            DTCSoftwareCFO.cc
            DTC_Types.cc
            mu2edev.cc
	    mu2esim.cc
        LIBRARIES
        pthread
)

cet_make_exec(DTCLibTest SOURCE main.cc LIBRARIES DTCInterface)

cet_make_exec(mu2eUtil SOURCE util_main.cc LIBRARIES DTCInterface)

cet_make_Exec(requestSender SOURCE requestSender.cc LIBRARIES DTCInterface)

cet_make_exec(DTCRegDump SOURCE dtcRegDump.cc LIBRARIES DTCInterface)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

add_custom_command(
    OUTPUT DTC.node
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND V=1 ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS DTC.cc DTC_Types.cc mu2edev.cc mu2esim.cc
)
add_custom_target(DTCJSInterface ALL echo DEPENDS DTC.node)
#add_custom_command(TARGET DTCJSInterface PRE_BUILD
#                   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/  ${CMAKE_BINARY_DIR}/bin/web/
#)
add_custom_command(TARGET DTCJSInterface POST_BUILD
    #   COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/DTC.node ../bin/web/modules/DTC/server/DTC.node
    #COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/DTCLibTest.node ../bin/web/modules/DTC/server/DTCLibTest.node
   COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/ ${CMAKE_CURRENT_BINARY_DIR}/bin/web/
   COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/DTC.node ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/DTC/server/DTC.node
   COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/DTCLibTest.node  ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/DTC/server/DTCLibTest.node
)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/web DESTINATION ${${product}_bin_dir})


cet_script(updateMu2e.sh)

