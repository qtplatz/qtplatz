/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Quick Studio Components.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import LandingPageApi
import LandingPage as Theme

Rectangle {
    id: root

    property bool qdsInstalled: qdsVersionText.text.length > 0
    property bool projectFileExists: false
    property string qtVersion: qsTr("Unknown")
    property string qdsVersion: qsTr("Unknown")
    property alias generateProjectFileButton: generateProjectFileButton

    color: Theme.Colors.backgroundSecondary
    height: column.childrenRect.height + (2 * Theme.Values.spacing)

    Connections {
        target: generateProjectFileButton
        function onClicked() { LandingPageApi.generateProjectFile() }
    }

    Column {
        id: column

        width: parent.width
        anchors.centerIn: parent
        spacing: Theme.Values.spacing

        PageText {
            id: projectFileInfoTitle
            width: parent.width
            text: qsTr("QML PROJECT FILE INFO")
        }

        Column {
            id: projectFileInfoVersionBox
            width: parent.width
            visible: root.projectFileExists

            PageText {
                id: qtVersionText
                width: parent.width
                padding: Theme.Values.spacing
                text: qsTr("Qt Version - ") + root.qtVersion
            }

            PageText {
                id: qdsVersionText
                width: parent.width
                padding: Theme.Values.spacing
                text: qsTr("Qt Design Studio Version - ") + root.qdsVersion
            }
        }

        Column {
            id: projectFileInfoMissingBox
            width: parent.width
            visible: !projectFileInfoVersionBox.visible

            PageText {
                id: projectFileInfoMissingText
                width: parent.width
                padding: Theme.Values.spacing
                text: qsTr("No QML project file found - Would you like to create one?")
            }

            PushButton {
                id: generateProjectFileButton
                text: qsTr("Generate")
                anchors.horizontalCenter: parent.horizontalCenter
            }
        }
    }
}
