// This program is a simple Register Dump module
// Eric Flumerfelt, FNAL RSI
// 5/15/2015

#include <sys/ioctl.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#ifndef _WIN32
# include "dtcInterfaceLib/DTC_Registers.h"
#else
# include "DTC_Registers.h"
#endif

int main()
{
	auto thisDTC = new DTCLib::DTC_Registers();

    int cols = 80;
    int lines = 24;

#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    cols = ts.ts_cols;
    lines = ts.ts_lines;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    cols = ts.ws_col;
    lines = ts.ws_row;
#endif /* TIOCGSIZE */

    printf("Terminal is %dx%d\n", cols, lines);


    if(cols > 400) { cols = 120; }

    std::cout << thisDTC->FormattedRegDump(cols) << std::endl;

    std::cout << std::endl << std::endl;
    std::cout << thisDTC->RingCountersRegDump(cols);

    
    std::cout << std::endl << std::endl;
    std::cout << thisDTC->PerformanceMonitorRegDump(cols);

	delete thisDTC;
	return 0;
}

