// Copyright (C) 2022 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import StudioTheme 1.0 as StudioTheme

Column {
    width: parent.width

    PaneSection {}

    ControlSection {}

    PaddingSection {}

    InsetSection {}

    FontSection {
        caption: qsTr("Font Inheritance")
        expanded: false
    }
}
