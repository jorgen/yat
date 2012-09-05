import QtQuick 2.0

Rectangle {
    id: screenItem

    property QtObject screen: null
    property Component lineFactory: Qt.createComponent("TerminalLine.qml")

    anchors.fill: parent

    color: "black"

    Component.onCompleted: resetScreenItems();

    function createLineItem(screenLine) {
        var line = lineFactory.createObject(screenItem,
                                            {
                                                "id": "terminalLine",
                                                "textLine": screenLine,
                                                "width": parent.width,
                                                "height": terminal.fontHeight,
                                                "x": 0,
                                                "y": screenLine.index * terminal.fontHeight,
                                            });
        screenLine.quickItem = line;
    }

    function resetScreenItems() {
        for (var i = 0; i < screen.height; i++) {
            var screen_line = screen.at(i);
            createLineItem(screen_line);
        }
    }

    Connections {
        id: connections

        target: terminal.screen

        onLinesInserted: {
            var last_index_before_insertion = screen.height -1 - count;

            for (var i = 0; i < count; i++) {
                var screen_line = screen.at(last_index_before_insertion +i);
                createLineItem(screen_line);
            }
        }

        onLineRemoved: {
            console.log("Line destroyed " + item);
            item.parent = null;
            item.visible = false;
            item.destroy();
        }


        onFlash: {
            flashAnimation.start()
        }

        onCursorPositionChanged: {
            cursor.x = x * screen.charWidth;
            cursor.y = y * screen.lineHeight
        }

        onReset: {
            resetScreenItems();
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

    HighlightArea {
        characterHeight: screen.lineHeight
        characterWidth: screen.charWidth

        start: screen.selectionAreaStart
        end: screen.selectionAreaEnd

        visible: screen.selectionEnabled
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

    MouseArea {
        id:mousArea

        property point drag_start

        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.MiddleButton
        onPressed: {
            if (mouse.button == Qt.LeftButton) {
                hoverEnabled = true;
                var character = Math.floor((mouse.x / screen.charWidth));
                var line = Math.floor(mouse.y / screen.lineHeight);
                var start = Qt.point(character,line);
                drag_start = start;
                screen.selectionAreaStart = start;
                screen.selectionAreaEnd = start;
            }
        }

        onPositionChanged: {
            var character = Math.floor(mouse.x / screen.charWidth);
            var line = Math.floor(mouse.y / screen.lineHeight);
            var current_pos = Qt.point(character,line);
            if (line < drag_start.y || (line === drag_start.y && character < drag_start.x)) {
                screen.selectionAreaStart = current_pos;
                screen.selectionAreaEnd = drag_start;
            }else {
                screen.selectionAreaEnd = current_pos;
                screen.selectionAreaStart = drag_start;
            }
        }

        onReleased: {
            if (mouse.button == Qt.LeftButton) {
                hoverEnabled = false;
                screen.sendSelectionToSelection();
            }
        }

        onClicked: {
            if (mouse.button == Qt.MiddleButton) {
                screen.pasteFromSelection();
            }
        }
        onDoubleClicked: {
            if (mouse.button == Qt.LeftButton) {
                var character = Math.floor(mouse.x / screen.charWidth);
                var line = Math.floor(mouse.y / screen.lineHeight);
                screen.doubleClicked(Qt.point(character,line));
            }
        }
    }
}
