/****************************************************************************
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
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
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
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
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 1.0

/*
    This is exactly the same as states.qml, except that we have appended
    a set of transitions to apply animations when the item changes
    between each state.
*/

Rectangle {
    id: page
    width: 340; height: 280
    color: "#343434"

    Image {
        id: userIcon
        x: scanTypeTofRect.x; y: scanTypeTofRect.y
        source: "images/scantype.png"
    }

    Rectangle {
        id: scanTypeTofRect

        anchors { left: parent.left; top: parent.top; leftMargin: 10; topMargin: 20 }
        width: 46; height: 54
        color: "Transparent"; border.color: "Gray"; radius: 6

        // Clicking in here sets the state to the default state, returning the image to
        // its initial position
        MouseArea { anchors.fill: parent; onClicked: page.state = '' }
    }

    Rectangle {
        id: scanTypeProportionalRect

        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: 20 }
        width: 46; height: 54
        color: "Transparent"; border.color: "Gray"; radius: 6
        anchors.verticalCenterOffset: -100

        // Clicking in here sets the state to 'middleRight'
        MouseArea { anchors.fill: parent; onClicked: page.state = 'scanTypeProportional' }
    }

    Rectangle {
        id: scanTypeConstantRect

        anchors { right: parent.right; top: parent.top; topMargin: 20; rightMargin: 10 }
        width: 46; height: 54
        color: "Transparent"; border.color: "Gray"; radius: 6

        // Clicking in here sets the state to 'bottomLeft'
        MouseArea { anchors.fill: parent; onClicked: page.state = 'scanTypeConstant' }
    }

    states: [
        // In state 'middleRight', move the image to middleRightRect
        State {
            name: "scanTypeProportional"
            PropertyChanges { target: userIcon; x: scanTypeProportionalRect.x; y: scanTypeProportionalRect.y }

            PropertyChanges {
                target: scanTypeProportionalRect
                x: 89
                y: 19
                anchors.rightMargin: 205
                anchors.verticalCenterOffset: -94
            }

            PropertyChanges {
                target: scanTypeConstantRect
                x: 170
                y: 20
                anchors.bottomMargin: 206
                anchors.leftMargin: 170
            }
        },

        // In state 'bottomLeft', move the image to bottomLeftRect
        State {
            name: "scanTypeConstant"
            PropertyChanges { target: userIcon; x: scanTypeConstantRect.x; y: scanTypeConstantRect.y  }

            PropertyChanges {
                target: scanTypeConstantRect
                x: 203
                y: 17
                anchors.bottomMargin: 209
                anchors.leftMargin: 203
            }

            PropertyChanges {
                target: scanTypeProportionalRect
                x: 110
                y: 19
                anchors.rightMargin: 184
                anchors.verticalCenterOffset: -94
            }
        }
    ]

    // Transitions define how the properties change when the item moves between each state
    transitions: [

        // When transitioning to 'middleRight' move x,y over a duration of 1 second,
        // with OutBounce easing function.
        Transition {
            from: "*"; to: "scanTypeProportional"
            NumberAnimation { properties: "x,y"; easing.type: Easing.OutBounce; duration: 1000 }
        },

        // When transitioning to 'bottomLeft' move x,y over a duration of 2 seconds,
        // with InOutQuad easing function.
        Transition {
            from: "*"; to: "scanTypeConstant"
            NumberAnimation { properties: "x,y"; easing.type: Easing.InOutQuad; duration: 1000 }
        },

        // For any other state changes move x,y linearly over duration of 200ms.
        Transition {
            NumberAnimation { properties: "x,y"; duration: 200 }
        }
    ]
}

