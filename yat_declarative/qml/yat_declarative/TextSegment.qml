import QtQuick 2.0

Rectangle {
    id: textSegmentItem
    property QtObject textSegment
    property string text
    property color foregroundColor
    property color backgroundColor
    property font font

    height: textItem.paintedHeight
    width: textItem.paintedWidth
    y: 0
    color: backgroundColor

    Text {
        id: textItem
        text: textSegmentItem.text
        color: textSegmentItem.foregroundColor
        height: paintedHeight
        width: paintedWidth
        font: textSegmentItem.font
        textFormat: Text.PlainText
    }

    Connections {
        target: textSegment

        onTextChanged: {
            textItem.text = textSegment.text
        }

        onStyleChanged: {
            textSegmentItem.backgroundColor = textSegment.backgroundColor;
            textSegmentItem.foregroundColor = textSegment.foregroundColor;
        }

        onAboutToDestroy: {
            textSegmentItem.destroy();
        }

        onIndexChanged: {
            textSegmentItem.x = textSegment.index *  textSegment.screen.charWidth;
        }
    }
}
