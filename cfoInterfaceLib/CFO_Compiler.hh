#ifndef CFOINTERFACELIB_MU2ECOMPILER_HH
#define CFOINTERFACELIB_MU2ECOMPILER_HH

#include <deque>
#include <fstream>
#include <memory>
#include <sstream>
#include <stack>
#include <string>

/// <summary>
/// The CFO Namespace
/// </summary>
namespace CFOLib {

/// <summary>
/// Represents a CFO source code file
/// </summary>
class CFO_Source_File
{
public:
	/// <summary>
	/// Construct a CFO_Source_File using file data
	/// </summary>
	/// <param name="input">CFO Source code</param>
	explicit CFO_Source_File(std::string const& input)
		: buffer_(input), index_(0) {}
	/// <summary>
	/// Default Constructor
	/// </summary>
	explicit CFO_Source_File()
		: buffer_(""), index_(0) {}

	/// <summary>
	/// Read the next character from the buffer, without incrementing the index
	/// </summary>
	/// <returns>Next character in the buffer</returns>
	char peek()
	{
		if (index_ < buffer_.size()) return buffer_[index_];
		return '\n';
	}

	/// <summary>
	/// Read the next character from the buffer, incrementing the index
	/// </summary>
	/// <returns>Next character in the buffer</returns>
	char get()
	{
		if (index_ >= buffer_.size()) return '\n';
		return buffer_[index_++];
	}

	/// <summary>
	/// Determine if the buffer is empty
	/// </summary>
	/// <returns>Whether the buffer is empty</returns>
	bool eof() { return index_ == buffer_.size(); }

	/// <summary>
	/// "Close" the stream, wiping the buffer
	/// </summary>
	void close()
	{
		index_ = 0;
		buffer_ = "";
	}

private:
	std::string buffer_;
	size_t index_;
};

/// <summary>
/// The CFO Compiler class
/// </summary>
class CFO_Compiler
{
public:
	/*************
     DIRECTIVES
  ************/
	/// <summary>
	/// Instruction name
	/// </summary>
	enum class CFO_INSTR : uint8_t
	{
		NOOP = 0,
		START = 1,
		DATA_REQUEST = 2,
		INC = 3,
		SET = 4,
		AND = 5,
		OR = 6,
		LOOP = 7,
		DO_LOOP = 8,
		REPEAT = 9,
		WAIT = 10,
		END = 11,
		INVALID = 0xFF,
	};

	/// <summary>
	/// Macro names
	/// </summary>
	enum class CFO_MACRO : int
	{
		SLICE = 1,
		NON_MACRO = 0,
	};

	/// <summary>
	/// Number of arguments for each macro
	/// </summary>
	const int CFO_MACRO_SLICE_ARG_COUNT = 6;

public:
	/// <summary>
	/// Instantiate the CFO Compiler class
	/// </summary>
	/// <param name="clockSpeed">Clock to use for calculating delays</param>
	explicit CFO_Compiler(int clockSpeed = 40000000)
		: FPGAClock_(clockSpeed){};
	/// <summary>
	/// Default destructor
	/// </summary>
	virtual ~CFO_Compiler() = default;

	/// <summary>
	/// Process an input file and create a byte block for sending to the CFO
	/// </summary>
	/// <param name="file">File to read</param>
	/// <returns>Array of bytes</returns>
	std::deque<char> processFile(CFO_Source_File const& file);

private:
	// For changing/adding new instructions check README file that comes with this source.
	/***********************
   ** Function Prototypes
   **********************/
	void readSpace();
	void newLine();
	void readLine();
	void transcribeIns();
	void transcribeMacro();
	void errorCheck(CFO_INSTR);
	int64_t calcParameter(CFO_INSTR);
	std::string readInstruction();
	void readMacro();
	void feedInstruction(std::string, std::string, int64_t, std::string);
	void macroSetup(std::string);
	CFO_INSTR parse_instruction(std::string);
	CFO_MACRO parse_macro(std::string);
	void outParameter(int64_t);
	bool isComment();
	bool isMacro();
	void macroErrorCheck(CFO_MACRO);

	std::stack<int64_t> loopStack_;
	std::deque<std::string> macroArgument_;
	std::string instructionBuffer_;
	std::string argumentBuffer_;
	std::string identifierBuffer_;
	int64_t parameterBuffer_;
	int64_t lineNumber_;
	int64_t FPGAClock_;
	int macroArgCount_;
	CFO_MACRO macroOpcode_;
	bool macroFlag_;

	CFO_Source_File inFile_;
	std::deque<char> output_;
};

}  // namespace CFOLib

#endif  // CFOINTERFACELIB_MU2ECOMPILER_HH