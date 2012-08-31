import QtQuick 2.0

Item {
    anchors.fill: parent

    property real characterWidth: 0
    property real characterHeight: 0
    property int screenWidth: width / characterWidth

    property int startLine: 0
    property int startChar: 0
    property int endLine: 0
    property int endChar: 0

    property color color: "grey"
    opacity: 0.8

    Rectangle {
        id: begginning
        color: parent.color
        opacity: parent.opacity
    }

    Rectangle {
        id: middle
        color: parent.color
        opacity: parent.opacity
        width: parent.width
        x: 0

    }

    Rectangle {
        id: end
        color: parent.color
        opacity: parent.opacity
        x: 0
    }

    onCharacterWidthChanged: calculateRectangles();
    onCharacterHeightChanged: calculateRectangles();
    onScreenWidthChanged: calculateRectangles();

    onStartLineChanged: calculateRectangles();
    onStartCharChanged: calculateRectangles();
    onEndLineChanged: calculateRectangles();
    onEndCharChanged: calculateRectangles();

    onVisibleChanged: calculateRectangles();

    function calculateRectangles() {
        if (visible === false)
            return;

        begginning.x = startChar * characterWidth;
        begginning.y = startLine * characterHeight;
        begginning.height = characterHeight;
        if (startLine == endLine) {
            middle.visible = false;
            end.visible = false
            begginning.width = (endChar - startChar) * characterWidth;
        } else {
            begginning.width = (screenWidth - startChar) * characterWidth;
            if (startLine == endLine - 1) {
                middle.visible = false;
            }else {
                middle.visible = true;
                middle.height = (endLine - startLine - 1) * characterHeight;
                middle.y = (startLine + 1) * characterHeight;
            }
            end.visible = true;
            end.y = endLine * characterHeight;
            end.height = characterHeight;
            end.width = endChar * characterWidth;
        }
    }

}
