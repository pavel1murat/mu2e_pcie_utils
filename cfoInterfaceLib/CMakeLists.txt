
cet_make_library(LIBRARY_NAME CFOInterface SOURCE
            cfodev.cpp
        LIBRARIES
        pthread
)

cet_make_exec(cfo_cntl SOURCE my_cntl.cc LIBRARIES CFOInterface)


# Install_headers MUST BE FIRST...for some reason
install_headers()
install_source()


cet_script(Setup_CFO.sh)

