import QtQuick 2.0

Item{
    property QtObject textLine

    Row {
        Repeater {
            model: textLine === null ? 0 : textLine.size
            Text {
                text: textLine.at(index).text()
                font: terminalItem.terminalScreen().font
            }

        }
    }
}
