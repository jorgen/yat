import QtQuick 2.0

Item{
    id: text_line
    property QtObject textLine
    property Component segmentFactory: Qt.createComponent("TextSegment.qml")

    function setTextItemForSegment(index) {
        var segment = textLine.at(index);
        var segmentItem = segment.quickItem;
        if (segmentItem === null) {
            segmentItem = segmentFactory.createObject(text_line, {
                                                          "textSegment": textLine.at(index)
                                                      });
            segment.quickItem = segmentItem;
        } else {
            segmentItem.textSegment = textLine.at(index);
        }
        segmentItem.x = segment.index *  segment.screen.charWidth;
        segmentItem.text = segment.text;
        segmentItem.foregroundColor = segment.foregroundColor;
        segmentItem.backgroundColor = segment.backgroundColor;
        segmentItem.font = segment.screen.font
        segmentItem.text.font = segment.screen.font
    }

    function resetModel() {
        if (textLine === null)
            return;
        for (var i = 0; i < textLine.size(); i++) {
            setTextItemForSegment(i);
        }
    }

    onTextLineChanged:  {
        if (segmentFactory && segmentFactory.status === Component.Ready) {
            console.log("RESETTING");
            resetModel();
        }
    }

    Connections {
        target: textLine

        onIndexChanged: {
            y = textLine.index * height;
        }

        onNewTextSegment: {
            setTextItemForSegment(index);
        }

        onReset: {
            resetModel();

        }
    }
}

