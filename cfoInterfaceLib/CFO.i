%module CFO
%include "stdint.i"
%include "carrays.i"
%array_functions(uint8_t, u8array)

%rename(Equals) operator=;
%ignore CFO::CFO_DataPacket::CFO_DataPacket(cfo_databuff_t*);

%{
#include "cfo_driver/cfo_mmap_ioctl.h"

#include "CFO_Types.h"
#include "CFO_Packets.h"
#include "CFO_Registers.h"
#include "CFO.h"
using namespace CFOLib;
%}

%include "std_vector.i"
%include "std_string.i"

%include "../cfo_driver/cfo_mmap_ioctl.h"

%include "CFO_Types.h"
%include "CFO_Packets.h"
%include "CFO_Registers.h"
%include "CFO.h"

