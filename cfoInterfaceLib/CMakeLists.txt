set(NODEJS_ADDON_NAME "CFO")

cet_make_library(LIBRARY_NAME CFOInterface SOURCE
            CFO.cpp
            CFOLibTest.cpp
            CFOSoftwareCFO.cpp
			CFO_Registers.cpp
			CFO_Packets.cpp
            CFO_Types.cpp
            cfodev.cpp
	    cfosim.cpp
        LIBRARIES
        pthread
)

cet_make_exec(cfoUtil SOURCE util_main.cc LIBRARIES CFOInterface)

cet_make_exec(CFORegDump SOURCE cfoRegDump.cc LIBRARIES CFOInterface)

cet_make_exec(cfo_cntl SOURCE my_cntl.cc LIBRARIES CFOInterface)

create_nodejs_addon(ADDON_NAME "CFO" LIBRARIES CFOInterface INCLUDES ${CMAKE_CURRENT_SOURCE_DIR} )

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()


cet_script(Setup_CFO.sh)

