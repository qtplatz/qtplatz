// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import QtQuickDesignerTheme 1.0
import StudioTheme 1.0 as StudioTheme

Section {
    id: section
    caption: qsTr("Button")

    anchors.left: parent.left
    anchors.right: parent.right

    SectionLayout {
        PropertyLabel {
            text: qsTr("Appearance")
            tooltip: qsTr("Whether the button is flat and/or highlighted.")
            blockedByTemplate: !backendValues.flat.isAvailable
                               && !backendValues.highlighted.isAvailable
        }

        SecondColumnLayout {
            CheckBox {
                text: qsTr("Flat")
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.flat
                enabled: backendValue.isAvailable
            }

            Spacer { implicitWidth: StudioTheme.Values.twoControlColumnGap }

            CheckBox {
                text: qsTr("Highlight")
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.highlighted
                enabled: backendValue.isAvailable
            }

            ExpandingSpacer {}
        }
    }
}
