#include "clang-tidy/ClangTidyCheck.h"

namespace clang::tidy::extra {

class I18nStringCheck : public ClangTidyCheck {
  const std::vector<StringRef> AllowedFunctionsList;

public:
  I18nStringCheck(StringRef Name, ClangTidyContext *Context);
  void registerMatchers(ast_matchers::MatchFinder *Finder) override;
  void check(const ast_matchers::MatchFinder::MatchResult &Result) override;
  void storeOptions(ClangTidyOptions::OptionMap &Opts) override;
};

} // namespace clang::tidy::extra
