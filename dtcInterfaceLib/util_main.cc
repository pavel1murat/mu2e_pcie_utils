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

using namespace DTCLib;
using namespace std;

int
main(  int	argc
     , char	*argv[] )
{
    DTC *thisDTC = new DTC(DTC_SimMode_Hardware);

    if      (argc > 1 && strcmp(argv[1],"read")==0)
    {
	DTC_DataHeaderPacket packet = thisDTC->ReadNextDAQPacket();
	cout << packet.toJSON() << '\n';
    }
    else if (argc > 1 && strcmp(argv[1],"release")==0)
    {   thisDTC->ReleaseAllBuffers( DTC_DMA_Engine_DAQ );
    }
    else// if (argc > 1 && strcmp(argv[1],"get")==0)
    {
	vector<void*> data=thisDTC->GetData( DTC_Timestamp((uint64_t)0) );
	if (data.size() > 0)
	{   cout << data.size() << " packets returned\n";
	    for (size_t i = 0; i < data.size(); ++i)
	    {   TRACE(19, "DTC::GetJSONData constructing DataPacket:");
		DTC_DataPacket     test = DTC_DataPacket(data[i]);
		cout << test.toJSON() << '\n';
		printf("data@%p=0x%08x\n", data[i], *(uint32_t*)(data[i]) );
		//DTC_DataHeaderPacket h1 = DTC_DataHeaderPacket(data[i]);
		//cout << h1.toJSON() << '\n';
		DTC_DataHeaderPacket h2 = DTC_DataHeaderPacket(test);
		cout << h2.toJSON() << '\n';
	    }
	}
	else
	    cout << "no data returned\n";
    }
    return (0);
}   // main
