/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
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
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

StudioControls.ComboBox {
    id: comboBox

    property variant backendValue

    labelColor: edit && !colorLogic.errorState ? StudioTheme.Values.themeTextColor
                                               : colorLogic.textColor
    property string scope: "Qt"

    enum ValueType { String, Integer, Enum }
    property int valueType: ComboBox.ValueType.Enum

    onModelChanged: colorLogic.invalidate()

    hasActiveDrag: comboBox.backendValue !== undefined && comboBox.backendValue.hasActiveDrag

    // This is available in all editors.

    onValueTypeChanged: {
        if (comboBox.valueType === ComboBox.ValueType.Integer)
            comboBox.useInteger = true
        else
            comboBox.useInteger = false
    }

    // This property shouldn't be used anymore, valueType has come to replace it.
    property bool useInteger: false

    onUseIntegerChanged: {
        if (comboBox.useInteger) {
            comboBox.valueType = ComboBox.ValueType.Integer
        } else {
            if (comboBox.valueType === ComboBox.ValueType.Integer)
                comboBox.valueType = ComboBox.ValueType.Enum // set to default
        }
    }

    property bool __isCompleted: false

    property bool manualMapping: false

    signal valueFromBackendChanged

    property bool block: false

    property bool showExtendedFunctionButton: true

    property alias colorLogic: colorLogic

    DropArea {
        id: dropArea

        anchors.fill: parent

        property string assetPath: ""

        onEntered: (drag) => {
            dropArea.assetPath = drag.getDataAsString(drag.keys[0]).split(",")[0]
            drag.accepted = comboBox.backendValue !== undefined && comboBox.backendValue.hasActiveDrag
            comboBox.hasActiveHoverDrag = drag.accepted
        }

        onExited: comboBox.hasActiveHoverDrag = false

        onDropped: {
            comboBox.backendValue.commitDrop(dropArea.assetPath)
            comboBox.hasActiveHoverDrag = false
        }

    }

    ExtendedFunctionLogic {
        id: extFuncLogic
        backendValue: comboBox.backendValue
    }

    actionIndicator.icon.color: extFuncLogic.color
    actionIndicator.icon.text: extFuncLogic.glyph
    actionIndicator.onClicked: extFuncLogic.show()
    actionIndicator.forceVisible: extFuncLogic.menuVisible

    actionIndicator.visible: comboBox.showExtendedFunctionButton

    ColorLogic {
        id: colorLogic
        backendValue: comboBox.backendValue
        onValueFromBackendChanged: colorLogic.invalidate()

        function invalidate() {
            if (comboBox.block)
                return

            comboBox.block = true

            if (comboBox.manualMapping) {
                comboBox.valueFromBackendChanged()
            } else {
                switch (comboBox.valueType) {
                case ComboBox.ValueType.String:
                    if (comboBox.currentText !== comboBox.backendValue.value) {
                        var index = comboBox.find(comboBox.backendValue.value)
                        if (index < 0)
                            index = 0

                        if (index !== comboBox.currentIndex)
                            comboBox.currentIndex = index
                    }
                    break
                case ComboBox.ValueType.Integer:
                    if (comboBox.currentIndex !== comboBox.backendValue.value)
                        comboBox.currentIndex = comboBox.backendValue.value
                    break
                case ComboBox.ValueType.Enum:
                default:
                    if (comboBox.backendValue === undefined)
                        break

                    var enumString = comboBox.backendValue.enumeration

                    if (enumString === "")
                        enumString = comboBox.backendValue.value

                    index = comboBox.find(enumString)

                    if (index < 0)
                        index = 0

                    if (index !== comboBox.currentIndex)
                        comboBox.currentIndex = index
                }
            }

            comboBox.block = false
        }
    }

    onAccepted: {
        if (!comboBox.__isCompleted)
            return

        let inputValue = comboBox.editText

        let index = comboBox.find(inputValue)
        if (index !== -1)
            inputValue = comboBox.textAt(index)

        comboBox.backendValue.value = inputValue

        comboBox.dirty = false
    }

    onCompressedActivated: {
        if (!comboBox.__isCompleted)
            return

        if (comboBox.backendValue === undefined)
            return

        if (comboBox.manualMapping)
            return

        switch (comboBox.valueType) {
        case ComboBox.ValueType.String:
            comboBox.backendValue.value = comboBox.currentText
            break
        case ComboBox.ValueType.Integer:
            comboBox.backendValue.value = comboBox.currentIndex
            break
        case ComboBox.ValueType.Enum:
        default:
            comboBox.backendValue.setEnumeration(comboBox.scope, comboBox.currentText)
        }
    }

    Component.onCompleted: {
        colorLogic.invalidate()
        comboBox.__isCompleted = true
    }

    Connections {
        target: modelNodeBackend
        function onSelectionToBeChanged() {
            comboBox.popup.close()
        }
    }
}
