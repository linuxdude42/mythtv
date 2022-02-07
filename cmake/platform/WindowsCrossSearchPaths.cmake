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

# Look for precompiled libraries. Debian doesn't have many of these, but Fedora
# has a bunch.
set(MINGW_SYSROOT "/usr/${TOOLCHAIN_PREFIX}/sys-root/mingw")
list(PREPEND PKG_CONFIG_PATH "${MINGW_SYSROOT}/lib/pkgconfig")
list(APPEND CMAKE_FIND_ROOT_PATH "${MINGW_SYSROOT}/lib/pkgconfig")
