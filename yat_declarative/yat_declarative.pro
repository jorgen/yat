QT += gui quick
TARGET = yat

include(../backend/backend.pri)
SOURCES += main.cpp \
    terminal_item.cpp \
    register_qml_types.cpp \

QML_IMPORT_PATH =

OTHER_FILES += \
        qml/yat_declarative/main.qml \
        qml/yat_declarative/TerminalScreen.qml

RESOURCES += \
        qml_sources.qrc

HEADERS += \
    register_qml_types.h \
    terminal_item.h
