// This program is a simple Register Dump module
// Eric Flumerfelt, FNAL RSI
// 5/15/2015

#include <iostream>
#ifndef _WIN32
# include "dtcInterfaceLib/DTC.h"
#else
# include "DTC.h"
#endif

int main(int argc, char** argv)
{
    DTCLib::DTC* thisDTC = new DTCLib::DTC();

    std::cout << thisDTC->RegDump() << std::endl;

    return 0;
}

