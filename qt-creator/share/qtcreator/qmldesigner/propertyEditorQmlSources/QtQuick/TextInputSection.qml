// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import StudioTheme 1.0 as StudioTheme

Section {
    id: textInputSection
    anchors.left: parent.left
    anchors.right: parent.right
    caption: qsTr("Text Input")

    property bool isTextInput: false

    SectionLayout {
        PropertyLabel { text: qsTr("Selection color") }

        ColorEditor {
            backendValue: backendValues.selectionColor
            supportGradient: false
        }

        PropertyLabel { text: qsTr("Selected text color") }

        ColorEditor {
            backendValue: backendValues.selectedTextColor
            supportGradient: false
        }

        PropertyLabel { text: qsTr("Selection mode") }

        SecondColumnLayout {
            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                backendValue: backendValues.mouseSelectionMode
                scope: "TextInput"
                model: ["SelectCharacters", "SelectWords"]
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: textInputSection.isTextInput
            text: qsTr("Input mask")
        }

        SecondColumnLayout {
            visible: textInputSection.isTextInput

            LineEdit {
                backendValue: backendValues.inputMask
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                showTranslateCheckBox: false
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: textInputSection.isTextInput
            text: qsTr("Echo mode")
        }

        SecondColumnLayout {
            visible: textInputSection.isTextInput

            ComboBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                backendValue: backendValues.echoMode
                scope: "TextInput"
                model: ["Normal", "Password", "PasswordEchoOnEdit", "NoEcho"]
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: textInputSection.isTextInput
            text: qsTr("Password character")
            tooltip: qsTr("Character displayed when users enter passwords.")
        }

        SecondColumnLayout {
            visible: textInputSection.isTextInput

            LineEdit {
                backendValue: backendValues.passwordCharacter
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                width: implicitWidth
                showTranslateCheckBox: false
            }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: !textInputSection.isTextInput
            text: qsTr("Tab stop distance")
            tooltip: qsTr("Default distance between tab stops in device units.")
        }

        SecondColumnLayout {
            visible: !textInputSection.isTextInput

            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.tabStopDistance
                maximumValue: 200
                minimumValue: 0
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            ControlLabel { text: "px" }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: !textInputSection.isTextInput
            text: qsTr("Text margin")
            tooltip: qsTr("Margin around the text in the Text Edit in pixels.")
        }

        SecondColumnLayout {
            visible: !textInputSection.isTextInput

            SpinBox {
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.textMargin
                maximumValue: 200
                minimumValue: -200
            }

            Spacer { implicitWidth: StudioTheme.Values.controlLabelGap }

            ControlLabel { text: "px" }

            ExpandingSpacer {}
        }

        PropertyLabel {
            visible: textInputSection.isTextInput
            text: qsTr("Maximum length")
            tooltip: qsTr("Maximum permitted length of the text in the Text Input.")
        }

        SecondColumnLayout {
            visible: textInputSection.isTextInput

            SpinBox {
                implicitWidth: StudioTheme.Values.singleControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                backendValue: backendValues.maximumLength
                minimumValue: 0
                maximumValue: 32767
            }

            ExpandingSpacer {}
        }

        component FlagItem : SecondColumnLayout {
            property alias backendValue: checkBox.backendValue
            CheckBox {
                id: checkBox
                implicitWidth: StudioTheme.Values.twoControlColumnWidth
                               + StudioTheme.Values.actionIndicatorWidth
                text: (checkBox.backendValue === undefined) ? "" : checkBox.backendValue.valueToString
            }

            ExpandingSpacer {}
        }

        PropertyLabel { text: qsTr("Read only") }

        FlagItem { backendValue: backendValues.readOnly }

        PropertyLabel { text: qsTr("Cursor visible") }

        FlagItem { backendValue: backendValues.cursorVisible }

        PropertyLabel { text: qsTr("Focus on press") }

        FlagItem { backendValue: backendValues.activeFocusOnPress }

        PropertyLabel {
            visible: textInputSection.isTextInput
            text: qsTr("Auto scroll")
        }

        FlagItem {
            visible: textInputSection.isTextInput
            backendValue: backendValues.autoScroll
        }

        PropertyLabel { text: qsTr("Overwrite mode") }

        FlagItem { backendValue: backendValues.overwriteMode }

        PropertyLabel { text: qsTr("Persistent selection") }

        FlagItem { backendValue: backendValues.persistentSelection }

        PropertyLabel { text: qsTr("Select by mouse") }

        FlagItem { backendValue: backendValues.selectByMouse }

        PropertyLabel {
            visible: !textInputSection.isTextInput
            text: qsTr("Select by keyboard")
        }

        FlagItem {
            visible: !textInputSection.isTextInput
            backendValue: backendValues.selectByKeyboard
        }
    }
}
