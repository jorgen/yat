import QtQuick 2.0
import QtQuick.Window 2.0

Window {
    id: terminalWindow
    TerminalScreen {
        id: terminal
        anchors.fill: parent
        Component.onCompleted: terminalWindow.visible = true
    }
    width: 800
    height: 600
    color: terminal.screen.defaultBackgroundColor
}
