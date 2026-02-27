#
# Copyright (C) 2026 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

add_executable(test_lirc test_lirc.cpp test_lirc.h)

target_include_directories(test_lirc PRIVATE . ../.. ../../devices)

target_compile_definitions(
  test_lirc PRIVATE TEST_SOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(test_lirc PUBLIC mythui Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME Settings COMMAND test_lirc)
