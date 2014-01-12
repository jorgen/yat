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
import QtQuick.Controls 1.1

Window {
    id: terminalWindow

    TabView {
        id: tabView
        anchors.fill: contentItem
        tabsVisible: count > 1
        focus: true

        Component.onCompleted: {
            add_terminal();
            set_current_terminal_active(0);
            terminalWindow.show();
        }

        function add_terminal()
        {
            var tab = tabView.addTab("", Qt.createComponent("TerminalScreen.qml"));
            tab.active = true;
            tab.title = Qt.binding(function() { return tab.item.screen.title; } );
            tab.item.aboutToBeDestroyed.connect(destroy_tab);
            tabView.currentIndex = tabView.count - 1;

        }
        function destroy_tab(screenItem) {
            if (tabView.count == 1) {
                Qt.quit();
                return;
            }
            for (var i = 0; i < tabView.count; i++) {
                if (tabView.getTab(i).item === screenItem) {
                    if (i === 0)
                        tabView.currentIndex = 1;
                    tabView.getTab(i).item.parent = null;
                    tabView.removeTab(i);
                    break;
                }
            }
        }

        function set_current_terminal_active(index) {
            terminalWindow.color = Qt.binding(function() { return tabView.getTab(index).item.screen.defaultBackgroundColor;})
            tabView.getTab(index).item.forceActiveFocus();
        }

        onCurrentIndexChanged: {
            set_current_terminal_active(tabView.currentIndex);
        }

        Action {
            id: newTabAction
            shortcut: "Ctrl+Shift+T"
            onTriggered: {
                tabView.add_terminal();
            }
        }
        Action {
            id: nextTabAction
            shortcut: "Ctrl+Shift+]"
            onTriggered: {
                tabView.currentIndex = (tabView.currentIndex + 1) % tabView.count;
            }
        }
        Action {
            id: previousTabAction
            shortcut: "Ctrl+Shift+["
            onTriggered: {
                if (tabView.currentIndex > 0) {
                    tabView.currentIndex--;
                } else {
                    tabView.currentIndex = tabView.count -1;
                }
            }
        }
    }

    width: 800
    height: 600

}
