 // This file (util_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
 // May 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
 // or COPYING file. If you do not have such a file, one can be obtained by
 // contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
 // $RCSfile: .emacs.gnu,v $
 // rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <stdio.h>		// printf
#include <iostream>
#include "DTC.h"
#ifdef _WIN32
# include <chrono>
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
# define TRACE(...)
#else
# include "trace.h"
#endif

int
main(  int	argc
     , char	*argv[] )
{
    DTCLib::DTC *thisDTC = new DTCLib::DTC();
    DTCLib::DTC_DataHeaderPacket packet = thisDTC->ReadNextDAQPacket();
    std::cout << packet.toJSON() << '\n';
    return (0);
}   // main
