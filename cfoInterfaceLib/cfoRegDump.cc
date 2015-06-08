// This program is a simple Register Dump module
// Eric Flumerfelt, FNAL RSI
// 5/15/2015

#include <iostream>
#ifndef _WIN32
# include "cfoInterfaceLib/CFO.h"
#else
# include "CFO.h"
#endif

int main(int argc, char** argv)
{
    CFOLib::CFO* thisCFO = new CFOLib::CFO();

    if (argc > 1) {
        std::cout << thisCFO->RegDump() << std::endl;
        std::cout << std::endl << std::endl;
    }
    std::cout << thisCFO->ConsoleFormatRegDump() << std::endl;

    return 0;
}

