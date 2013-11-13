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

    ShaderEffect {
        anchors.fill: parent

        property variant source: fragmentSource

        fragmentShader:
            "uniform lowp float qt_Opacity;" +
            "uniform sampler2D source;" +
            "varying highp vec2 qt_TexCoord0;" +

            "void main() {" +
            "   lowp vec4 color = texture2D(source, qt_TexCoord0 ) * qt_Opacity;" +
            "   gl_FragColor = vec4(1.0 - color.r, 1.0 - color.g, 1.0 - color.b, color.a);" +
            "}"

        ShaderEffectSource {
            id: fragmentSource
            sourceItem: background
            live: true

            sourceRect: Qt.rect(cursor.x,cursor.y,cursor.width,cursor.height);
        }
    }
}

