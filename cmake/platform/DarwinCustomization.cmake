#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

if(NOT APPLE)
  return()
endif()

# MacPorts or Homebrew?
if(EXISTS /opt/local/share/macports)
  set(MACPORTS ON)
elseif(
  EXISTS /opt/local/bin/port
  OR EXISTS /usr/local/bin/brew
  OR EXISTS /opt/homebrew/bin/brew)
  set(HOMEBREW ON)
endif()

#
# Apple builds require Objective C++ compiler.
#
include(CheckLanguage)
check_language(OBJCXX)

# FFmpeg needs a little help in finding the mp3lame library.
if(MACPORTS)
  list(APPEND FF_PLATFORM_ARGS "--extra-ldflags=-L/opt/local/lib")
elseif(HOMEBREW)
  list(APPEND FF_PLATFORM_ARGS "--extra-ldflags=-L/opt/homebrew/lib")
endif()

# Qt6 builds need a little help finding the libraries.
set(_QT_BASE "/opt/local/libexec/${QT_PKG_NAME_LC}")
list(APPEND CMAKE_FRAMEWORK_PATH "${_QT_BASE}")
list(APPEND CMAKE_MODULE_PATH "${_QT_BASE}/lib/cmake")

# ~~~
# Using the Apple Clang compiler (version 13 tested) spits out this error:
#
#    ld: library not found for -lSystem
#
# According to the internet, the fix is to add the extra -L argument
# below.  That, however, produces a different set of warnings about
# missing include files.  The include file warnings are solved by the
# extra -I argument below.
# ~~~
if("${CMAKE_C_COMPILER}" MATCHES "Xcode")
  if(${CMAKE_C_COMPILER_VERSION} VERSION_GREATER_EQUAL 13)
    message(STATUS "Adding Apple Clang '-lSystem' hack")
    list(
      APPEND
      FF_PLATFORM_ARGS
      "--extra-ldflags=-L/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/lib"
    )
    list(
      APPEND
      FF_PLATFORM_ARGS
      "--extra-cflags=-I/Library/Developer/CommandLineTools/SDKs/MacOSX.sdk/usr/include"
    )
  endif()
endif()
