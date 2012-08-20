import QtQuick 2.0

Item{
    id: text_line
    property QtObject textLine: null

    ListView {
        anchors.fill: parent
        model: lineModel
        delegate: Rectangle {
            height: text.paintedHeight
            width: text.paintedWidth
            color: textSegment.backgroundColor
            Text {
                id: text
                height: paintedHeight
                width: paintedWidth
                text: textSegment === undefined? "" : textSegment.text
                font.family: "courier"
                font.pointSize: 10
                color: textSegment.forgroundColor
                font.bold: true
                textFormat: Text.PlainText
//                renderType: Text.NativeRendering
            }
        }
        orientation: ListView.Horizontal
    }

    ListModel {
        id: lineModel
    }

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

