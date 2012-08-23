import QtQuick 2.0

import org.yat 1.0

Rectangle {
    id:terminal
    width: 800
    height: 600
    color: terminalItem.screen.screenBackground();
    property font font: terminalItem.screen.font;

    property real fontWidth: 0;
    property real fontHeight: 0;

    TerminalItem {
        id: terminalItem
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
            var pty_width = width / fontWidth;
            terminalItem.screen.setWidth(pty_width);
        } else {
            terminalItem.screen.setWidth(10);
        }
    }

    function setTerminalHeight() {
        if (fontHeight > 0) {
            var pty_height = height / fontHeight;
            terminalItem.screen.setHeight(pty_height);
        }
    }

    TerminalScreen {
        anchors.fill: parent
        terminalScreen: terminalItem.screen
    }

    onFontChanged: {
        fontWidth = terminalItem.screen.characterWidth();
        fontHeight = terminalItem.screen.lineHeight();
    }

}
