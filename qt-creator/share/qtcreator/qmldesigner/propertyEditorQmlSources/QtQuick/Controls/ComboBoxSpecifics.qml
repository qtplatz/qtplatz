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
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import StudioTheme 1.0 as StudioTheme

Column {
    width: parent.width

    Section {
        caption: qsTr("Combo Box")

        width: parent.width

        SectionLayout {
            PropertyLabel {
                text: qsTr("Text role")
                tooltip: qsTr("The model role used for displaying text.")
            }

            SecondColumnLayout {
                LineEdit {
                    implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                    width: implicitWidth
                    backendValue: backendValues.textRole
                }

                ExpandingSpacer {}
            }

            PropertyLabel {
                text: qsTr("Display text")
                tooltip: qsTr("Holds the text that is displayed on the combo box button.")
            }

            SecondColumnLayout {
                LineEdit {
                    implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                    width: implicitWidth
                    backendValue: backendValues.displayText
                }

                ExpandingSpacer {}
            }

            PropertyLabel {
                text: qsTr("Current index")
                tooltip: qsTr("The index of the current item.")
            }

            SecondColumnLayout {
                SpinBox {
                    implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                    width: implicitWidth
                    minimumValue: -9999999
                    maximumValue: 9999999
                    decimals: 0
                    backendValue: backendValues.currentIndex
                }

                ExpandingSpacer {}
            }

            PropertyLabel {
                text: qsTr("Flat")
                tooltip: qsTr("Whether the combo box button is flat.")
            }

            SecondColumnLayout {
                CheckBox {
                    text: backendValues.flat.valueToString
                    implicitWidth: StudioTheme.Values.twoControlColumnWidth
                                   + StudioTheme.Values.actionIndicatorWidth
                    backendValue: backendValues.flat
                }

                ExpandingSpacer {}
            }

            PropertyLabel {
                text: qsTr("Editable")
                tooltip: qsTr("Whether the combo box is editable.")
            }

            SecondColumnLayout {
                CheckBox {
                    text: backendValues.editable.valueToString
                    implicitWidth: StudioTheme.Values.twoControlColumnWidth
                                   + StudioTheme.Values.actionIndicatorWidth
                    backendValue: backendValues.editable
                }

                ExpandingSpacer {}
            }
        }
    }

    ControlSection {}

    FontSection {}

    PaddingSection {}

    InsetSection {}
}
