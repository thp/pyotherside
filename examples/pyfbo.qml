/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.0
import io.thp.pyotherside 1.5


Item {
    width: 400
    height: 400

    // The checkers background
    ShaderEffect {
        id: tileBackground
        anchors.fill: parent

        property real tileSize: 16
        property color color1: Qt.rgba(0.9, 0.9, 0.9, 1);
        property color color2: Qt.rgba(0.85, 0.85, 0.85, 1);

        property size pixelSize: Qt.size(width / tileSize, height / tileSize);

        fragmentShader:
            "
            uniform lowp vec4 color1;
            uniform lowp vec4 color2;
            uniform highp vec2 pixelSize;
            varying highp vec2 qt_TexCoord0;
            void main() {
                highp vec2 tc = sign(sin(3.14152 * qt_TexCoord0 * pixelSize));
                if (tc.x != tc.y)
                    gl_FragColor = color1;
                else
                    gl_FragColor = color2;
            }
            "
    }

    PyFBO {
        id: fbo
        anchors.fill: parent
        anchors.margins: 10
        property var t: 0

        SequentialAnimation on t {
            NumberAnimation { to: 1; duration: 2500; easing.type: Easing.InQuad }
            NumberAnimation { to: 0; duration: 2500; easing.type: Easing.OutQuad }
            loops: Animation.Infinite
            running: true
        }

        // The transform is just to show something interesting..
        transform: [
            Rotation { id: rotation; axis.x: 0; axis.z: 0; axis.y: 1; angle: 0; origin.x: fbo.width / 2; origin.y: fbo.height / 2; },
            Translate { id: txOut; x: -fbo.width / 2; y: -fbo.height / 2 },
            Scale { id: scale; },
            Translate { id: txIn; x: fbo.width / 2; y: fbo.height / 2 }
        ]

        onTChanged: {
            if (renderer) {
                py.call(py.getattr(renderer, 'set_t'), [t], update);
            }
        }
    }

    // Just to show something interesting
    SequentialAnimation {
        PauseAnimation { duration: 5000 }
        ParallelAnimation {
            NumberAnimation { target: scale; property: "xScale"; to: 0.6; duration: 1000; easing.type: Easing.InOutBack }
            NumberAnimation { target: scale; property: "yScale"; to: 0.6; duration: 1000; easing.type: Easing.InOutBack }
        }
        NumberAnimation { target: rotation; property: "angle"; to: 80; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: rotation; property: "angle"; to: -80; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: rotation; property: "angle"; to: 0; duration: 1000; easing.type: Easing.InOutCubic }
        NumberAnimation { target: fbo; property: "opacity"; to: 0.5; duration: 1000; easing.type: Easing.InOutCubic }
        PauseAnimation { duration: 1000 }
        NumberAnimation { target: fbo; property: "opacity"; to: 0.8; duration: 1000; easing.type: Easing.InOutCubic }
        ParallelAnimation {
            NumberAnimation { target: scale; property: "xScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
            NumberAnimation { target: scale; property: "yScale"; to: 1; duration: 1000; easing.type: Easing.InOutBack }
        }
        running: true
        loops: Animation.Infinite
    }

    Rectangle {
        id: labelFrame
        anchors.margins: -10
        radius: 5
        color: "white"
        border.color: "black"
        opacity: 0.8
        anchors.fill: label
    }

    Text {
        id: label
        anchors.bottom: fbo.bottom
        anchors.left: fbo.left
        anchors.right: fbo.right
        anchors.margins: 20
        wrapMode: Text.WordWrap
        text: "The squircle is an FBO, rendered by the application on the scene graph rendering thread. The FBO is managed and displayed using a PyFBO item."
    }

    Python {
        id: py

        Component.onCompleted: {
            addImportPath(Qt.resolvedUrl('.'));
            importModule('renderer', function () {
                call('renderer.Renderer', [], function (renderer) {
                    fbo.renderer = renderer;
                });
            });
        }

        onError: console.log(traceback);
    }

}
