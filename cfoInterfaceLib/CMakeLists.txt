cet_make_library(LIBRARY_NAME CFOInterface SOURCE
		CFO_Registers.cpp
		CFO_Compiler.cpp
        LIBRARIES
		DTCInterface
        pthread
)

cet_make_exec(CFORegDump SOURCE cfoRegDump.cc LIBRARIES CFOInterface)
cet_make_exec(mu2eCompiler SOURCE mu2eCompiler.cc LIBRARIES CFOInterface)

# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()
