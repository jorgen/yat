import QtQuick 2.0

Item {
    property QtObject terminalScreen: null

    anchors.fill: parent

    ListView {
        anchors.fill: parent
        model:  screenModel
        delegate: TerminalLine {
            id: lineDelegate
            height: terminal.fontHeight
            width: parent.width
            textLine: line
        }
        snapMode: ListView.SnapOneItem
    }

    Connections {
        id: connections

        target: terminalItem.screen

        onScrollUp: {
            screenModel.move(0,from_line - (count -1), count);
        }

        onScrollDown: {
            console.log("FIXME SCROLLDOWN TerminalScreen.qml");
        }

        onLinesInserted: {
            console.log("lines inserted");
            var model_size = screenModel.count;
            for (var i = 0; i < count; i++) {
                screenModel.append({
                                       "line": terminalItem.screen.at(model_size + i)
                                   });
            }

            resetModel();
        }

        onLinesRemoved: {
            for (var i = 0; i < count; i++) {
                screenModel.remove(0)
            }
        }
    }


    ListModel {
        id: screenModel
        Component.onCompleted:  resetModel();
    }


    function resetModel() {
        screenModel.clear();
        for (var i = 0; i < terminalItem.screen.height(); i++) {
            screenModel.append({
                                   "line": terminalItem.screen.at(i)
                               });
        }
    }

    Item {
        id: keyHandler
        focus: true
        Keys.onPressed: {
            terminalItem.screen.write(event.text)
            if (event.text === "?") {
                terminalItem.screen.printScreen()
            }
        }
    }
}
