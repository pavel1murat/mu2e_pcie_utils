%include "stdint.i"
%module DTC
%{
#include "DTC.h"
#include "DTC_Types.h"
using namespace DTC;
%}

%include "DTC.h"
%include "DTC_Types.h"

%typemap(in) DTC_RingID {
  $1 = static_cast<DTC_RingID>($input);
 }
