#ifndef CFOINTERFACELIB_MU2ECOMPILER_HH
#define CFOINTERFACELIB_MU2ECOMPILER_HH

#include <deque>
#include <fstream>
#include <memory>
#include <sstream>
#include <stack>
#include <string>

namespace CFOLib {
class CFO_Source_File {
 public:
  explicit CFO_Source_File(std::string const& input) : buffer_(input), index_(0) {}
  explicit CFO_Source_File() : buffer_(""), index_(0) {}

  char peek() {
    if (index_ < buffer_.size()) return buffer_[index_];
    return '\n';
  }

  char get() {
    if (index_ >= buffer_.size()) return '\n';
    return buffer_[index_++];
  }

  bool eof() { return index_ == buffer_.size(); }

  void close() {
    index_ = 0;
    buffer_ = "";
  }

 private:
  std::string buffer_;
  size_t index_;
};

class CFO_Compiler {
 public:
  /*************
     DIRECTIVES
  ************/
  // Instruction name
  enum class CFO_INSTR : uint8_t {
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

  // Macro Names
  enum class CFO_MACRO : int {
    SLICE = 1,
    NON_MACRO = 0,
  };

  // Macro amount
  const int CFO_MACRO_SLICE_ARG_COUNT = 6;

 public:
  explicit CFO_Compiler(int clockSpeed = 40000000) : FPGAClock_(clockSpeed){};
  virtual ~CFO_Compiler() = default;

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
  int macroInsCount_;
  CFO_MACRO macroOpcode_;
  bool macroFlag_;

  CFO_Source_File inFile_;
  std::deque<char> output_;
};

}  // namespace CFOLib

#endif  // CFOINTERFACELIB_MU2ECOMPILER_HH