//===--- I18nStringCheck.cpp - clang-tidy ---------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

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

I18nStringCheck::I18nStringCheck(StringRef Name, ClangTidyContext *Context)
    : ClangTidyCheck(Name, Context),
      AllowedFunctionsList(utils::options::parseStringList(
          Options.get("AllowedFunctions", "_;"
                                          "N_;"
                                          "gettext"))) {}

void I18nStringCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(
      traverse(TK_IgnoreUnlessSpelledInSource,
               stringLiteral(optionally(hasParent(callExpr(
                                 callee(functionDecl().bind("function"))))))
                   .bind("stringLiteral")),
      this);
}

void I18nStringCheck::storeOptions(ClangTidyOptions::OptionMap &Opts) {
  Options.store(Opts, "AllowedFunctions",
                utils::options::serializeStringList(AllowedFunctionsList));
}

void I18nStringCheck::check(const MatchFinder::MatchResult &Result) {
  const auto *SL = Result.Nodes.getNodeAs<StringLiteral>("stringLiteral");

  if (const auto *F = Result.Nodes.getNodeAs<FunctionDecl>("function")) {
    if (std::find(AllowedFunctionsList.begin(), AllowedFunctionsList.end(),
                  F->getName()) != AllowedFunctionsList.end()) {
      diag(SL->getBeginLoc(), "string literal is i18n-ed",
           DiagnosticIDs::Remark);
      return;
    }
  }

  if (Result.SourceManager->isMacroBodyExpansion(SL->getBeginLoc())) {
    return;
  }

  if (Result.SourceManager->isMacroArgExpansion(SL->getBeginLoc())) {
    auto ExpandedMacroName =
        Lexer::getImmediateMacroName(SL->getBeginLoc(), *Result.SourceManager,
                                     Result.Context->getLangOpts());
    if (std::find(AllowedFunctionsList.begin(), AllowedFunctionsList.end(),
                  ExpandedMacroName) != AllowedFunctionsList.end()) {
      diag(SL->getBeginLoc(), "string literal is i18n-ed",
           DiagnosticIDs::Remark);
      return;
    }
  }

  diag(SL->getBeginLoc(), "string literal is not i18n-ed");
}

} // namespace clang::tidy::extra

namespace clang::tidy {

static ClangTidyModuleRegistry::Add<extra::ExtraModule>
    X("extra-module", "Adds extra lint checks.");

} // namespace clang::tidy
