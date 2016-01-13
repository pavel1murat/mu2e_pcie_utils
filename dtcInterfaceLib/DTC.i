%module DTC
%include "stdint.i"
%include "carrays.i"
%array_functions(uint8_t, u8array)

%rename(Equals) operator=;
%ignore DTC::DTC_DataPacket::DTC_DataPacket(mu2e_databuff_t*);

%{
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"

#include "DTC_Types.h"
#include "DTC_Packets.h"
#include "DTC_Registers.h"
#include "DTC.h"
using namespace DTCLib;
%}

%include "std_vector.i"
%include "std_string.i"

%include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"

%include "DTC_Types.h"
%include "DTC_Packets.h"
%include "DTC_Registers.h"
%include "DTC.h"

