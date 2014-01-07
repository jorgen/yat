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
import QtQuick.Window 2.0
import QtQuick.Controls 1.0

Window {
    id: terminalWindow

    TabView {
        id: tabView
        anchors.fill: contentItem
        tabsVisible: count > 1
        focus: true

        Component.onCompleted: {
            var tab = tabView.addTab("some text", Qt.createComponent("TerminalScreen.qml"));
            tab.item.forceActiveFocus();
            terminalWindow.color = Qt.binding(function() { return tabView.getTab(tabView.currentIndex).item.screen.defaultBackgroundColor;});
            terminalWindow.show();
        }

        onCurrentIndexChanged: {
            background.color = Qt.binding(function() { return tabView.getTab(tabView.currentIndex).item.screen.defaultBackgroundColor;})
        }
    }

    width: 800
    height: 600

}
