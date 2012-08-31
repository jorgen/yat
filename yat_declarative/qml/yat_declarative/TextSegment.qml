import QtQuick 2.0

Rectangle {
    property QtObject textSegment: null

    height: text.paintedHeight
    width: text.paintedWidth
    anchors.top: parent.top
    x: textSegment.index *  textSegment.screen.charWidth

    color: textSegment.backgroundColor

    Text {
        id: text
        text: textSegment.text
        color: textSegment.foregroundColor
        font: textSegment.screen.font
        height: paintedHeight
        width: paintedWidth
        textFormat: Text.PlainText
    }
}
