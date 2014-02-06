
/**
 * PyOtherSide: Asynchronous Python 3 Bindings for Qt 5
 * Copyright (c) 2011, 2013, Thomas Perl <m@thp.io>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR
 * OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 **/

import QtQuick 2.0
import io.thp.pyotherside 1.0

Image {
    id: image
    width: 300
    height: 300

    property real zoomFactor: 1.2
    property real zoom: 2.5
    property real xCenter: -0.5
    property real yCenter: 0.0
    property real xRange: zoom
    property real yRange: zoom * height / width
    property real mLeft: xCenter - xRange / 2
    property real mRight: xCenter + xRange / 2
    property real mTop: yCenter - yRange / 2
    property real mBottom: yCenter + yRange / 2

    onXRangeChanged: updateImageTimer.start()
    onYRangeChanged: updateImageTimer.start()
    onXCenterChanged: updateImageTimer.start()
    onYCenterChanged: updateImageTimer.start()

    sourceSize {
        width: image.width / 4
        height: image.height / 4
    }

    MouseArea {
        anchors.fill: parent
        property real lastPosX
        property real lastPosY
        onPressed: {
            lastPosX = mouse.x;
            lastPosY = mouse.y;
        }

        onPositionChanged: {
            var diffX = (mouse.x - lastPosX);
            var diffY = (mouse.y - lastPosY);

            image.xCenter -= diffX / image.width * image.xRange;
            image.yCenter -= diffY / image.height * image.yRange;

            lastPosX = mouse.x;
            lastPosY = mouse.y;
        }
    }

    Column {
        spacing: 10
        anchors {
            right: parent.right
            top: parent.top
            margins: 10
        }

        Rectangle {
            width: 30; height: 30
            color: '#aaffffff'
            Text {
                anchors.centerIn: parent
                text: '-'
            }
            MouseArea {
                anchors.fill: parent
                onClicked: image.zoom *= image.zoomFactor
            }
        }

        Rectangle {
            width: 30; height: 30
            color: '#aaffffff'
            Text {
                anchors.centerIn: parent
                text: '+'
            }
            MouseArea {
                anchors.fill: parent
                onClicked: image.zoom /= image.zoomFactor
            }
        }
    }

    Python {
        id: py

        Component.onCompleted: {
            // Add the directory of this .qml file to the search path
            py.addImportPath(Qt.resolvedUrl('.'));

            py.importModule('mandelbrot', function () {
                // Do the first image update once the module is loaded
                updateImageTimer.start();
            });
        }

        onError: console.log('Python error: ' + traceback)
    }

    Timer {
        id: updateImageTimer
        interval: 100
        onTriggered: image.source = 'image://python/' + image.mLeft + '/' + image.mRight + '/' + image.mTop + '/' + image.mBottom
    }
}
