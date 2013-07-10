DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

LIBS += -lutil -L/home/jlind/install/lib

CONFIG += c++11

MOC_DIR = .moc
OBJECTS_DIR = .obj

HEADERS += \
           $$PWD/yat_pty.h \
           $$PWD/controll_chars.h \
           $$PWD/parser.h \
           $$PWD/screen.h \
           $$PWD/color_palette.h \
           $$PWD/text_style.h \
           $$PWD/screen_data.h

SOURCES += \
           $$PWD/yat_pty.cpp \
           $$PWD/parser.cpp \
           $$PWD/screen.cpp \
           $$PWD/color_palette.cpp \
           $$PWD/screen_data.cpp
