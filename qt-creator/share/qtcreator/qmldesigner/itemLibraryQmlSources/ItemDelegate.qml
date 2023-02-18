// Copyright (C) 2016 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuickDesignerTheme 1.0
import HelperWidgets 2.0
import StudioTheme 1.0 as StudioTheme

Item {
    id: delegateRoot

    property alias textColor: text.color

    signal showContextMenu()

    Rectangle {
        anchors.rightMargin: 1
        anchors.topMargin: 1
        anchors.fill: parent

        color: mouseRegion.containsMouse ? StudioTheme.Values.themeControlBackgroundHover : StudioTheme.Values.themePanelBackground
        Behavior on color {
            ColorAnimation {
                duration: StudioTheme.Values.hoverDuration
                easing.type: StudioTheme.Values.hoverEasing
            }
        }

        Image {
            id: itemIcon // to be set by model

            anchors.top: parent.top
            anchors.topMargin: styleConstants.cellVerticalMargin
            anchors.horizontalCenter: parent.horizontalCenter

            width: itemLibraryIconWidth  // to be set in Qml context
            height: itemLibraryIconHeight   // to be set in Qml context
            source: itemLibraryIconPath     // to be set by model

            // Icons generated for components can change if the component is edited,
            // so don't cache them locally at Image level.
            cache: itemComponentSource === ""
        }

        Text {
            id: text
            font.pixelSize: Theme.smallFontPixelSize()
            elide: Text.ElideMiddle
            wrapMode: Text.WordWrap
            anchors.top: itemIcon.bottom
            anchors.topMargin: styleConstants.cellVerticalSpacing
            anchors.left: parent.left
            anchors.leftMargin: styleConstants.cellHorizontalMargin
            anchors.right: parent.right
            anchors.rightMargin: styleConstants.cellHorizontalMargin
            anchors.bottom: parent.bottom
            anchors.bottomMargin: styleConstants.cellHorizontalMargin

            verticalAlignment: Qt.AlignVCenter
            horizontalAlignment: Qt.AlignHCenter
            text: itemName  // to be set by model
            color: StudioTheme.Values.themeTextColor
            renderType: Text.NativeRendering
        }

        ImagePreviewTooltipArea {
            id: mouseRegion
            anchors.fill: parent

            onShowContextMenu: delegateRoot.showContextMenu()
            onPressed: (mouse)=> {
                allowTooltip = false
                hide()
                if (mouse.button === Qt.LeftButton)
                    rootView.startDragAndDrop(itemLibraryEntry, mapToGlobal(mouse.x, mouse.y))
            }
            onDoubleClicked: (mouse)=> {
                if (mouse.button === Qt.LeftButton && itemComponentSource) {
                    hide()
                    rootView.goIntoComponent(itemComponentSource)
                }
            }
        }
    }
}
