%include "stdint.i"
%module CFOLibTest

%rename(Equals) operator=;

%{
#include "CFOLibTest.h"
using namespace CFOLib;
%}

%include "std_vector.i"
%include "std_string.i"

%include "CFOLibTest.h"
