set(NODEJS_ADDON_NAME "DTC")

cet_make_library(LIBRARY_NAME DTCInterface SOURCE
            DTC.cpp
            DTCLibTest.cpp
            DTCSoftwareCFO.cpp
			DTC_Registers.cpp
			DTC_Packets.cpp
            DTC_Types.cpp
            mu2edev.cpp
	    mu2esim.cpp
        LIBRARIES
        pthread
)

cet_make_exec(mu2eUtil SOURCE util_main.cc LIBRARIES DTCInterface)

cet_make_exec(rocUtil SOURCE dcs_main.cc LIBRARIES DTCInterface)

cet_make_Exec(requestSender SOURCE requestSender.cc LIBRARIES DTCInterface)

cet_make_exec(DTCRegDump SOURCE dtcRegDump.cc LIBRARIES DTCInterface)

cet_make_exec(my_cntl SOURCE my_cntl.cc LIBRARIES DTCInterface)

create_nodejs_addon(ADDON_NAME "DTC" LIBRARIES DTCInterface INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} )

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

FILE(COPY web DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/bin)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/web DESTINATION ${${product}_bin_dir})

cet_script(updateMu2e.sh Setup_DTC.sh)

add_subdirectory(test)
