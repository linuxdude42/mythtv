#
# Copyright (C) 2025 David Hampton
#
# See the file LICENSE_FSF for licensing information.
#

add_executable(test_mheg_base_classes test_mheg_base_classes.cpp test_mheg_base_classes.h
  ../../Actions.cpp
  ../../BaseActions.cpp
  ../../BaseClasses.cpp
  ../../Bitmap.cpp
  ../../DynamicLineArt.cpp
  ../../Engine.cpp
  ../../Groups.cpp
  ../../Ingredients.cpp
  ../../Link.cpp
  ../../ParseBinary.cpp
  ../../ParseNode.cpp
  ../../ParseText.cpp
  ../../Presentable.cpp
  ../../Programs.cpp
  ../../Root.cpp
  ../../Stream.cpp
  ../../Text.cpp
  ../../TokenGroup.cpp
  ../../Variables.cpp
  ../../Visible.cpp
)

target_include_directories(test_mheg_base_classes PRIVATE . ../..)

target_link_libraries(test_mheg_base_classes PUBLIC mythbase Qt${QT_VERSION_MAJOR}::Test)

add_test(NAME MhegBaseClasses COMMAND test_mheg_base_classes)
