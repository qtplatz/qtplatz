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
import StudioTheme 1.0 as StudioTheme

Section {
    anchors.left: parent.left
    anchors.right: parent.right
    caption: qsTr("Grid Layout")

    SectionLayout {
        PropertyLabel { text: qsTr("Columns & Rows") }

        SecondColumnLayout {
            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.columns
                minimumValue: 0
                maximumValue: 2000
                decimals: 0
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            IconLabel { icon: StudioTheme.Constants.columnsAndRows }

            Spacer { implicitWidth: StudioTheme.Values.controlGap }

            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.rows
                minimumValue: 0
                maximumValue: 2000
                decimals: 0
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            IconLabel {
                icon: StudioTheme.Constants.columnsAndRows
                rotation: 90
            }

            ExpandingSpacer {}
        }

        PropertyLabel { text: qsTr("Spacing") }

        SecondColumnLayout {
            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.columnSpacing
                minimumValue: -4000
                maximumValue: 4000
                decimals: 0
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            IconLabel { icon: StudioTheme.Constants.columnsAndRows }

            Spacer { implicitWidth: StudioTheme.Values.controlGap }

            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.rowSpacing
                minimumValue: -4000
                maximumValue: 4000
                decimals: 0
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            IconLabel {
                icon: StudioTheme.Constants.columnsAndRows
                rotation: 90
            }

            ExpandingSpacer {}
        }

        PropertyLabel { text: qsTr("Flow") }

        SecondColumnLayout {
            ComboBox {
                model: ["LeftToRight", "TopToBottom"]
                backendValue: backendValues.flow
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                scope: "GridLayout"
            }

            ExpandingSpacer {}
        }

        PropertyLabel { text: qsTr("Layout direction") }

        SecondColumnLayout {
            ComboBox {
                model: ["LeftToRight", "RightToLeft"]
                backendValue: backendValues.layoutDirection
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                scope: "Qt"
            }

            ExpandingSpacer {}
        }
    }
}
