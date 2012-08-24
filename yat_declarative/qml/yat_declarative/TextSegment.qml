import QtQuick 2.0

Rectangle {
    property QtObject textSegment: null

    height: text.paintedHeight
    width: text.paintedWidth
    anchors.top: parent.top
    x: textSegment === null ? 0 : textSegment.index *  textSegment.screen.charWidth

    color: textSegment === null ? "white" : textSegment.backgroundColor

    scale: mouseArea.containsMouse ? 1.2 : 1.0

    Behavior on scale {
        NumberAnimation {
            duration: 100
        }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
    }

    Text {
        id: text
        text: textSegment === null ? "" : textSegment.text
        color: textSegment === null ? "black" : textSegment.foregroundColor
        font: textSegment === null ? "Sans" : textSegment.screen.font
        height: paintedHeight
        width: paintedWidth
        textFormat: Text.PlainText
    }
    Component.onCompleted: {
        textSegment.dispatchEvents();
    }
}
