README
------------
MU2ECOMPILER
------------

The Mu2eCompiler translates Mu2e instruction code into binary files that can be later loaded into the DDR of the CFO module.
The CFO module creates the heartbeat packets of the MU2E experiment. The heartbeats control other sections of the MU2E experiments.

Getting Started
To use the MU2ECOMPILER, you must have the Mu2eCompiler.exe file. If not, you must recompile the .cpp file. I used gcc for compilation with C++98.

DEVELOPER'S GUIDE
To understand how the compiler runs, one must understand how instructions and parameters are stored.

*Instruction parameters are stored in 3 string variables and one int_64t variable.
In the case of "WAIT period=32 ns", it gets stored as:
instructionName = "WAIT"
argumentName    = "period"
parameterName   = 32
identifierName      = ns

*On the other hand, macros store only their name in a string variable while the rest is stored in a string DEQUE (double-ended deque).
 The compiler separates the words after the macro name based on spaces and equal signs and assigns each word a place in a deque storage.
In the case of " SLICE event_tag=125 bit_width=3 eventtag=2 ", it gets stored as:
macroName = "SLICE"
macroArgument[0]  = "event_tag"
macroArgument[1]  = "125"
macroArgument[2]  = "bitwidth"
...
macroArgument[5] = "2"

OPERATIONS ON THE COMPILER

1) CREATING NEW INSTRUCTIONS:

Added instructions must follow the syntax established:

a)   [instruction name] [optional argument=][value] [identifier]
     e.g. WAIT period=25 ns or WAIT 25 ns

or

b)   [instruction name] [string value]
     e.g. WAIT NEXT

with that syntax in mind, to add a new instruction you need:

A) Add a directive with the name of the instruction before main in the DIRECTIVES section with a valuenot in use.

e.g. #define [nameOfInstruction] 11

B) Add your instruction to the "instructiontoInt" function.
This will assign your instruction's string name to a number value (usually its opCode given in the hardware as well).

e.g. else if(instructionBuffer == "NEWINSTRUCTION") {return [directiveName from Part A]}

C)If you plan on using the errorCheck function, add the implementation on the "errorCheck" function.

e.g. case NEWINSTRUCTION:
     if(desiredBuffer == "0xWRONG"){throw runtime_error("Error Message")}

D)Finally, you need to add your instruction into the transcribeIns (transcribe instruction) function which streams the binary into the output file.

The format of the case statement is:

case [instruction directive from part A]:
errorCheck();
outStream([instructionOpcode]);
outStream([reservedbyte, usually 0x00]);
outParameter(parameterBuffer or parameterCalc or 0x[6 byte value])
break;

 E)If the parameter of the instruction needs to be calculated add your implementation to calcParameter() function and use the parameterCalc variable provided in transcribeIns for outParameter.

e.g.
(In calcParameter()):
case [instruction directive from Part A]:
[calculation of parameter...]
return parameter;
break;

(In transcribeIns()):
case [instruction directive from part A]:
errorCheck();
outStream([instructionOpcode]);
outStream([reservedbyte, usually 0x00]);
outParameter(parameterCalc)
break;


2) CREATING A MACRO
Added macros should follow the syntax:
[macrostringname] [argument=][value] [argument=][value] [argument=][value] ...etc

However, fundamentally macros in the compiler can have any syntax past the [macro string name] since any argument/parameter after macro's name gets stored in a "macroArgument" deque. This is to offer greater freedom in the design of macros, however I must remind new developers that "with great power comes great responsibility", and that your macros should follow certain rules for other user's intuitiveness.

A)Add the Macro name and number of arguments in the macro directives before main. Assign a number not used for the macro name, and the number of arguments for the other (obviously).
e.g. #define NEWMACRO 2
e.g. #define NEWMACROARG 6 (6 string places allocated in deque)

B) Add your macro to the "macrotoInt" function.
This will assign your macro's string name to a number value. The hardware has no concept of a macro's value unlike instructions, so this can be any value.

e.g. else if(instructionBuffer == "NEWMACRO") {return [directiveName from Part A]}

C)Unlike creating instructions, you also have to add the number of parameters used on the macroSetup function.
e.g.	else if(instructionBuffer == "NEWMACRO")
	macroArgCount = NEWMACROARG;


D)If you plan on using the macroErrorCheck function, add the implementation on the "macroErrorCheck" function.
e.g. case NEWMACRO:
     if(macroArgument[2] == "[WRONG ARGUMENT]"){throw runtime_error("Error Message")}


E)Finally, you need to add your instruction into the transcribeMacro function. The process of transcribing a macro is divided in 4 sections inside the case statement:

case NEWMACRO:
1) Checking errors with macroErrorCheck:
e.g. macroErrorCheck/
2) Defining variables and calculating the MacroParameters.
**To convert the string arguments to integers, I use the atoi() function by turning the string into a c string. E.g. atoi(macroArgument[1].c_str()).

e.g.
int_64t parameter1 = atoi(macroArgument[1].c_str)
parameter1++;

3)Feeding the parameters into instructions using feedInstruction() and transcribeIns() functions
e.g.
 feedInstruction("[instruction]",["argument"],[parametervalue],"[identifier]");
 transcribeIns();
 feedInstruction("OR",[argument],parameter1, "");
 transcribeIns();

4) Remember to let the macro clear the deque! (i.e. don't exit before macroArgument.clear() happens at the end of the switch statement.)


