import QtQuick 2.0

Flickable {
    id: flick
    property var textDocument : edit.textDocument

    anchors.fill: parent
    contentWidth: edit.paintedWidth
    contentHeight: edit.paintedHeight
    clip: true

    TextEdit {
       id: edit
       anchors.fill: parent

       focus: false
       activeFocusOnPress: false
       wrapMode: TextEdit.NoWrap
       textFormat: TextEdit.PlainText
       MouseArea {
        anchors.fill: parent
        onClicked: flick.ensureVisible(edit.cursorRectangle)
       }
    }

    function ensureVisible(r)
    {
        if (contentX >= r.x)
            contentX = r.x;
        else if (contentX+width <= r.x+r.width)
            contentX = r.x+r.width-width;
        if (contentY >= r.y)
            contentY = r.y;
        else if (contentY+height <= r.y+r.height)
            contentY = r.y+r.height-height;
    }

}
