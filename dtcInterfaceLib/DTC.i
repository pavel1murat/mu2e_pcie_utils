%include "stdint.i"
%module DTC

%rename(Equals) operator=;
%ignore DTC::DTC_DataPacket::DTC_DataPacket(mu2e_databuff_t,bool);

%{
#include "DTC_Types.h"
#include "DTC.h"
using namespace DTC;
%}

%include "DTC_Types.h"
%include "DTC.h"
