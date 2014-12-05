%include "stdint.i"
%module DTC
%{
#include "DTC_Types.h"
#include "DTC.h"
using namespace DTC;
%}

%include "DTC_Types.h"
%include "DTC.h"
