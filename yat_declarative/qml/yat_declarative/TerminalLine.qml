import QtQuick 2.0

Item{
    property QtObject textLine
//    property Font font

    Row {
        Repeater {
            model: textLine.size()
            Text {
                text: textLine.at(index).text()
                font: terminalItem.terminalScreen().font
            }
        }
    }
}
