import QtQuick 2.0

Rectangle {
    id: textSegmentItem
    property QtObject textSegment
    property string text
    property color foregroundColor
    property color backgroundColor
    property font font

    y: 0
    width: textItem.paintedWidth
    height: textItem.paintedHeight

    color: backgroundColor

    Component.onCompleted: {
        backgroundColor = randomBg();
    }
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
            textSegmentItem.text = textSegment.text
        }

        onStyleChanged: {
            //textSegmentItem.backgroundColor = textSegment.backgroundColor;
            textSegmentItem.foregroundColor = textSegment.foregroundColor;
        }

        onIndexChanged: {
            textSegmentItem.x = textSegment.index *  textSegment.screen.charWidth;
        }

        onVisibleChanged: {
            textSegmentItem.visible = textSegment.visible;
        }
    }

    function randomBg()
    {
        var hex1=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
            var hex2=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
            var hex3=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
            var hex4=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
            var hex5=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
            var hex6=new Array("0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F")
            var bg="#"+hex1[Math.floor(Math.random()*hex1.length)]+hex2[Math.floor(Math.random()*hex2.length)]+hex3[Math.floor(Math.random()*hex3.length)]+hex4[Math.floor(Math.random()*hex4.length)]+hex5[Math.floor(Math.random()*hex5.length)]+hex6[Math.floor(Math.random()*hex6.length)]
            return bg
    }
}
