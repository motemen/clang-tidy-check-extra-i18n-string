// RUN: %check_clang_tidy %s misc-i18n-string %t --
// --load=./build/libExtraI18nStringCheck.so

#define _(s) gettext((s))

void gettext(const char *s);
void printf(const char *fmt);

void func() {
  "raw";
  // CHECK-MESSAGES: :[[@LINE-1]]:3: warning: string literal is not i18n-ed
  // [extra-i18n-string]
  _("non-raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:5: remark: this string literal passed
  // [extra-i18n-string]
  printf("raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:10: warning: string literal is not i18n-ed
  // [extra-i18n-string]
  gettext("non-raw");
  // CHECK-MESSAGES: :[[@LINE-1]]:5: remark: this string literal passed
  // [extra-i18n-string]
}
