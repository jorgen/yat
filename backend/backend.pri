DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD

LIBS += -lutil -L/home/jlind/install/lib

MOC_DIR = .moc
OBJECTS_DIR = .obj

HEADERS += \
           $$PWD/yat_pty.h \
           $$PWD/terminal_state.h \
           $$PWD/text_segment.h \
           $$PWD/controll_chars.h \
           $$PWD/tokenizer.h \
           $$PWD/terminal_screen.h \
    ../backend/text_segment_line.h

SOURCES += \
           $$PWD/yat_pty.cpp \
           $$PWD/terminal_state.cpp \
           $$PWD/text_segment.cpp \
           $$PWD/tokenizer.cpp \
           $$PWD/terminal_screen.cpp \
    ../backend/text_segment_line.cpp
