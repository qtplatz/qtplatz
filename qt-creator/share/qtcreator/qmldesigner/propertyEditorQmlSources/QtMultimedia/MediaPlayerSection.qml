/****************************************************************************
**
** Copyright (C) 2021 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
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
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

Section {
    id: root
    caption: qsTr("Media Player")

    anchors.left: parent.left
    anchors.right: parent.right

    property bool showAudioOutput: false
    property bool showVideoOutput: false

    // TODO position property, what should be the range?!

    SectionLayout {
        PropertyLabel { text: qsTr("Playback rate") }

        SecondColumnLayout {
            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.playbackRate
                decimals: 1
                minimumValue: -1000 // TODO correct range
                maximumValue: 1000
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showAudioOutput
            text: qsTr("Audio output")
            tooltip: qsTr("Target audio output.")
        }

        SecondColumnLayout {
            visible: root.showAudioOutput

            ItemFilterComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                typeFilter: "QtQuick.AudioOutput"
                validator: RegExpValidator { regExp: /(^$|^[a-z_]\w*)/ }
                backendValue: backendValues.audioOutput
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: root.showVideoOutput
            text: qsTr("Video output")
            tooltip: qsTr("Target video output.")
        }

        SecondColumnLayout {
            visible: root.showVideoOutput

            ItemFilterComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                typeFilter: "QtQuick.VideoOutput"
                validator: RegExpValidator { regExp: /(^$|^[a-z_]\w*)/ }
                backendValue: backendValues.videoOutput
            }

            ExpandingSpacer {}
        }
    }
}
