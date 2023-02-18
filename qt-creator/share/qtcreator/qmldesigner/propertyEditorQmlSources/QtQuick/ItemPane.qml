// Copyright (C) 2021 The Qt Company Ltd.
// SPDX-License-Identifier: LicenseRef-Qt-Commercial OR GPL-3.0 WITH Qt-GPL-exception-1.0

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import HelperWidgets 2.0
import StudioControls 1.0 as StudioControls
import StudioTheme 1.0 as StudioTheme

PropertyEditorPane {
    id: itemPane

    ComponentSection {
        showState: true
    }

    DynamicPropertiesSection {
        propertiesModel: SelectionDynamicPropertiesModel {}
        visible: !hasMultiSelection
    }

    GeometrySection {}

    Section {
        caption: qsTr("Visibility")
        anchors.left: parent.left
        anchors.right: parent.right

        SectionLayout {
            PropertyLabel { text: qsTr("Visibility") }

            SecondColumnLayout {
                CheckBox {
                    text: qsTr("Visible")
                    implicitWidth: StudioTheme.Values.twoControlColumnWidth
                                   + StudioTheme.Values.actionIndicatorWidth
                    backendValue: backendValues.visible
                }

                Spacer { implicitWidth: StudioTheme.Values.twoControlColumnGap }

                CheckBox {
                    text: qsTr("Clip")
                    implicitWidth: StudioTheme.Values.twoControlColumnWidth
                                   + StudioTheme.Values.actionIndicatorWidth
                    backendValue: backendValues.clip
                }

                ExpandingSpacer {}
            }

            PropertyLabel { text: qsTr("Opacity") }

            SecondColumnLayout {
                SpinBox {
                    implicitWidth: StudioTheme.Values.singleControlColumnWidth
                                   + StudioTheme.Values.actionIndicatorWidth
                    sliderIndicatorVisible: true
                    backendValue: backendValues.opacity
                    decimals: 2
                    minimumValue: 0
                    maximumValue: 1
                    hasSlider: true
                    stepSize: 0.1
                }

                ExpandingSpacer {}
            }
        }
    }

    Item {
        height: 4
        width: 4
    }

    StudioControls.TabBar {
        id: tabBar

        anchors.left: parent.left
        anchors.right: parent.right

        StudioControls.TabButton {
            text: backendValues.__classNamePrivateInternal.value
        }
        StudioControls.TabButton {
            text: qsTr("Layout")
        }
    }

    StackLayout {
        id: tabView
        property int currentHeight: children[tabView.currentIndex].implicitHeight
        property int extraHeight: 40

        anchors.left: parent.left
        anchors.right: parent.right
        currentIndex: tabBar.currentIndex
        height: currentHeight + extraHeight

        Column {
            width: parent.width

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

            Loader {
                id: specificsOne
                anchors.left: parent.left
                anchors.right: parent.right
                source: specificsUrl
                visible: specificsOne.source.toString() !== ""
            }

            AdvancedSection {
                expanded: false
            }

            LayerSection {
                expanded: false
            }
        }

        Column {
            width: parent.width

            LayoutSection {}

            MarginSection {
                visible: anchorBackend.isInLayout
                backendValueTopMargin: backendValues.Layout_topMargin
                backendValueBottomMargin: backendValues.Layout_bottomMargin
                backendValueLeftMargin: backendValues.Layout_leftMargin
                backendValueRightMargin: backendValues.Layout_rightMargin
                backendValueMargins: backendValues.Layout_margins
            }

            AlignDistributeSection {
                visible: !anchorBackend.isInLayout
            }
        }
    }
}
