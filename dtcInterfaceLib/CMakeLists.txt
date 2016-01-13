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

cet_make_exec(DTCLibTest SOURCE main.cc LIBRARIES DTCInterface)

cet_make_exec(mu2eUtil SOURCE util_main.cc LIBRARIES DTCInterface)

cet_make_exec(rocUtil SOURCE dcs_main.cc LIBRARIES DTCInterface)

cet_make_Exec(requestSender SOURCE requestSender.cc LIBRARIES DTCInterface)

cet_make_exec(DTCRegDump SOURCE dtcRegDump.cc LIBRARIES DTCInterface)

cet_make_exec(tester SOURCE tester.cc LIBRARIES DTCInterface)

set(NODEJS_ADDON_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} )
set(NODEJS_ADDON_LIBS "DTCInterface")

create_nodejs_addon( ${NODEJS_ADDON_NAME} ${NODEJS_ADDON_INCLUDES} ${NODEJS_ADDON_LIBS} )

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()

install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin/web DESTINATION ${${product}_bin_dir})

cet_script(updateMu2e.sh Setup_DTC.sh)
