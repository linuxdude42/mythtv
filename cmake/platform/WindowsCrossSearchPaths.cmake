#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

# Look for precompiled libraries. Debian doesn't have many of these, but Fedora
# has a bunch.
if(HOST_LSB_RELEASE_ID MATCHES "fedora")
  set(MINGW_SYSROOT "/usr/${TOOLCHAIN_PREFIX}/sys-root/mingw")
  list(PREPEND PKG_CONFIG_PATH "${MINGW_SYSROOT}/lib/pkgconfig")
  list(APPEND CMAKE_FIND_ROOT_PATH "${MINGW_SYSROOT}/lib/pkgconfig")
  if(EXISTS ${MINGW_SYSROOT}/lib/libz.dll.a)
    set(ZLIB_ROOT ${MINGW_SYSROOT})
  else()
    set(ZLIB_ROOT ${LIBS_INSTALL_PREFIX})
  endif()
elseif(HOST_LSB_RELEASE_ID MATCHES "debian" OR HOST_LSB_RELEASE_ID MATCHES
                                               "ubuntu")

  # Choices are 14-win32 and 14-posix. Using 14-win32, all the
  # libraries are built but the compilation fails in MythTV in
  # mediamonitor-windows.cpp because of missing GTHREAD COND support.
  # Using 14-posix to provide GTHREAD COND support, the mesa library
  # can't be compiled because of missing pthread symbols.
  set(MINGW_EXTRA_LIBRARY_PATH "-L/usr/lib/gcc/${TOOLCHAIN_PREFIX}/14-posix")
  #set(MINGW_EXTRA_LIBRARY_PATH "-L/usr/lib/gcc/${TOOLCHAIN_PREFIX}/14-win32")
endif()
