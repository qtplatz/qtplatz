// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuickDesignerTheme 1.0
import Qt.labs.qmlmodels 1.0
import HelperWidgets 2.0
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

FocusScope {
    id: root

    readonly property int delegateTopAreaHeight: StudioTheme.Values.height + 8
    readonly property int delegateBottomAreaHeight: delegateHeight - 2 * delegateStateMargin - delegateTopAreaHeight - 2
    readonly property int delegateStateMargin: 16
    readonly property int delegatePreviewMargin: 10
    readonly property int effectiveHeight: root.height < 130 ? 89 : Math.min(root.height, 287)

    readonly property int scrollBarH: statesListView.ScrollBar.horizontal.scrollBarVisible ? StudioTheme.Values.scrollBarThickness : 0
    readonly property int listMargin: 10
    readonly property int delegateWidth: 264
    readonly property int delegateHeight: Math.max(effectiveHeight - scrollBarH - 2 * listMargin, 69)
    readonly property int innerSpacing: 2

    property int currentStateInternalId: 0

    signal createNewState
    signal deleteState(int internalNodeId)
    signal duplicateCurrentState

    Connections {
        target: statesEditorModel
        function onChangedToState(n) { root.currentStateInternalId = n }
    }

    Rectangle {
        id: background
        anchors.fill: parent
        color: StudioTheme.Values.themePanelBackground
    }

    AbstractButton {
        id: addStateButton

        buttonIcon: StudioTheme.Constants.plus
        iconFont: StudioTheme.Constants.iconFont
        iconSize: StudioTheme.Values.myIconFontSize
        tooltip: qsTr("Add a new state.")
        visible: canAddNewStates
        anchors.right: parent.right
        anchors.rightMargin: 4
        anchors.bottom: parent.bottom
        anchors.bottomMargin: statesListView.contentWidth - statesListView.contentX - root.delegateWidth / 2 > statesListView.width ? scrollBarH + 5 : -35
        width: 35
        height: 35

        Behavior on anchors.bottomMargin {
            PropertyAnimation {
                duration: 700
                easing.type: Easing.InOutBack
            }
        }

        onClicked: root.createNewState()
    }

    ListView {
        id: statesListView

        clip: true
        anchors.fill: parent
        anchors.topMargin: listMargin
        anchors.leftMargin: listMargin
        anchors.rightMargin: listMargin

        model: statesEditorModel
        orientation: ListView.Horizontal
        spacing: root.innerSpacing

        property int prevCount: 0
        onCountChanged: {
            if (count > prevCount)
                Qt.callLater(statesListView.positionViewAtEnd)
            prevCount = count
        }

        delegate: DelegateChooser {
            role: "type"

            DelegateChoice {
                roleValue: "state"

                StatesDelegate {
                    width: root.delegateWidth
                    height: root.delegateHeight
                    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                    anchors.verticalCenterOffset: -.5 * (scrollBarH + listMargin)
                    isBaseState: 0 === internalNodeId
                    isCurrentState: root.currentStateInternalId === internalNodeId
                    delegateStateName: stateName
                    delegateStateImageSource: stateImageSource
                    delegateHasWhenCondition: hasWhenCondition
                    delegateWhenConditionString: whenConditionString

                    topAreaHeight: root.delegateTopAreaHeight
                    bottomAreaHeight: root.delegateBottomAreaHeight
                    stateMargin: root.delegateStateMargin
                    previewMargin: root.delegatePreviewMargin
                    scrollBarH: root.scrollBarH
                    listMargin: root.listMargin
                }
            }

            DelegateChoice {
                roleValue: "add"

                Rectangle {
                    visible: canAddNewStates

                    width: root.delegateWidth
                    height: root.delegateHeight
                    anchors.verticalCenter: parent ? parent.verticalCenter : undefined
                    anchors.verticalCenterOffset: -.5 * (scrollBarH + listMargin)
                    color: Qt.lighter(StudioTheme.Values.themeControlBackgroundInteraction, addState.containsMouse ? 1.5 : 1)

                    ToolTip.text: qsTr("Add a new state.")
                    ToolTip.visible: addState.containsMouse
                    ToolTip.delay: 1000

                    Rectangle { // inner rect
                        width: parent.width - 30
                        height: parent.height - 30
                        anchors.centerIn: parent
                        color: StudioTheme.Values.themeStateBackground
                    }

                    Text {
                        text: "+"
                        anchors.centerIn: parent
                        anchors.verticalCenterOffset: -(5 + (font.pixelSize - 35) / 9)
                        font.pixelSize: parent.height * .5
                        color: Qt.lighter(StudioTheme.Values.themeControlBackgroundInteraction, addState.containsMouse ? 1.5 : 1)
                    }

                    MouseArea {
                        id: addState
                        hoverEnabled: true
                        anchors.fill: parent
                        onClicked: root.createNewState()
                    }
                }
            }
        }

        ScrollBar.horizontal: HorizontalScrollBar {}
    }
}
