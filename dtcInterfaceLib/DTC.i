%module DTC
%include "stdint.i"
%include "carrays.i"
%array_functions(uint8_t, u8array)

%rename(Equals) operator=;
%ignore DTC::DTC_DataPacket::DTC_DataPacket(mu2e_databuff_t*);

%{
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"

#include "DTC_Types.h"
#include "DTC.h"
using namespace DTC;
%}

%template(readDCSRequestPacket) DTC::ReadDMAPacket<DTC::DTC_DCSRequestPacket>;
%template(readReadoutRequestPacket) DTC::ReadDMAPacket<DTC::DTC_ReadoutRequestPacket>;
%template(readDataRequestPacket) DTC::ReadDMAPacket<DTC::DTC_DataRequestPacket>;
%template(readDCSReplyPacket) DTC::ReadDMAPacket<DTC::DTC_DCSReplyPacket>;
%template(readDataHeaderPacket) DTC::ReadDMAPacket<DTC::DTC_DataHeaderPacket>;

%include "std_vector.i"
%include "std_string.i"

%include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"

%include "DTC_Types.h"
%include "DTC.h"
