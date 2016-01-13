cet_make_library(LIBRARY_NAME DTCInterface SOURCE
            DTC.cc
            DTCLibTest.cc
            DTCSoftwareCFO.cc
			DTC_Registers.cc
			DTC_Packets.cc
            DTC_Types.cc
            mu2edev.cc
	    mu2esim.cc
        LIBRARIES
        pthread
)

cet_make_exec(DTCLibTest SOURCE main.cc LIBRARIES DTCInterface)

cet_make_exec(mu2eUtil SOURCE util_main.cc LIBRARIES DTCInterface)

cet_make_exec(rocUtil SOURCE dcs_main.cc LIBRARIES DTCInterface)

cet_make_Exec(requestSender SOURCE requestSender.cc LIBRARIES DTCInterface)

cet_make_exec(DTCRegDump SOURCE dtcRegDump.cc LIBRARIES DTCInterface)

cet_make_exec(tester SOURCE tester.cc LIBRARIES DTCInterface)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/DTC/server/DTC.node 
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMAND V=1 ${CMAKE_CURRENT_SOURCE_DIR}/build.sh ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/web/ ${CMAKE_CURRENT_BINARY_DIR}/bin/web/
    COMMAND ${CMAKE_COMMAND} -E rename ${CMAKE_CURRENT_BINARY_DIR}/DTC.node ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/DTC/server/DTC.node
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/DTC.cc 
            ${CMAKE_CURRENT_SOURCE_DIR}/DTCLibTest.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/DTCSoftwareCFO.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/DTC_Types.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/DTC_Registers.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/DTC_Packets.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2edev.cc
            ${CMAKE_CURRENT_SOURCE_DIR}/mu2esim.cc
)
add_custom_target(DTCJSInterface ALL 
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/bin/web/modules/DTC/server/DTC.node
)
add_dependencies(DTCJSInterface DTCInterface)

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/web DESTINATION ${${product}_bin_dir})


cet_script(updateMu2e.sh Setup_DTC.sh)
