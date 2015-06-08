// This file (util_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// May 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <cstdio>		// printf
#include <cstdlib>		// strtoul
#include <iostream>
#include "CFO.h"
#ifdef _WIN32
# include <chrono>
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
# define TRACE(...)
# define TRACE_CNTL(...)
#else
# include "trace.h"
# include <unistd.h>		// usleep
#endif

using namespace CFOLib;
using namespace std;

int
main(int	argc
, char	*argv[])
{
    if (argc > 1 && strcmp(argv[1], "read") == 0)
    {
        CFO *thisCFO = new CFO(CFO_SimMode_Hardware);
        CFO_DataHeaderPacket packet = thisCFO->ReadNextDAQPacket();
        cout << packet.toJSON() << '\n';
    }
    else if (argc > 1 && strcmp(argv[1], "read_data") == 0)
    {
        mu2edev device;
        device.init();
        unsigned reads = 1;
        if (argc > 2) reads = strtoul(argv[2], NULL, 0);
        for (unsigned ii = 0; ii < reads; ++ii)
        {
            void *buffer;
            int tmo_ms = 0;
            int sts = device.read_data(CFO_DMA_Engine_DAQ, &buffer, tmo_ms);
            TRACE( 1, "util - read for DAQ - ii=%u sts=%d %p", ii, sts, buffer);
            usleep(0);
        }
    }
    else if (argc > 1 && strcmp(argv[1], "read_release") == 0)
    {
        mu2edev device;
        device.init();
        unsigned releases = 1;
        if (argc > 2) releases = strtoul(argv[2], NULL, 0);
        for (unsigned ii = 0; ii < releases; ++ii)
        {
            void *buffer;
            int tmo_ms = 0;
            int stsRD = device.read_data(CFO_DMA_Engine_DAQ, &buffer, tmo_ms);
            int stsRL = device.read_release(CFO_DMA_Engine_DAQ, 1);
            TRACE(12, "util - release/read for DAQ and DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
            usleep(0);
        }
    }
    else if (argc > 1 && strcmp(argv[1],"CFO")==0)
    {
        CFO *thisCFO = new CFO(CFO_SimMode_Hardware);
	thisCFO->EnableRing(CFO_Ring_0, CFO_RingEnableMode(true, true, false), CFO_ROC_0);
	thisCFO->SetInternalSystemClock();
        thisCFO->DisableTiming();
	thisCFO->SetMaxROCNumber( CFO_Ring_0, CFO_ROC_0 );
        unsigned gets = 1;
        if (argc > 2) gets = strtoul(argv[2], NULL, 0);
        for (unsigned ii = 0; ii < gets; ++ii)
        {
            vector<void*> data = thisCFO->GetData(CFO_Timestamp((uint64_t)ii));
            if (data.size() > 0)
            {
                cout << data.size() << " packets returned\n";
                for (size_t i = 0; i < data.size(); ++i)
                {
                    TRACE(19, "CFO::GetJSONData constructing DataPacket:");
                    CFO_DataPacket     test = CFO_DataPacket(data[i]);
                    //cout << test.toJSON() << '\n'; // dumps whole databuff_t
                    printf("data@%p=0x%08x\n", data[i], *(uint32_t*)(data[i]));
                    //CFO_DataHeaderPacket h1 = CFO_DataHeaderPacket(data[i]);
                    //cout << h1.toJSON() << '\n';
                    CFO_DataHeaderPacket h2 = CFO_DataHeaderPacket(test);
                    cout << h2.toJSON() << '\n';
                    for (int jj = 0; jj < h2.GetPacketCount(); ++jj) {
                        cout << "\t" << CFO_DataPacket(((uint8_t*)data[i]) + ((jj + 1) * 16)).toJSON() << endl;
                    }
                }
            }
            else
	    {   TRACE_CNTL( "modeM", 0L );
                cout << "no data returned\n";
		return (0);
	    }
        }
    }
    else// if (argc > 1 && strcmp(argv[1],"get")==0)
    {
        CFO *thisCFO = new CFO(CFO_SimMode_Hardware);
        unsigned gets = 1;
        if (argc > 1) gets = strtoul(argv[1], NULL, 0);
        for (unsigned ii = 0; ii < gets; ++ii)
        {
            vector<void*> data = thisCFO->GetData(CFO_Timestamp((uint64_t)ii));
            if (data.size() > 0)
            {
                cout << data.size() << " packets returned\n";
                for (size_t i = 0; i < data.size(); ++i)
                {
                    TRACE(19, "CFO::GetJSONData constructing DataPacket:");
                    CFO_DataPacket     test = CFO_DataPacket(data[i]);
                    //cout << test.toJSON() << '\n'; // dumps whole databuff_t
                    printf("data@%p=0x%08x\n", data[i], *(uint32_t*)(data[i]));
                    //CFO_DataHeaderPacket h1 = CFO_DataHeaderPacket(data[i]);
                    //cout << h1.toJSON() << '\n';
                    CFO_DataHeaderPacket h2 = CFO_DataHeaderPacket(test);
                    cout << h2.toJSON() << '\n';
                    for (int jj = 0; jj < h2.GetPacketCount(); ++jj) {
                        cout << "\t" << CFO_DataPacket(((uint8_t*)data[i]) + ((jj + 1) * 16)).toJSON() << endl;
                    }
                }
            }
            else
	    {   TRACE_CNTL( "modeM", 0L );
                cout << "no data returned\n";
		return (0);
	    }
        }
    }
    return (0);
}   // main
