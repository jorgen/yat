/*******************************************************************************
* Copyright (c) 2013 Jørgen Lind
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*******************************************************************************/

import QtQuick 2.0

Item {
    id: highlightArea

    property real characterWidth: 0
    property real characterHeight: 0
    property int screenWidth: width / characterWidth

    property point start
    property point end

    property color color: "grey"

    width: parent.width
    height: parent.height

    opacity: 0.8

    Rectangle {
        id: begginning_rectangle
        color: parent.color
        opacity: parent.opacity
        y:0
        height: characterHeight
    }

    Rectangle {
        id: middle_rectangle
        color: parent.color
        opacity: parent.opacity
        width: parent.width
        x: 0
        anchors.top: begginning_rectangle.bottom

    }

    Rectangle {
        id: end_rectangle
        color: parent.color
        opacity: parent.opacity
        x: 0
        height: characterHeight
        anchors.top: middle_rectangle.bottom
    }

    onCharacterWidthChanged: calculateRectangles();
    onCharacterHeightChanged: calculateRectangles();
    onScreenWidthChanged: calculateRectangles();

    onStartChanged: calculateRectangles();
    onEndChanged: calculateRectangles();

    function calculateRectangles() {
        highlightArea.y = start.y * characterHeight;
        begginning_rectangle.x = start.x * characterWidth;
        if (start.y === end.y) {
            middle_rectangle.visible = false;
            end_rectangle.visible = false
            begginning_rectangle.width = (end.x - start.x) * characterWidth;
        } else {
            begginning_rectangle.width = (screenWidth - start.x) * characterWidth;
            if (start.y === end.y - 1) {
                middle_rectangle.height = 0;
                middle_rectangle.visible = false;
            }else {
                middle_rectangle.visible = true;
                middle_rectangle.height = (end.y - start.y - 1) * characterHeight;
            }
            end_rectangle.visible = true;
            end_rectangle.width = end.x * characterWidth;
        }
    }

}
