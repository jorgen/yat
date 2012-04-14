TEMPLATE = app
TARGET = yat
DEPENDPATH += .
INCLUDEPATH += .

LIBS += -lutil

MOC_DIR = .moc
OBJECTS_DIR = .obj

HEADERS += \
           yat_pty.h

SOURCES += \
           main.cpp \
           yat_pty.cpp
