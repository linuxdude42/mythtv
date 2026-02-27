include ( ../../../../settings.pro )

QT += network testlib widgets
using_opengl: QT += opengl

TEMPLATE = app
TARGET = test_lirc
INCLUDEPATH += ../../devices

LIBS += -L../.. -lmythui-$$LIBVERSION
LIBS += -L../../../libmythbase -lmythbase-$$LIBVERSION

QMAKE_LFLAGS += -Wl,$$_RPATH_$(PWD)/../..
QMAKE_LFLAGS += -Wl,$$_RPATH_$(PWD)/../../../libmythbase

DEFINES += TEST_SOURCE_DIR='\'"$${PWD}"'\'

# Input
HEADERS += test_lirc.h
SOURCES += test_lirc.cpp

QMAKE_CLEAN += $(TARGET) $(TARGETA) $(TARGETD) $(TARGET0) $(TARGET1) $(TARGET2)
QMAKE_CLEAN += ; ( cd $(OBJECTS_DIR) && rm -f *.gcov *.gcda *.gcno )

LIBS += $$EXTRA_LIBS $$LATE_LIBS

# Fix runtime linking on Ubuntu 17.10.
linux:QMAKE_LFLAGS += -Wl,--disable-new-dtags
