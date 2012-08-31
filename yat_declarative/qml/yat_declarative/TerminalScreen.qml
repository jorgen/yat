import QtQuick 2.0

Rectangle {
    property QtObject screen: null

    anchors.fill: parent

    color: "black"

    Repeater {
        anchors.fill: parent
        model:screenModel
        TerminalLine {
            id: terminalLine
            height: terminal.fontHeight
            width: parent.width
            textLine: line
            x: 0
            y: line.index * terminal.fontHeight
            Connections {
                id: terminalLineConnections
                target: textLine

                onIndexChanged: {
                    terminalLine.y = terminalLine.textLine.index * terminalLine.height
                }
            }
        }
    }

    Connections {
        id: connections

        target: terminal.screen

        onMoveLines: {
            screenModel.move(from_line, to_line, count);
        }

        onLinesInserted: {
            var model_size = screenModel.count;
            for (var i = 0; i < count; i++) {
                screenModel.append({
                                       "line": terminal.screen.at(model_size + i)
                                   });
            }
        }

        onLinesRemoved: {
            for (var i = 0; i < count; i++) {
                screenModel.remove(0)
            }
        }

        onFlash: {
            flashAnimation.start()
        }

        onCursorPositionChanged: {
            cursor.x = x * screen.charWidth;
            cursor.y = y * screen.lineHeight
        }

        onReset: resetModel();
    }


    ListModel {
        id: screenModel
        Component.onCompleted:  resetModel();
    }


    function resetModel() {
        screenModel.clear();
        for (var i = 0; i < terminal.screen.height(); i++) {
            screenModel.append({
                                   "line": terminal.screen.at(i)
                               });
        }
    }

    Item {
        id: keyHandler
        focus: true
        Keys.onPressed: {
            terminal.screen.sendKey(event.text, event.key, event.modifiers);
            if (event.text === "?") {
                terminal.screen.printScreen()
            }
        }
    }

    Rectangle {
        id: cursor
        width: screen.charWidth
        height: screen.lineHeight
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
