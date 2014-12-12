%include "stdint.i"
%module DTC

%rename(Equals) operator=;

%{
#include "DTC_Types.h"
#include "DTC.h"
using namespace DTC;
%}

%include "DTC_Types.h"
%include "DTC.h"
