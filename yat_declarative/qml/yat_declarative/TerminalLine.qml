import QtQuick 2.0

Item{
    id: text_line
    property QtObject textLine: null

    ListView {
        anchors.fill: parent
        model: lineModel
        delegate: TextSegment {
            textSegment: segment
        }
        orientation: ListView.Horizontal
    }

    ListModel {
        id: lineModel
        Component.onCompleted: resetModel();
    }

    function resetModel() {
        lineModel.clear();
        for (var i = 0; i < textLine.size(); i++) {
            lineModel.append({
                                 "segment": textLine.at(i)
                             })
        }
    }

    Connections {
        target: textLine

        onNewTextSegment: {
            lineModel.insert(index, {
                                 "segment": textLine.at(index)
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

