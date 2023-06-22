cmake_minimum_required(VERSION 3.0)
project(ExtraI18nStringCheck VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED CONFIG)

add_library(ExtraI18nStringCheck MODULE I18nStringCheck.cpp)

set_target_properties(ExtraI18nStringCheck PROPERTIES OUTPUT_NAME ExtraI18nStringCheck_${LLVM_VERSION_MAJOR})

target_include_directories(ExtraI18nStringCheck PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(ExtraI18nStringCheck PRIVATE
    clangTidy
    clangTidyUtils
)

enable_testing()

set(TEST_CONFIG_OPTION "-config={\"CheckOptions\":[{\"key\":\"extra-i18n-string.RemarkPassed\",\"value\":true},{\"key\":\"extra-i18n-string.AllowedFunctions\",\"value\":\"_\;gettext\;prompt:1\"},{\"key\":\"extra-i18n-string.SkipPattern\",\"value\":\"^$|^%s$\"}]}")

add_test(
    NAME main
    COMMAND python ${CMAKE_SOURCE_DIR}/test/check_clang_tidy.py ${CMAKE_SOURCE_DIR}/test/i18n-string.cpp extra-i18n-string tmp --
        -load=${CMAKE_BINARY_DIR}/libExtraI18nStringCheck_${LLVM_VERSION_MAJOR}.so
        ${TEST_CONFIG_OPTION}
    WORKING_DIRECTORY ..
)
