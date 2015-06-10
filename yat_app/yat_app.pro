TEMPLATE = app
TARGET = yat

QT += qml quick
SOURCES += main.cpp
RESOURCES += yat.qrc

ICON = resources/icon.png
macx: ICON = resources/icon.icns
win32: RC_FILE = resources/icon.rc

