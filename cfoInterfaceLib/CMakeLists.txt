cet_make_library(LIBRARY_NAME CFOInterface SOURCE
			CFO_Registers.cpp
			CFO_Packets.cpp
            CFO_Types.cpp
            cfodev.cpp
        LIBRARIES
        pthread
)

cet_make_exec(CFORegDump SOURCE cfoRegDump.cc LIBRARIES CFOInterface)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()
