import QtQuick 2.0

Item {
    property QtObject terminalScreen

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

    ListModel {
        id: screenModel

        property QtObject terminalScreen

        onTerminalScreenChanged: resetModel();

    }
    Connections {
        target: terminalScreen

        onScrollUp: {
            screenModel.move(0,from_line,count);
        }

        onScrollDown: {
            screenModel.move(screenModel.count-1, from_line,count);
        }

//        onLinesChanged: {
//            console.log("should I do something\n");
//        }

        onLinesInserted: {
            resetModel();
        }

        onLinesRemoved: {
            resetModel();
        }
    }

    function resetModel() {
        screenModel.clear();
        console.log("TESTING")
        for (var i = 0; i < terminalScreen.height(); i++) {
            screenModel.append({
                                   "line": terminalScreen.at(i)
                               });
        }
    }

    Item {
        id: keyHandler
        focus: true
        Keys.onPressed: {
            terminalItem.state().write(event.text)
            if (event.text === "?") {
                terminalItem.screen().printScreen()
            }
        }
    }

    onTerminalScreenChanged: {
        screenModel.terminalScreen = terminalScreen
    }

}
