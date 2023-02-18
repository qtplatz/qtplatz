// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import StudioTheme 1.0 as StudioTheme

Rectangle {
    id: root

    property var pointA: Qt.vector2d()
    property var pointB: Qt.vector2d()

    property bool linked: false

    property var middle: {
        var ab = root.pointB.minus(root.pointA) // B - A
        return root.pointA.plus(ab.normalized().times(ab.length() * 0.5))
    }

    property var position: {
        // Calculate the middle point between A and B
        var ab = root.pointB.minus(root.pointA) // B - A
        var midAB = root.pointA.plus(ab.normalized().times(ab.length() * 0.5))
        var perpendicularAB = Qt.vector2d(ab.y, -ab.x)
        return midAB.plus(perpendicularAB.normalized().times(8.0 * StudioTheme.Values.scaleFactor))
    }

    color: "transparent"
    border.color: "transparent"

    x: root.position.x - (StudioTheme.Values.height * 0.5)
    y: root.position.y - (StudioTheme.Values.height * 0.5)

    implicitWidth: StudioTheme.Values.height
    implicitHeight: StudioTheme.Values.height

    transformOrigin: Item.Center

    T.Label {
        id: icon
        anchors.fill: parent
        text: root.linked ? StudioTheme.Constants.linked
                          : StudioTheme.Constants.unLinked
        visible: true
        color: "grey"
        font.family: StudioTheme.Constants.iconFont.family
        font.pixelSize: StudioTheme.Values.myIconFontSize
        verticalAlignment: Text.AlignVCenter
        horizontalAlignment: Text.AlignHCenter
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        anchors.margins: 4.0 * StudioTheme.Values.scaleFactor
        hoverEnabled: true
        onPressed: root.linked = !root.linked
    }

    states: [
        State {
            name: "default"
            when: !mouseArea.containsMouse
            PropertyChanges {
                target: icon
                color: "grey"//StudioTheme.Values.themeControlBackground
            }
        },
        State {
            name: "hover"
            when: mouseArea.containsMouse
            PropertyChanges {
                target: icon
                color: "white"//StudioTheme.Values.themeHoverHighlight
            }
        }
    ]
}
