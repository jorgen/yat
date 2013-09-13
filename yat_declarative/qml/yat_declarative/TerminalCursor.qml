import QtQuick 2.0

import org.yat 1.0

ObjectDestructItem {
    id: cursor

    property real fontHeight
    property real fontWidth

    height: fontHeight
    width: fontWidth
    x: objectHandle.x * fontWidth
    y: objectHandle.y * fontHeight
    z: 1.1

    visible: objectHandle.visible

    Rectangle {
        anchors.fill: parent
        color: "grey"
    }
}

