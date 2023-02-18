// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuickDesignerTheme 1.0
import HelperWidgets 2.0

PropertyEditorPane {
    id: itemPane

    signal toolBarAction(int action)
    signal previewEnvChanged(string env)
    signal previewModelChanged(string model)

    // invoked from C++ to refresh material preview image
    function refreshPreview()
    {
        topSection.refreshPreview()
    }

    // Called also from C++ to close context menu on focus out
    function closeContextMenu()
    {
        topSection.closeContextMenu()
    }

    // Called from C++ to initialize preview menu checkmarks
    function initPreviewData(env, model)
    {
        topSection.previewEnv = env;
        topSection.previewModel = model
    }

    MaterialEditorTopSection {
        id: topSection

        onToolBarAction: (action) => itemPane.toolBarAction(action)
        onPreviewEnvChanged: itemPane.previewEnvChanged(previewEnv)
        onPreviewModelChanged: itemPane.previewModelChanged(previewModel)
    }

    Item { width: 1; height: 10 }

    DynamicPropertiesSection {
        propertiesModel: MaterialEditorDynamicPropertiesModel {}
    }

    Loader {
        id: specificsTwo

        property string theSource: specificQmlData

        anchors.left: parent.left
        anchors.right: parent.right
        visible: theSource !== ""
        sourceComponent: specificQmlComponent

        onTheSourceChanged: {
            active = false
            active = true
        }
    }

    Item {
        width: 1
        height: 10
        visible: specificsTwo.visible
    }

    Loader {
        id: specificsOne
        anchors.left: parent.left
        anchors.right: parent.right
        source: specificsUrl
    }
}
