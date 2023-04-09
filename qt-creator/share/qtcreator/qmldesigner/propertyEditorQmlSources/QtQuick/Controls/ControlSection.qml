/****************************************************************************
**
** Copyright (C) 2022 The Qt Company Ltd.
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
import HelperWidgets 2.0
import QtQuick.Layouts 1.15
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

Section {
    caption: qsTr("Control")

    anchors.left: parent.left
    anchors.right: parent.right

    SectionLayout {
        PropertyLabel {
            text: qsTr("Enable")
            tooltip: qsTr("Whether the control is enabled and hover events are received.")
        }

        SecondColumnLayout {
            CheckBox {
                text: qsTr("Control")
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.enabled
            }

            Spacer { implicitWidth: StudioTheme.Values.twoControlColumnGap }

            CheckBox {
                text: qsTr("Hover")
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.hoverEnabled
                enabled: backendValues.hoverEnabled.isAvailable
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            text: qsTr("Focus policy")
            tooltip: qsTr("Focus policy of the control.")
            blockedByTemplate: !backendValues.focusPolicy.isAvailable
        }

        SecondColumnLayout {
            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                backendValue: backendValues.focusPolicy
                model: [ "TabFocus", "ClickFocus", "StrongFocus", "WheelFocus", "NoFocus" ]
                scope: "Qt"
                enabled: backendValues.focusPolicy.isAvailable
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            text: qsTr("Spacing")
            tooltip: qsTr("Spacing between internal elements of the control.")
        }

        SecondColumnLayout {
            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                maximumValue: 9999999
                minimumValue: -9999999
                decimals: 0
                backendValue: backendValues.spacing
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            text: qsTr("Wheel")
            tooltip: qsTr("Whether control accepts wheel events.")
            blockedByTemplate: !backendValues.wheelEnabled.isAvailable
        }

        SecondColumnLayout {
            CheckBox {
                text: backendValues.wheelEnabled.valueToString
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.wheelEnabled
                enabled: backendValues.wheelEnabled.isAvailable
            }

            ExpandingSpacer {}
        }
    }
}
