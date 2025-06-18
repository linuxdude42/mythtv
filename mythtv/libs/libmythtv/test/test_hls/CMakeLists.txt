#
# Copyright (C) 2025 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

add_executable(
  test_hls
  ../../HLS/httplivestreambuffer.cpp
  test_hls.cpp test_hls.h)

target_include_directories(test_hls PRIVATE . ../..)

target_link_libraries(test_hls PUBLIC mythtv Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME HLSM3U8 COMMAND test_hls)
