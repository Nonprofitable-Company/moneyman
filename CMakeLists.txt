cmake_minimum_required(VERSION 3.20)
project(moneyman VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Treat warnings as errors
add_compile_options(-Wall -Wextra -Werror)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLCIPHER REQUIRED sqlcipher)

# === Source library (shared between app and tests) ===
add_library(moneyman_lib STATIC
    src/db/database.cpp
    src/db/database.h
    src/accounting/account.cpp
    src/accounting/account.h
    src/accounting/journal_entry.cpp
    src/accounting/journal_entry.h
    src/app/main_window.cpp
    src/app/main_window.h
)

target_include_directories(moneyman_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SQLCIPHER_INCLUDE_DIRS}
)

target_link_libraries(moneyman_lib PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    ${SQLCIPHER_LIBRARIES}
)

target_compile_options(moneyman_lib PUBLIC ${SQLCIPHER_CFLAGS_OTHER})

# === Main executable ===
add_executable(moneyman src/main.cpp)
target_link_libraries(moneyman PRIVATE moneyman_lib)

# === Tests ===
enable_testing()
find_package(Catch2 3 REQUIRED)

add_executable(test_accounting
    tests/test_accounting/test_account.cpp
    tests/test_accounting/test_journal_entry.cpp
)
target_link_libraries(test_accounting PRIVATE moneyman_lib Catch2::Catch2WithMain)
add_test(NAME test_accounting COMMAND test_accounting)

add_executable(test_db
    tests/test_db/test_database.cpp
)
target_link_libraries(test_db PRIVATE moneyman_lib Catch2::Catch2)
add_test(NAME test_db COMMAND test_db)
