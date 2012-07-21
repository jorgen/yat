import QtQuick 2.0

import com.yat 1.0

Rectangle {
    id:terminal
    width: 360
    height: 360

    property int fontWidth: dummyText.paintedWidth
    property int fontHeight: dummyText.paintedHeight

    TerminalItem {
        id: terminalItem
    }

    Text {
        id: dummyText
        text: "A"
        font: terminalItem.terminalScreen().font
        visible: false
    }

    onWidthChanged: {
        setTerminalWidth();
    }
    onHeightChanged: {
      setTerminalHeight();
    }

    onFontHeightChanged: {
        setTerminalHeight();
    }
    onFontWidthChanged: {
        setTerminalWidth();
    }

    function setTerminalWidth() {
        if (fontWidth > 0) {
            terminalItem.width = width / fontWidth;
        }
    }

    function setTerminalHeight() {
        if (fontHeight > 0) {
            terminalItem.height = height / fontHeight;
        }
    }

    Column {
        Repeater {
            model: terminalItem.terminalScreen().height
            TerminalLine {
                height: fontHeight
                width: terminal.width

                textLine: terminalItem.terminalScreen().at(terminalItem.terminalScreen().height - index -1)


            }
        }
    }
}
