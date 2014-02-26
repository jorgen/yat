QT += core-private gui-private qml-private quick quick-private
TARGET = yat_qml_plugin
TEMPLATE = lib
CONFIG += plugin

include(../../backend/backend.pri)

SOURCES += \
          plugin/terminal_screen.cpp \
          plugin/object_destruct_item.cpp \
          plugin/mono_text.cpp \
          plugin/yat_extension_plugin.cpp \

HEADERS += \
          plugin/terminal_screen.h \
          plugin/object_destruct_item.h \
          plugin/mono_text.h \
          plugin/yat_extension_plugin.h \

OTHER_FILES = \
              Line.qml \
              Screen.qml \
              Text.qml \
              Selection.qml

