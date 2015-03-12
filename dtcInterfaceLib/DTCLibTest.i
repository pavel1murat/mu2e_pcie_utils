%include "stdint.i"
%module DTCLibTest

%rename(Equals) operator=;

%{
#include "DTCLibTest.h"
using namespace DTCLib;
%}

%include "std_vector.i"
%include "std_string.i"

%include "DTCLibTest.h"
