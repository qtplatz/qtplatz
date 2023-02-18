// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

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
