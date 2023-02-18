// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuickDesignerTheme 1.0
import HelperWidgets 2.0
import StudioTheme 1.0 as StudioTheme
import ToolBarAction 1.0

Rectangle {
    id: root

    color: StudioTheme.Values.themeSectionHeadBackground
    width: row.width
    height: 40

    signal toolBarAction(int action)

    Row {
        id: row

        anchors.verticalCenter: parent.verticalCenter
        leftPadding: 6

        IconButton {
            icon: StudioTheme.Constants.applyMaterialToSelected

            normalColor: StudioTheme.Values.themeSectionHeadBackground
            iconSize: StudioTheme.Values.bigIconFontSize
            buttonSize: root.height
            enabled: hasMaterial && hasModelSelection && hasQuick3DImport && !hasMaterialRoot
            onClicked: root.toolBarAction(ToolBarAction.ApplyToSelected)
            tooltip: qsTr("Apply material to selected model.")
        }

        IconButton {
            icon: StudioTheme.Constants.newMaterial

            normalColor: StudioTheme.Values.themeSectionHeadBackground
            iconSize: StudioTheme.Values.bigIconFontSize
            buttonSize: root.height
            enabled: hasQuick3DImport && !hasMaterialRoot
            onClicked: root.toolBarAction(ToolBarAction.AddNewMaterial)
            tooltip: qsTr("Create new material.")
        }

        IconButton {
            icon: StudioTheme.Constants.deleteMaterial

            normalColor: StudioTheme.Values.themeSectionHeadBackground
            iconSize: StudioTheme.Values.bigIconFontSize
            buttonSize: root.height
            enabled: hasMaterial && hasQuick3DImport && !hasMaterialRoot
            onClicked: root.toolBarAction(ToolBarAction.DeleteCurrentMaterial)
            tooltip: qsTr("Delete current material.")
        }

        IconButton {
            icon: StudioTheme.Constants.openMaterialBrowser

            normalColor: StudioTheme.Values.themeSectionHeadBackground
            iconSize: StudioTheme.Values.bigIconFontSize
            buttonSize: root.height
            enabled: hasMaterial && hasQuick3DImport && !hasMaterialRoot
            onClicked: root.toolBarAction(ToolBarAction.OpenMaterialBrowser)
            tooltip: qsTr("Open material browser.")
        }
    }
}
