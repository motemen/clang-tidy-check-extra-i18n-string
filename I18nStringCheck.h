#include "clang-tidy/ClangTidyCheck.h"

#if LLVM_VERSION_MAJOR >= 15
#define STRING StringRef
#else
#define STRING std::string
#endif

namespace clang::tidy::extra {

struct AllowedFunctionEntry {
  STRING Name;
  std::vector<uint> ArgPositions;
};

class I18nStringCheck : public ClangTidyCheck {
  const std::vector<AllowedFunctionEntry> AllowedFunctionsList;
  const bool RemarkPassed;
  const std::string SkipPatternStr;
  llvm::Regex SkipPattern;

  static std::vector<AllowedFunctionEntry>
  parseAllowedFunctions(const STRING &AllowedFunctions);
  static std::string serializeAllowedFunctions(
      const std::vector<AllowedFunctionEntry> &AllowedFunctions);
  bool isAllowedFunctionCall(const CallExpr *CE, const StringLiteral *S) const;
  bool isAllowedMacroExpansion(const StringRef MacroName) const;

public:
  I18nStringCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;
};

} // namespace clang::tidy::extra
