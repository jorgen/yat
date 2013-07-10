import QtQuick 2.0

import org.yat 1.0

Item {
    id: screenItem

    property font font
    font.family: "Courier"

    TerminalItem {
        id: terminalItem

        onFlash: {
            flashAnimation.start()
        }

        onCursorPositionChanged: {
            cursor.x = x * m_font.charWidth;
            cursor.y = y * m_font.lineHeight
        }
    }

    TerminalEdit {
        id: primary
        anchors.fill: parent
        opacity: 0
        focus: false
    }

    TerminalEdit {
        id: secondary
        anchors.fill: parent
        opacity: 1
        focus: false
    }

    Component.onCompleted: {
        terminalItem.createScreen(primary.textDocument, secondary.textDocument);
    }

    Item {
        id: keyHandler
        focus: true
        Keys.onPressed: {
            terminalItem.sendKey(event.text, event.key, event.modifiers);
            if (event.text === "?") {
                terminalItem.printScreen()
            }
        }
    }

   Rectangle {
        id: cursor
        width: 10
        height: 10
        x: 0
        y: 0
        color: "grey"
    }

    Rectangle {
        id: flash
        anchors.fill: parent
        color: "grey"
        opacity: 0
        SequentialAnimation {
            id: flashAnimation
            NumberAnimation {
                target: flash
                property: "opacity"
                to: 1
                duration: 75
            }
            NumberAnimation {
                target: flash
                property: "opacity"
                to: 0
                duration: 75
            }
        }
    }
}
