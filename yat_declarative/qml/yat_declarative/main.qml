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
    }

    Connections {
        target: terminalItem.terminalScreen();
        onScrollUp: {
            var screen = terminalItem.terminalScreen();
            if (terminalItem.terminalScreen().at(screen.geometry.height-1).size() > 0)
                print(terminalItem.terminalScreen().at(screen.geometry.height-1).at(0).text());
        }

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
            console.log("witdth changed " + width / fontWidth);
            terminalItem.width = width / fontWidth;
        }
    }

    function setTerminalHeight() {
        if (fontHeight > 0) {
            console.log("height changed " + height / fontHeight);
            terminalItem.height = height / fontHeight;
        }
    }

    Column {
        Repeater {
            model: terminalItem.terminalScreen().height
            TerminalLine {
                height: fontHeight
                width: terminal.width

                textLine: terminalItem.terminalScreen().at(index)

            }
        }
    }
}
