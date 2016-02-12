// This program is a simple Register Dump module
// Eric Flumerfelt, FNAL RSI
// 5/15/2015

#include <iostream>
#ifndef _WIN32
# include "dtcInterfaceLib/DTC_Registers.h"
#else
# include "DTC_Registers.h"
#endif

int main(int argc, char** argv)
{
	DTCLib::DTC_Registers* thisDTC = new DTCLib::DTC_Registers();

	std::cout << thisDTC->FormattedRegDump() << std::endl;

	return 0;
}
