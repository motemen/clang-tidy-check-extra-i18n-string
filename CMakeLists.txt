cmake_minimum_required(VERSION 3.0)
project(ExtraI18nStringCheck VERSION 0.1.0)

find_package(Clang REQUIRED CONFIG)

add_library(ExtraI18nStringCheck MODULE I18nStringCheck.cpp)

target_include_directories(ExtraI18nStringCheck PRIVATE ${CLANG_INCLUDE_DIRS})

target_link_libraries(ExtraI18nStringCheck PRIVATE
    clangTidy
    clangTidyUtils
)
