TEMPLATE = app
TARGET = yat
DEPENDPATH += .
INCLUDEPATH += .

LIBS += -lutil

MOC_DIR = .moc
OBJECTS_DIR = .obj

HEADERS += \
           yat_pty.h \
           terminal_state.h \
    text_segment.h \
    controll_chars.h \
    tokenizer.h

SOURCES += \
           main.cpp \
           yat_pty.cpp \
           terminal_state.cpp \
    text_segment.cpp \
    tokenizer.cpp
