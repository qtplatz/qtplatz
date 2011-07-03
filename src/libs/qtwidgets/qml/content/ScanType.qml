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
    id: scanTypeRect
    width: parent.width; height: parent.height
    color: "#1c1c47"

    Image {
        id: userIcon
        x: scanTypeTofRect.x; y: scanTypeTofRect.y
        source: "images/scantype.png"
    }

    Text {
        id: caption;
        text: "Scan Type:"; font.pointSize: 13; font.family: "Monotype Corsiva"; color: "white"
    }

    Rectangle {
        id: scanTypeTofRect

        anchors { left: caption.right; top: parent.top; leftMargin: 20; topMargin: 5 }
        width: 46; height: 54
        color: "Transparent"; border.color: "Gray"; radius: 6
        Row {
            Text { text: "Tof"; font.pointSize: 20; font.family: "Monotype Corsiva"; color: "white" }
        }
        // Clicking in here sets the state to the default state, returning the image to
        // its initial position
        MouseArea { anchors.fill: parent; onClicked: scanTypeRect.state = '' }
    }

    Rectangle {
        id: scanTypeProportionalRect

        anchors { horizontalCenter: parent.horizontalCenter; top: parent.top; topMargin: scanTypeTofRect.anchors.topMargin }
        width: 46; height: 54
        color: "Transparent"; border.color: "Gray"; radius: 6
        // anchors.verticalCenterOffset: -100
        Text { text: "Proportional"; font.pointSize: 20; font.family: "Monotype Corsiva"; color: "white" }
        // Clicking in here sets the state to 'middleRight'
        MouseArea { anchors.fill: parent; onClicked: scanTypeRect.state = 'scanTypeProportional' }
    }

    Rectangle {
        id: scanTypeConstantRect

        anchors { right: parent.right; top: parent.top; topMargin: scanTypeTofRect.anchors.topMargin; rightMargin: 40 }
        width: 46; height: 54
        color: "Transparent"; border.color: "Gray"; radius: 6
        Text { text: "Constant"; font.pointSize: 20; font.family: "Monotype Corsiva"; color: "white" }
        // Clicking in here sets the state to 'bottomLeft'
        MouseArea { anchors.fill: parent; onClicked: scanTypeRect.state = 'scanTypeConstant' }
    }

    states: [
        // In state 'scanTypeProportional', move the image to center
        State {
            name: "scanTypeProportional"
            PropertyChanges { target: userIcon; x: scanTypeProportionalRect.x; y: scanTypeProportionalRect.y }
        },

        // In state 'scanTypeConstant', move the image to right
        State {
            name: "scanTypeConstant"
            PropertyChanges { target: userIcon; x: scanTypeConstantRect.x; y: scanTypeConstantRect.y  }
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

