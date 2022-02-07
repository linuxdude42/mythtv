#
# Copyright (C) 2022-2023 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

#
# Set default build prefix.
#
set(MYTH_DEFAULT_LIBS_PREFIX
    "${PROJECT_BINARY_DIR}/tmp_libinstall_${CMAKE_ANDROID_ARCH_ABI}_${QT_PKG_NAME_LC}"
)
set(MYTH_DEFAULT_PREFIX
    "${PROJECT_BINARY_DIR}/tmp_install_${CMAKE_ANDROID_ARCH_ABI}_${QT_PKG_NAME_LC}"
)
