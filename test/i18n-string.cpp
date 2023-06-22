// RUN: %check_clang_tidy %s misc-i18n-string %t --
// -load=./build/libExtraI18nStringCheck.so
// -config={"CheckOptions":{"extra-i18n-string.RemarkPassed":true,"extra-i18n-string.AllowedFunctions":"_;gettext;prompt:1","extra-i18n-string.SkipPattern":"^$|^%s$"}}

#define _(s) gettext((s))

void gettext(const char *s);
void printf(const char *fmt);
void prompt(const char *category, const char *msg);

void func() {
  auto s = "raw";
  // CHECK-MESSAGES: :[[@LINE-1]]:12: warning: string literal is not i18n-ed
  // [extra-i18n-string]
  _("non-raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:5: remark: this string literal passed
  // [extra-i18n-string]
  printf("raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:10: warning: string literal is not i18n-ed
  // [extra-i18n-string]
  gettext("non-raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:11: remark: this string literal passed
  // [extra-i18n-string]
  auto s2 = "raw"; // NOLINT
  prompt("category", "non-raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:22: warning: string literal is not i18n-ed
  // [extra-i18n-string]
  auto s3 = "";
  // CHECK-MESSAGES: :[[@LINE-1]]:13: remark: this string literal passed
  // [extra-i18n-string]
  printf("%s");
  // CHECK-MESSAGES: :[[@LINE-1]]:10: remark: this string literal passed
  // [extra-i18n-string]
}