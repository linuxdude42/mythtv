#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

# Make sure that pkg-config only looks for host system libraries, and doesn't
# find any build system libraries.
if(${CMAKE_VERSION} VERSION_LESS 3.22)
  set(PKG_CONFIG "pkg-config" "--env-only")
else()
  set(PKG_CONFIG_ARGN "--env-only")
endif()

# The pkgconfig files we generated are in CMAKE_INSTALL_PREFIX.
