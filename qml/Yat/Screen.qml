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

import QtQuick 2.10
import QtQuick.Controls 2.2
import Qt.labs.handlers 1.0
import Yat 1.0 as Yat

Yat.TerminalScreen {
    id: screenItem

    property font font
    property real fontWidth: fontMetricText.paintedWidth
    property real fontHeight: fontMetricText.paintedHeight

    font.family: screen.platformName != "cocoa" ? "monospace" : "menlo"
    anchors.fill: parent
    focus: true

    Component {
        id: textComponent
        Yat.Text {
        }
    }
    Component {
        id: cursorComponent
        Yat.Cursor {
        }
    }
    Shortcut {
        sequence: "Ctrl+Shift+C"
        onActivated: screen.selection.sendToClipboard()
    }
    Shortcut {
        sequence: "Ctrl+Shift+V"
        onActivated: screen.selection.pasteFromClipboard()
    }
    Shortcut {
        sequence: "Shift+Insert"
        onActivated: screen.selection.pasteFromSelection()
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            Qt.inputMethod.show();
        }
    }

    Keys.onPressed: {
        if (event.text === "?") {
            screen.printScreen()
        }
        screen.sendKey(event.text, event.key, event.modifiers);
    }

    Text {
        id: fontMetricText
        text: "B"
        font: parent.font
        visible: false
        textFormat: Text.PlainText
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: screen.defaultBackgroundColor
    }
    Flickable {
        id: flickable
        anchors.top: parent.top
        anchors.left: parent.left
        contentWidth: width
        contentHeight: textContainer.height
        interactive: true
        flickableDirection: Flickable.VerticalFlick
        contentY: ((screen.contentHeight - screen.height) * screenItem.fontHeight)
        ScrollBar.vertical: ScrollBar { }

        Item {
            id: textContainer
            width: parent.width
            height: screen.contentHeight * screenItem.fontHeight

            Selection {
                characterHeight: fontHeight
                characterWidth: fontWidth
                screenWidth: screenItem.width

                startX: screen.selection.startX
                startY: screen.selection.startY

                endX: screen.selection.endX
                endY: screen.selection.endY
                visible: screen.selection.enable
                z: 1
            }

            DragHandler {
                target: null
                acceptedDevices: PointerDevice.Mouse
                property int drag_start_x
                property int drag_start_y
                onActiveChanged: {
                    if (active) {
                        drag_start_x = Math.floor((point.pressPosition.x / fontWidth));
                        drag_start_y = Math.floor(point.pressPosition.y / fontHeight);
                        screen.selection.startX = drag_start_x;
                        screen.selection.startY = drag_start_y;
                        screen.selection.endX = drag_start_x;
                        screen.selection.endY = drag_start_y;
                    } else {
                        screen.selection.sendToSelection();
                    }
                }
                onPointChanged: if (active) {
                    var character = Math.floor(point.position.x / fontWidth);
                    var line = Math.floor(point.position.y / fontHeight);
                    var current_pos = Qt.point(character,line);
                    if (line < drag_start_y || (line === drag_start_y && character < drag_start_x)) {
                        screen.selection.startX = character;
                        screen.selection.startY = line;
                        screen.selection.endX = drag_start_x;
                        screen.selection.endY = drag_start_y;
                    } else {
                        screen.selection.startX = drag_start_x;
                        screen.selection.startY = drag_start_y;
                        screen.selection.endX = character;
                        screen.selection.endY = line;
                    }
                }
            }

            TapHandler {
                acceptedButtons: Qt.MiddleButton
                gesturePolicy: TapHandler.DragThreshold
                onTapped: screen.selection.pasteFromSelection()
            }

            TapHandler {
                gesturePolicy: TapHandler.DragThreshold
                onTapped: {
                    switch (tapCount) {
                    case 1:
                        screen.selection.startX = 0;
                        screen.selection.startY = 0;
                        screen.selection.endX = 0;
                        screen.selection.endY = 0;
                        break;
                    case 2:
                        var character = Math.floor(point.position.x / fontWidth);
                        var line = Math.floor(point.position.y / fontHeight);
                        screen.doubleClicked(character,line);
                        screen.selection.sendToSelection();
                        break;
                    }
                }
            }
        }

        Item {
            id: cursorContainer
            width: textContainer.width
            height: textContainer.height
        }

        onContentYChanged: {
            if (!atYEnd) {
                var top_line = Math.floor(Math.max(contentY,0) / screenItem.fontHeight);
                screen.ensureVisiblePages(top_line);
            }
        }
    }

    Connections {
        id: connections

        target: screen

        onFlash: {
            flashAnimation.start()
        }

        onReset: {
            resetScreenItems();
        }

        onTextCreated: {
            var textSegment = textComponent.createObject(screenItem,
                {
                    "parent" : textContainer,
                    "objectHandle" : text,
                    "font" : screenItem.font,
                    "fontWidth" : screenItem.fontWidth,
                    "fontHeight" : screenItem.fontHeight,
                });
        }

        onCursorCreated: {
            if (cursorComponent.status != Component.Ready) {
                console.log(cursorComponent.errorString());
                return;
            }
            var cursorVariable = cursorComponent.createObject(screenItem,
                {
                    "parent" : cursorContainer,
                    "objectHandle" : cursor,
                    "fontWidth" : screenItem.fontWidth,
                    "fontHeight" : screenItem.fontHeight,
                })
        }

        onRequestHeightChange: {
            terminalWindow.height = newHeight * screenItem.fontHeight;
            terminalWindow.contentItem.height = newHeight * screenItem.fontHeight;
        }

        onRequestWidthChange: {
            terminalWindow.width = newWidth * screenItem.fontWidth;
            terminalWindow.contentItem.width = newWidth * screenItem.fontWidth;
        }
    }

    onFontChanged: {
        setTerminalHeight();
        setTerminalWidth();
    }

    onWidthChanged: {
        setTerminalWidth();
    }
    onHeightChanged: {
        setTerminalHeight();
    }

    function setTerminalWidth() {
        if (fontWidth > 0) {
            var pty_width = Math.floor(width / fontWidth);
            flickable.width = pty_width * fontWidth;
            screen.width = pty_width;
        }
    }

    function setTerminalHeight() {
        if (fontHeight > 0) {
            var pty_height = Math.floor(height / fontHeight);
            flickable.height = pty_height * fontHeight;
            screen.height = pty_height;
        }
    }

    Rectangle {
        id: flash
        z: 1.2
        anchors.fill: parent
        color: "grey"
        opacity: 0
        SequentialAnimation {
            id: flashAnimation
            NumberAnimation {
                target: flash
                property: "opacity"
                to: 1
                duration: 75
            }
            NumberAnimation {
                target: flash
                property: "opacity"
                to: 0
                duration: 75
            }
        }
    }
}
