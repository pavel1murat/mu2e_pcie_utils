%include "stdint.i"
%module DTC

%rename(Equals) operator=;
%ignore DTC::DTC_DataPacket::DTC_DataPacket(mu2e_databuff_t*);

%{
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"
#include "DTC_Types.h"
#include "DTC.h"
using namespace DTC;
%}

%include "std_vector.i"
%include "std_string.i"

%include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"
%include "DTC_Types.h"
%include "DTC.h"
