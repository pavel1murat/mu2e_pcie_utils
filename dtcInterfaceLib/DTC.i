%include "stdint.i"
%module DTC

%rename(Equals) operator=;
%ignore DTC::DTC_DataPacket::DTC_DataPacket(mu2e_databuff_t*);

%{
#include "DTC_Types.h"
#include "DTC.h"
using namespace DTC;
%}

%include "std_vector.i"

%include "DTC_Types.h"
%include "DTC.h"
