import QtQuick 2.0

import org.yat 1.0

Rectangle {
    id:terminal
    width: 800
    height: 600
    color: terminalItem.screen().screenBackground();
    property int fontWidth: dummyText.paintedWidth
    property int fontHeight: dummyText.paintedHeight

    TerminalItem {
        id: terminalItem
    }

    Text {
        id: dummyText
        text: "A"
        font: terminalItem.screen().font
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
            terminalItem.screen().setWidth(width / fontWidth);
        }
    }

    function setTerminalHeight() {
        if (fontHeight > 0) {
            terminalItem.screen().setHeight(height / fontHeight);
        }
    }

    TerminalScreen {
        anchors.fill: parent
        terminalScreen: terminalItem.screen()
    }
}
