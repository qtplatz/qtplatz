// Copyright (C) 2020 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.0
import QtQuick3D 1.15
import LineGeometry 1.0

Node {
    id: pivotLine

    property alias startPos: lineGeometry.startPos
    property alias endPos: lineGeometry.endPos
    property alias name: lineGeometry.name // Name must be unique for each line
    property alias color: lineMat.diffuseColor

    Model {
        geometry: LineGeometry {
            id: lineGeometry
        }
        materials: [
            DefaultMaterial {
                id: lineMat
                lighting: DefaultMaterial.NoLighting
                cullMode: Material.NoCulling
            }
        ]
    }
}
