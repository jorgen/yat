import QtQuick 2.0

Item{
    id: text_line
    property QtObject textLine

    Connections {
        target: textLine

        onIndexChanged: {
            y = textLine.index * height;
        }

    }
}

