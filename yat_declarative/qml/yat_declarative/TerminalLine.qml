import QtQuick 2.0

Item{
    id: text_line
    property QtObject textLine: null

    Repeater {
        anchors.fill: parent
        model: lineModel
        TextSegment {
            textSegment: textLine.at(index)
        }
    }

    ListModel {
        id: lineModel
        Component.onCompleted: resetModel();
    }

    function resetModel() {
        lineModel.clear();
        for (var i = 0; i < textLine.size(); i++) {
            var textSegment = { "segment" : textLine.at(i)};
            lineModel.append(textSegment);
        }
    }

    Connections {
        target: textLine

        onNewTextSegment: {
            var textSegment = { "segment" : textLine.at(index)};
            lineModel.insert(index,textSegment);
        }

        onTextSegmentRemoved: {
            lineModel.remove(index);
        }

        onReset: {
            resetModel();
        }
    }
}

