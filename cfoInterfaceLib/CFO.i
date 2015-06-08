%module CFO
%include "stdint.i"
%include "carrays.i"
%array_functions(uint8_t, u8array)

%rename(Equals) operator=;

%{
#include "linux_driver/mymodule2/mu2e_mmap_ioctl.h"

#include "CFO_Types.h"
#include "CFO.h"
using namespace CFOLib;
%}

%include "std_vector.i"
%include "std_string.i"

%include "../linux_driver/mymodule2/mu2e_mmap_ioctl.h"

%include "CFO_Types.h"
%include "CFO.h"
