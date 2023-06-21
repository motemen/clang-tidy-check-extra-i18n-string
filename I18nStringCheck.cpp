#include "I18nStringCheck.h"
#include "clang-tidy/ClangTidyModule.h"
#include "clang-tidy/ClangTidyModuleRegistry.h"
#include "clang-tidy/utils/OptionsUtils.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/Lex/Lexer.h"

using namespace clang::ast_matchers;

namespace clang::tidy::extra {

class ExtraModule : public ClangTidyModule {
public:
  void addCheckFactories(ClangTidyCheckFactories &CheckFactories) override {
    CheckFactories.registerCheck<I18nStringCheck>("extra-i18n-string");
  }
};

std::vector<AllowedFunctionEntry>
I18nStringCheck::parseAllowedFunctions(const STRING &AllowedFunctions) {
  std::vector<AllowedFunctionEntry> Result;

  std::vector<STRING> AllowedFunctionsListStr =
      utils::options::parseStringList(AllowedFunctions);

  for (const auto &AllowedFunction : AllowedFunctionsListStr) {
    AllowedFunctionEntry AllowedFunctionEntry;

    STRING FuncName;
    STRING ArgPositionStr;

    if (std::tie(FuncName, ArgPositionStr) =
            StringRef(AllowedFunction).split(":");
        !ArgPositionStr.empty()) {
      AllowedFunctionEntry.Name = FuncName;

      STRING ArgPosition;
      while (std::tie(ArgPosition, ArgPositionStr) =
                 StringRef(ArgPositionStr).split(","),
             !ArgPosition.empty()) {

        AllowedFunctionEntry.ArgPositions.push_back(
            std::stoi(std::string(ArgPosition)) - 1);
      }
    } else {
      AllowedFunctionEntry.Name = AllowedFunction;
    }

    Result.push_back(AllowedFunctionEntry);
  }

  return Result;
}

std::string I18nStringCheck::serializeAllowedFunctions(
    const std::vector<AllowedFunctionEntry> &AllowedFunctions) {
  std::string Result;

  for (auto Entry = AllowedFunctions.begin(); Entry != AllowedFunctions.end();
       ++Entry) {
    if (Entry != AllowedFunctions.begin()) {
      Result += ";";
    }

    Result += Entry->Name;

    for (auto Pos = Entry->ArgPositions.begin();
         Pos != Entry->ArgPositions.end(); ++Pos) {
      if (Pos != Entry->ArgPositions.begin()) {
        Result += ",";
      }
      Result += ":" + std::to_string(*Pos + 1);
    }
  }

  return Result;
}

bool I18nStringCheck::isAllowedFunctionCall(const CallExpr *CE,
                                            const StringLiteral *S) const {
  const auto *F = CE->getDirectCallee();
  if (!F) {
    return false;
  }

  for (const auto &AllowedFunction : AllowedFunctionsList) {
    if (F->getName() == AllowedFunction.Name) {
      if (AllowedFunction.ArgPositions.empty()) {
        return true;
      }

      for (const auto &ArgPosition : AllowedFunction.ArgPositions) {
        if (CE->getNumArgs() > ArgPosition &&
            CE->getArg(ArgPosition)->IgnoreParenCasts()->IgnoreCasts() == S) {
          return true;
        }
      }
    }
  }

  return false;
}

bool I18nStringCheck::isAllowedMacroExpansion(const StringRef MacroName) const {
  for (const auto &AllowedFunction : AllowedFunctionsList) {
    if (MacroName == AllowedFunction.Name) {
      return true;
    }
  }

  return false;
}

I18nStringCheck::I18nStringCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      RemarkPassed(Options.get("RemarkPassed", false)),
      AllowedFunctionsList(
          parseAllowedFunctions(Options.get("AllowedFunctions", "_;"
                                                                "N_;"
                                                                "gettext"))) {}

void I18nStringCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      traverse(
          TK_IgnoreUnlessSpelledInSource,
          stringLiteral(optionally(hasParent(
                            callExpr(callee(functionDecl().bind("function")))
                                .bind("callExpr"))))
              .bind("stringLiteral")),
      this);
}

void I18nStringCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "RemarkPassed", RemarkPassed);
  Options.store(Opts, "AllowedFunctions",
                serializeAllowedFunctions(AllowedFunctionsList));
}

void I18nStringCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *S = Result.Nodes.getNodeAs<StringLiteral>("stringLiteral");

  if (const auto *CE = Result.Nodes.getNodeAs<CallExpr>("callExpr")) {
    if (isAllowedFunctionCall(CE, S)) {
      if (RemarkPassed) {
        diag(S->getBeginLoc(), "this string literal passed",
             DiagnosticIDs::Remark);
      }
      return;
    }
  }

  if (Result.SourceManager->isMacroBodyExpansion(S->getBeginLoc())) {
    return;
  }

  if (Result.SourceManager->isMacroArgExpansion(S->getBeginLoc())) {
    auto ExpandedMacroName = Lexer::getImmediateMacroName(
        S->getBeginLoc(), *Result.SourceManager, Result.Context->getLangOpts());
    if (isAllowedMacroExpansion(ExpandedMacroName)) {
      if (RemarkPassed) {
        diag(S->getBeginLoc(), "this string literal passed",
             DiagnosticIDs::Remark);
      }
      return;
    }
  }

  diag(S->getBeginLoc(), "string literal is not i18n-ed");
}

} // namespace clang::tidy::extra

namespace clang::tidy {

static ClangTidyModuleRegistry::Add<extra::ExtraModule>
    X("extra-module", "Adds extra lint checks.");

} // namespace clang::tidy
