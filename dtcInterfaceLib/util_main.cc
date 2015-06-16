// This file (util_main.cc) was created by Ron Rechenmacher <ron@fnal.gov> on
// May 13, 2015. "TERMS AND CONDITIONS" governing this file are in the README
// or COPYING file. If you do not have such a file, one can be obtained by
// contacting Ron or Fermi Lab in Batavia IL, 60510, phone: 630-840-3000.
// $RCSfile: .emacs.gnu,v $
// rev="$Revision: 1.23 $$Date: 2012/01/23 15:32:40 $";

#include <cstdio>		// printf
#include <cstdlib>		// strtoul
#include <iostream>
#include "DTC.h"
#ifdef _WIN32
# include <chrono>
# include <thread>
# define usleep(x)  std::this_thread::sleep_for(std::chrono::microseconds(x));
# ifndef TRACE
#  include <stdio.h>
#  ifdef _DEBUG
#   define TRACE(lvl,...) printf(__VA_ARGS__); printf("\n")
#  else
#   define TRACE(...)
#  endif
# endif
# define TRACE_CNTL(...)
#else
# include "trace.h"
# include <unistd.h>		// usleep
#endif

using namespace DTCLib;
using namespace std;

int
main(int	argc
, char	*argv[])
{
    if (argc > 1 && strcmp(argv[1], "read") == 0)
    {
        DTC *thisDTC = new DTC(DTC_SimMode_Hardware);
        DTC_DataHeaderPacket packet = thisDTC->ReadNextDAQPacket();
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
            int sts = device.read_data(DTC_DMA_Engine_DAQ, &buffer, tmo_ms);
            TRACE(1, "util - read for DAQ - ii=%u sts=%d %p", ii, sts, buffer);
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
            int stsRD = device.read_data(DTC_DMA_Engine_DAQ, &buffer, tmo_ms);
            int stsRL = device.read_release(DTC_DMA_Engine_DAQ, 1);
            TRACE(12, "util - release/read for DAQ and DCS ii=%u stsRD=%d stsRL=%d %p", ii, stsRD, stsRL, buffer);
            usleep(0);
        }
    }
    else if (argc > 1 && strcmp(argv[1], "DTC") == 0)
    {
        DTC *thisDTC = new DTC(DTC_SimMode_Hardware);
        thisDTC->EnableRing(DTC_Ring_0, DTC_RingEnableMode(true, true, false), DTC_ROC_0);
        thisDTC->SetInternalSystemClock();
        thisDTC->DisableTiming();
        thisDTC->SetMaxROCNumber(DTC_Ring_0, DTC_ROC_0);
        unsigned gets = 1;
        if (argc > 2) gets = strtoul(argv[2], NULL, 0);
        for (unsigned ii = 0; ii < gets; ++ii)
        {
            vector<void*> data = thisDTC->GetData(DTC_Timestamp((uint64_t)ii));
            if (data.size() > 0)
            {
                cout << data.size() << " packets returned\n";
                for (size_t i = 0; i < data.size(); ++i)
                {
                    TRACE(19, "DTC::GetJSONData constructing DataPacket:");
                    DTC_DataPacket     test = DTC_DataPacket(data[i]);
                    //cout << test.toJSON() << '\n'; // dumps whole databuff_t
                    printf("data@%p=0x%08x\n", data[i], *(uint32_t*)(data[i]));
                    //DTC_DataHeaderPacket h1 = DTC_DataHeaderPacket(data[i]);
                    //cout << h1.toJSON() << '\n';
                    DTC_DataHeaderPacket h2 = DTC_DataHeaderPacket(test);
                    cout << h2.toJSON() << '\n';
                    for (int jj = 0; jj < h2.GetPacketCount(); ++jj) {
                        cout << "\t" << DTC_DataPacket(((uint8_t*)data[i]) + ((jj + 1) * 16)).toJSON() << endl;
                    }
                }
            }
            else
            {
                TRACE_CNTL("modeM", 0L);
                cout << "no data returned\n";
                return (0);
            }
        }
    }
    else// if (argc > 1 && strcmp(argv[1],"get")==0)
    {
        DTC *thisDTC = new DTC(DTC_SimMode_Hardware);
        unsigned gets = 1000000;
        if (argc > 1) gets = strtoul(argv[1], NULL, 0);

        for (unsigned ii = 0; ii < gets; ++ii)
        {
            vector<void*> data = thisDTC->GetData(DTC_Timestamp((uint64_t)ii));
            if (data.size() > 0)
            {
                //cout << data.size() << " packets returned\n";
                for (size_t i = 0; i < data.size(); ++i)
                {
                    TRACE(19, "DTC::GetJSONData constructing DataPacket:");
                    DTC_DataPacket     test = DTC_DataPacket(data[i]);
                    //cout << test.toJSON() << '\n'; // dumps whole databuff_t
                    //printf("data@%p=0x%08x\n", data[i], *(uint32_t*)(data[i]));
                    //DTC_DataHeaderPacket h1 = DTC_DataHeaderPacket(data[i]);
                    //cout << h1.toJSON() << '\n';
                    DTC_DataHeaderPacket h2 = DTC_DataHeaderPacket(test);
                    //cout << h2.toJSON() << '\n';
                    // for (int jj = 0; jj < h2.GetPacketCount(); ++jj) {
                    //    cout << "\t" << DTC_DataPacket(((uint8_t*)data[i]) + ((jj + 1) * 16)).toJSON() << endl;
                    //}
                }
            }
            else
            {
                TRACE_CNTL("modeM", 0L);
                cout << "no data returned\n";
                return (0);
            }
        }
    }
    return (0);
}   // main
