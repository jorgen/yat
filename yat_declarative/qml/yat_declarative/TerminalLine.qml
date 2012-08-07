import QtQuick 2.0

Item{
    id: text_line
    property QtObject textLine: null

    Row {
        anchors.fill: parent
        Repeater {
            id: line_repeater
            model: lineModel
            Text {
                height: parent.height
                width: paintedWidth
                text: textSegment.text
                font: terminalItem.screen().font
                color: textSegment.forgroundColor()
                textFormat: Text.PlainText
            }
        }
    }

    ListModel {
        id: lineModel
    }

    onTextLineChanged: ;

    function resetModel() {
        lineModel.clear();
        for (var i = 0; i < textLine.size(); i++) {
            lineModel.append({
                                 "textSegment": textLine.at(i)
                             })
        }
    }

    Connections {
        target: textLine

        onNewTextSegment: {
            lineModel.insert(index, {
                                 "textSegment": textLine.at(index)
                             });
        }

        onTextSegmentRemoved: {
            lineModel.remove(index);
        }

        onReset: {
            resetModel();
        }
    }
}
