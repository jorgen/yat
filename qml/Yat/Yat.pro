QT += core-private gui-private qml-private quick quick-private
TARGET = yat
TEMPLATE = lib
CONFIG += plugin
TARGETPATH = Yat
DESTDIR = ../../imports/$$TARGETPATH

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
              qmldir \
              Cursor.qml \
              Text.qml \
              Screen.qml \
              Selection.qml

isEmpty(YAT_INSTALL_QML):YAT_INSTALL_QML = $$[QT_INSTALL_QML]

target.path = $$YAT_INSTALL_QML/$$TARGETPATH
INSTALLS += target

other_files.files = $$OTHER_FILES
other_files.path = $$YAT_INSTALL_QML/$$TARGETPATH
INSTALLS += other_files

for(other_file, OTHER_FILES) {
    ARGUMENTS = $${PWD}$${QMAKE_DIR_SEP}$$other_file $$DESTDIR
    !isEmpty(QMAKE_POST_LINK):QMAKE_POST_LINK += &&
    QMAKE_POST_LINK += $$QMAKE_COPY $$replace(ARGUMENTS, /, $$QMAKE_DIR_SEP)
}
