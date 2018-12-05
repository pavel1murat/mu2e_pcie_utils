%module DTC
%include "stdint.i"
%include "carrays.i"
%array_functions(uint8_t, u8array)

%rename(Equals) operator=;
%rename(Stream) operator<<;
%rename(Compare) operator==;
%rename(NotEquals) operator!=;
%rename(LessThan) operator<;
%rename(Plus) operator+;

%ignore DTC_DataPacket::DTC_DataPacket(mu2e_databuff_t*);
%ignore ReadNextDAQPacket;
%ignore ReadNextDCSPacket();

%{
#include "mu2e_driver/mu2e_mmap_ioctl.h"

#include "DTC_Types.h"
#include "DTC_Packets.h"
#include "DTC_Registers.h"
#include "DTC.h"
using namespace DTCLib;
%}

%include "std_vector.i"
%include "std_string.i"

%include "../mu2e_driver/mu2e_mmap_ioctl.h"

%include "DTC_Types.h"
%include "DTC_Packets.h"
%include "DTC_Registers.h"
%include "DTC.h"
