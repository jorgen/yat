DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

LIBS += -lutil -L/home/jlind/install/lib

MOC_DIR = .moc
OBJECTS_DIR = .obj

HEADERS += \
           $$PWD/yat_pty.h \
           $$PWD/text_segment.h \
           $$PWD/controll_chars.h \
           $$PWD/parser.h \
           $$PWD/terminal_screen.h \
           $$PWD/text_segment_line.h \
           $$PWD/color_palette.h \
           $$PWD/text_style.h

SOURCES += \
           $$PWD/yat_pty.cpp \
           $$PWD/text_segment.cpp \
           $$PWD/parser.cpp \
           $$PWD/terminal_screen.cpp \
           $$PWD/text_segment_line.cpp \
           $$PWD/color_palette.cpp \
           $$PWD/text_style.cpp
