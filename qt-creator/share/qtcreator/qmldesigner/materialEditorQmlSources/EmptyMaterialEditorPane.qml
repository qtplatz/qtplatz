// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuickDesignerTheme 1.0
import HelperWidgets 2.0
import StudioTheme 1.0 as StudioTheme

PropertyEditorPane {
    id: root

    signal toolBarAction(int action)
    signal previewEnvChanged(string env)
    signal previewModelChanged(string model)

    // Called from C++, dummy methods to avoid warnings
    function closeContextMenu() {}
    function initPreviewData(env, model) {}

    Column {
        id: col

        MaterialEditorToolBar {
            width: root.width

            onToolBarAction: (action) => root.toolBarAction(action)
        }

        Item {
            width: root.width - 2 * col.padding
            height: 150

            Text {
                text: hasQuick3DImport ? qsTr("There are no materials in this project.<br>Select '<b>+</b>' to create one.")
                                       : qsTr("To use <b>Material Editor</b>, first add the QtQuick3D module in the <b>Components</b> view.")
                textFormat: Text.RichText
                color: StudioTheme.Values.themeTextColor
                font.pixelSize: StudioTheme.Values.mediumFontSize
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                width: root.width
                anchors.centerIn: parent
            }
        }
    }
}
