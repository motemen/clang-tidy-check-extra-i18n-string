cmake_minimum_required(VERSION 3.0)
project(ExtraI18nStringCheck VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Clang REQUIRED CONFIG)

add_library(ExtraI18nStringCheck MODULE I18nStringCheck.cpp)

set_target_properties(ExtraI18nStringCheck PROPERTIES OUTPUT_NAME ExtraI18nStringCheck_${LLVM_VERSION})

target_include_directories(ExtraI18nStringCheck PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(ExtraI18nStringCheck PRIVATE
    clangTidy
    clangTidyUtils
)

enable_testing()

add_test(
    NAME main
    COMMAND python test/check_clang_tidy.py test/i18n-string.cpp extra-i18n-string tmp -- --load=./build/libExtraI18nStringCheck.so
    WORKING_DIRECTORY ..
)
