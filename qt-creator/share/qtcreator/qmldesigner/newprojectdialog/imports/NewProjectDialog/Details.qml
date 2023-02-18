// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick.Window
import QtQuick.Controls

import QtQuick
import QtQuick.Layouts
import StudioControls as SC
import StudioTheme as StudioTheme

import BackendApi

Item {
    width: DialogValues.detailsPaneWidth

    Component.onCompleted: BackendApi.detailsLoaded = true
    Component.onDestruction: BackendApi.detailsLoaded = false

    Rectangle {
        color: DialogValues.darkPaneColor
        anchors.fill: parent

        Item {
            x: DialogValues.detailsPanePadding                               // left padding
            width: parent.width - DialogValues.detailsPanePadding * 2        // right padding
            height: parent.height

            Column {
                anchors.fill: parent
                spacing: 5

                Text {
                    id: detailsHeading
                    text: qsTr("Details")
                    height: DialogValues.paneTitleTextHeight
                    width: parent.width
                    font.weight: Font.DemiBold
                    font.pixelSize: DialogValues.paneTitlePixelSize
                    lineHeight: DialogValues.paneTitleLineHeight
                    lineHeightMode: Text.FixedHeight
                    color: DialogValues.textColor
                    verticalAlignment: Qt.AlignVCenter
                }

                Flickable {
                    width: parent.width
                    height: parent.height - detailsHeading.height - DialogValues.defaultPadding
                            - savePresetButton.height
                    contentWidth: parent.width
                    contentHeight: scrollContent.height
                    boundsBehavior: Flickable.StopAtBounds
                    clip: true

                    ScrollBar.vertical: SC.VerticalScrollBar {}

                    Column {
                        id: scrollContent
                        width: parent.width - DialogValues.detailsPanePadding
                        spacing: DialogValues.defaultPadding

                        SC.TextField {
                            id: projectNameTextField
                            actionIndicatorVisible: false
                            translationIndicatorVisible: false
                            text: BackendApi.projectName
                            width: parent.width
                            color: DialogValues.textColor
                            selectByMouse: true
                            font.pixelSize: DialogValues.defaultPixelSize

                            onEditingFinished: {
                                text = text.charAt(0).toUpperCase() + text.slice(1)
                            }
                        }

                        Binding {
                            target: BackendApi
                            property: "projectName"
                            value: projectNameTextField.text
                        }

                        Item { width: parent.width; height: DialogValues.narrowSpacing(11) }

                        RowLayout { // Project location
                            width: parent.width

                            SC.TextField {
                                Layout.fillWidth: true
                                id: projectLocationTextField
                                actionIndicatorVisible: false
                                translationIndicatorVisible: false
                                text: BackendApi.projectLocation
                                color: DialogValues.textColor
                                selectByMouse: true
                                font.pixelSize: DialogValues.defaultPixelSize
                            }

                            Binding {
                                target: BackendApi
                                property: "projectLocation"
                                value: projectLocationTextField.text
                            }

                            SC.AbstractButton {
                                implicitWidth: 30
                                iconSize: 20
                                visible: true
                                buttonIcon: "…"
                                iconFont: StudioTheme.Constants.font

                                onClicked: {
                                    var newLocation = BackendApi.chooseProjectLocation()
                                    if (newLocation)
                                        projectLocationTextField.text = newLocation
                                }
                            } // SC.AbstractButton
                        } // Project location RowLayout

                        Item { width: parent.width; height: DialogValues.narrowSpacing(7) }

                        RowLayout { // StatusMessage
                            width: parent.width
                            spacing: 0

                            Image {
                                id: statusIcon
                                Layout.alignment: Qt.AlignTop
                                asynchronous: false
                            }

                            Text {
                                id: statusMessage
                                text: BackendApi.statusMessage
                                font.pixelSize: DialogValues.defaultPixelSize
                                lineHeight: DialogValues.defaultLineHeight
                                lineHeightMode: Text.FixedHeight
                                color: DialogValues.textColor
                                wrapMode: Text.Wrap
                                elide: Text.ElideRight
                                maximumLineCount: 3
                                Layout.fillWidth: true

                                states: [
                                    State {
                                        name: "warning"
                                        when: BackendApi.statusType === "warning"
                                        PropertyChanges {
                                            target: statusMessage
                                            color: DialogValues.textWarning
                                        }
                                        PropertyChanges {
                                            target: statusIcon
                                            source: "image://newprojectdialog_library/status-warning"
                                        }
                                    },

                                    State {
                                        name: "error"
                                        when: BackendApi.statusType === "error"
                                        PropertyChanges {
                                            target: statusMessage
                                            color: DialogValues.textError
                                        }
                                        PropertyChanges {
                                            target: statusIcon
                                            source: "image://newprojectdialog_library/status-error"
                                        }
                                    }
                                ]
                            } // Text
                        } // RowLayout

                        SC.CheckBox {
                            id: defaultLocationCheckbox
                            actionIndicatorVisible: false
                            text: qsTr("Use as default project location")
                            checked: false
                            font.pixelSize: DialogValues.defaultPixelSize
                        }

                        Binding {
                            target: BackendApi
                            property: "saveAsDefaultLocation"
                            value: defaultLocationCheckbox.checked
                        }

                        Rectangle { width: parent.width; height: 1; color: DialogValues.dividerlineColor }

                        SC.ComboBox {   // Screen Size ComboBox
                            id: screenSizeComboBox
                            actionIndicatorVisible: false
                            currentIndex: -1
                            model: BackendApi.screenSizeModel
                            textRole: "display"
                            width: parent.width
                            font.pixelSize: DialogValues.defaultPixelSize

                            onActivated: (index) => {
                                BackendApi.setScreenSizeIndex(index);

                                var size = BackendApi.screenSizeModel.screenSizes(index);
                                widthField.realValue = size.width;
                                heightField.realValue = size.height;
                            }

                            Connections {
                                target: BackendApi.screenSizeModel
                                function onModelReset() {
                                    var newIndex = screenSizeComboBox.currentIndex > -1
                                            ? screenSizeComboBox.currentIndex
                                            : BackendApi.screenSizeIndex()

                                    screenSizeComboBox.currentIndex = newIndex
                                    screenSizeComboBox.activated(newIndex)
                                }
                            }
                        } // Screen Size ComboBox

                        GridLayout { // orientation + width + height
                            width: parent.width
                            height: 85
                            columns: 4
                            rows: 2
                            columnSpacing: 10
                            rowSpacing: 10

                            // header items
                            Text {
                                text: qsTr("Width")
                                font.pixelSize: DialogValues.defaultPixelSize
                                lineHeight: DialogValues.defaultLineHeight
                                lineHeightMode: Text.FixedHeight
                                color: DialogValues.textColor
                            }

                            Text {
                                text: qsTr("Height")
                                font.pixelSize: DialogValues.defaultPixelSize
                                lineHeight: DialogValues.defaultLineHeight
                                lineHeightMode: Text.FixedHeight
                                color: DialogValues.textColor
                            }

                            Item { Layout.fillWidth: true }

                            Text {
                                text: qsTr("Orientation")
                                font.pixelSize: DialogValues.defaultPixelSize
                                lineHeight: DialogValues.defaultLineHeight
                                lineHeightMode: Text.FixedHeight
                                color: DialogValues.textColor
                            }

                            // content items
                            SC.RealSpinBox {
                                id: widthField
                                actionIndicatorVisible: false
                                implicitWidth: 70
                                labelColor: DialogValues.textColor
                                realFrom: 100
                                realTo: 100000
                                realValue: 100
                                realStepSize: 10
                                font.pixelSize: DialogValues.defaultPixelSize

                                onRealValueChanged: orientationButton.isHorizontal =
                                                    widthField.realValue >= heightField.realValue
                            } // Width Text Field

                            Binding {
                                target: BackendApi
                                property: "customWidth"
                                value: widthField.realValue
                            }

                            SC.RealSpinBox {
                                id: heightField
                                actionIndicatorVisible: false
                                implicitWidth: 70
                                labelColor: DialogValues.textColor
                                realFrom: 100
                                realTo: 100000
                                realValue: 100
                                realStepSize: 10
                                font.pixelSize: DialogValues.defaultPixelSize

                                onRealValueChanged: orientationButton.isHorizontal =
                                                    widthField.realValue >= heightField.realValue
                            } // Height Text Field

                            Binding {
                                target: BackendApi
                                property: "customHeight"
                                value: heightField.realValue
                            }

                            Item { Layout.fillWidth: true }

                            Item {
                                id: orientationButton
                                implicitWidth: 100
                                implicitHeight: 50
                                property bool isHorizontal: false

                                Row {
                                    spacing: orientationButton.width / 4

                                    function computeColor(barId) {
                                        var color = DialogValues.textColor

                                        if (barId === horizontalBar) {
                                            color = orientationButton.isHorizontal
                                                    ? DialogValues.textColorInteraction
                                                    : DialogValues.textColor
                                        } else {
                                            color = orientationButton.isHorizontal
                                                    ? DialogValues.textColor
                                                    : DialogValues.textColorInteraction
                                        }

                                        if (orientationButtonMouseArea.containsMouse)
                                            color = Qt.darker(color, 1.5)

                                        return color
                                    }

                                    Rectangle {
                                        id: horizontalBar
                                        color: parent.computeColor(horizontalBar)
                                        width: orientationButton.width / 2
                                        height: orientationButton.height / 2
                                        anchors.verticalCenter: parent.verticalCenter
                                        radius: 3
                                    }

                                    Rectangle {
                                        id: verticalBar
                                        color: parent.computeColor(verticalBar)
                                        width: orientationButton.width / 4
                                        height: orientationButton.height
                                        radius: 3
                                    }
                                }

                                MouseArea {
                                    id: orientationButtonMouseArea
                                    anchors.fill: parent
                                    hoverEnabled: true

                                    onClicked: {
                                        if (widthField.realValue && heightField.realValue) {
                                            [widthField.realValue, heightField.realValue] = [heightField.realValue, widthField.realValue]

                                            if (widthField.realValue === heightField.realValue)
                                                orientationButton.isHorizontal = !orientationButton.isHorizontal
                                            else
                                                orientationButton.isHorizontal = widthField.realValue > heightField.realValue
                                        }
                                    }
                                }
                            } // Orientation button
                        } // GridLayout: orientation + width + height

                        Rectangle {
                            width: parent.width
                            height: 1
                            color: DialogValues.dividerlineColor
                        }

                        SC.CheckBox {
                            id: useQtVirtualKeyboard
                            actionIndicatorVisible: false
                            text: qsTr("Use Qt Virtual Keyboard")
                            font.pixelSize: DialogValues.defaultPixelSize
                            checked: BackendApi.useVirtualKeyboard
                            visible: BackendApi.haveVirtualKeyboard
                        }

                        RowLayout { // Target Qt Version
                            width: parent.width
                            visible: BackendApi.haveTargetQtVersion

                            Text {
                                text: qsTr("Target Qt Version:")
                                font.pixelSize: DialogValues.defaultPixelSize
                                lineHeight: DialogValues.defaultLineHeight
                                lineHeightMode: Text.FixedHeight
                                color: DialogValues.textColor
                            }

                            SC.ComboBox {   // Target Qt Version ComboBox
                                id: qtVersionComboBox
                                actionIndicatorVisible: false
                                implicitWidth: 82
                                Layout.alignment: Qt.AlignRight
                                currentIndex: BackendApi.targetQtVersionIndex
                                font.pixelSize: DialogValues.defaultPixelSize

                                model: ListModel {
                                    ListElement { name: "Qt 5.15" }
                                    ListElement { name: "Qt 6.2" }
                                    ListElement { name: "Qt 6.3" }
                                    ListElement { name: "Qt 6.4" }
                                }

                                onActivated: (index) => {
                                    BackendApi.targetQtVersionIndex = index
                                }
                            } // Target Qt Version ComboBox

                            Binding {
                                target: BackendApi
                                property: "targetQtVersionIndex"
                                value: qtVersionComboBox.currentIndex
                            }

                        } // RowLayout

                        Binding {
                            target: BackendApi
                            property: "useVirtualKeyboard"
                            value: useQtVirtualKeyboard.checked
                        }
                    } // ScrollContent Column
                } // ScrollView
            } // Column

            SC.AbstractButton {
                id: savePresetButton
                width: StudioTheme.Values.singleControlColumnWidth
                buttonIcon: qsTr("Save Custom Preset")
                iconFont: StudioTheme.Constants.font
                iconSize: DialogValues.defaultPixelSize
                anchors.bottom: parent.bottom
                anchors.horizontalCenter: parent.horizontalCenter

                onClicked: savePresetDialog.open()
            }

            PopupDialog {
                id: savePresetDialog
                title: qsTr("Save Preset")
                standardButtons: Dialog.Save | Dialog.Cancel
                modal: true
                closePolicy: Popup.CloseOnEscape
                anchors.centerIn: parent
                width: DialogValues.popupDialogWidth

                onAccepted: BackendApi.savePresetDialogAccept()

                onOpened: {
                    presetNameTextField.selectAll()
                    presetNameTextField.forceActiveFocus()
                }

                ColumnLayout {
                    width: parent.width
                    spacing: 10

                    Text {
                        text: qsTr("Preset name")
                        font.pixelSize: DialogValues.defaultPixelSize
                        color: DialogValues.textColor
                    }

                    SC.TextField {
                        id: presetNameTextField
                        actionIndicatorVisible: false
                        translationIndicatorVisible: false
                        text: qsTr("MyPreset")
                        color: DialogValues.textColor
                        font.pixelSize: DialogValues.defaultPixelSize
                        Layout.fillWidth: true
                        maximumLength: 30
                        validator: RegularExpressionValidator { regularExpression: /\w[\w ]*/ }

                        onEditingFinished: {
                            presetNameTextField.text = text.trim()
                            presetNameTextField.text = text.replace(/\s+/g, " ")
                        }

                        onAccepted: savePresetDialog.accept()
                    }

                    Binding {
                        target: BackendApi
                        property: "presetName"
                        value: presetNameTextField.text
                    }
                }
            }
        } // Item
    } // Rectangle
} // root Item
