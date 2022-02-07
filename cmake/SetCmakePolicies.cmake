#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

# The setting of cmake minimum version 3.16 means that all policy numbers less
# than or equal to CMP0097 are set to NEW.
#
# https://cmake.org/cmake/help/latest/manual/cmake-policies.7.html#policies-introduced-by-cmake-3-16

# ~~~
# https://cmake.org/cmake/help/latest/policy/CMP0114.html
# ~~~
if(POLICY CMP0114)
  cmake_policy(SET CMP0114 NEW)
endif()

# ~~~
# https://cmake.org/cmake/help/latest/policy/CMP0135.html
# OLD: Use timestamps from downloaded file.
# NEW: Set timestamps of extracted content to current time.
#
# This messes up the projects that are based on autotools: fribidi,
# libass, and libsamplerate. Once cmake 3.24 becomes the minimum,
# uncomment the "DOWNLOAD_EXTRACT_TIMESTAMP ON" line in their External
# Project declaration, then this setting won't matter any more.
# ~~~
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 OLD)
endif()
