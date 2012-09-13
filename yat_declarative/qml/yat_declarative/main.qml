import QtQuick 2.0

import org.yat 1.0

TerminalItem {
    id: terminal
    width: 800
    height: 600
    property font font

    property real fontWidth
    property real fontHeight

    property Component terminalScreenComponent: Qt.createComponent("TerminalScreen.qml")

    property Item terminalScreen: null

    onScreenChanged: {
        if (screen === null) {
            terminalScreen = null;
            return;
        }
        terminalScreen = terminalScreenComponent.createObject(terminal,
                                                              {
                                                                  "screen": screen,
                                                              });
    }



}
