import QtQuick 1.0
import "content"

Rectangle {
    id: window
    property int currentIndex: 0

    ListModel {
        id: methodModel

        ListElement { name: "Centroid" }
        ListElement { name: "Isotope" }
        ListElement { name: "MS Calibration" }
        ListElement { name: "Elemental Comp" }
        ListElement { name: "Targeting" }
        ListElement { name: "Lock mass" }
        ListElement { name: "Chromatogram" }
        ListElement { name: "Report" }
    }

    VisualItemModel {
        id: editModel

        EditCentroidMethod {
            width: editListView.width; height: editListView.height
            id: centroidMethod
        }
        EditIsotopeMethod {
            width: editListView.width; height: editListView.height
            id: isotopeMethod
        }
        EditMSCalibMethod {
            width: editListView.width; height: editListView.height
            id: msCalibMethod
        }
        EditElementalCompMethod {
            width: editListView.width; height: editListView.height
            id: elementalCompMethod
        }
        EditTargetMethod {
            width: editListView.width; height: editListView.height
            id: targetMethod
        }
        EditLockMassMethod {
            width: editListView.width; height: editListView.height
            id: lockMassMethod
        }
        EditIntegrationMethod {
            width: editListView.width; height: editListView.height
            id: integrationMethod
        }
        EditReportMethod {
            width: editListView.width; height: editListView.height
            id: reportMethod
        }
    }


    Row {
        Rectangle {
            width: 200; height: window.height
            color: "#efefef"
            ListView {
                id: categories
                anchors.fill: parent
                model: methodModel
                footer: applyButtonDelegate
                delegate: CategoryDelegate {}
                highlight: Rectangle { color: "steelblue" }
                highlightMoveSpeed: 999999
                onCurrentIndexChanged: {
                    console.log( "onCurrentIndexChanged " + currentIndex )
                    editListView.currentIndex = currentIndex
                }
            }
            ScrollBar {
                scrollArea: categories; height: categories.height; width: 8
                anchors.right: categories.right
            }
        }

        ListView {
            id: editListView
            width: window.width - 200; height: window.height
            model: editModel

            // control the movement of the menu switching
            snapMode: ListView.SnapOneItem
            orientation: ListView.Vertical
            boundsBehavior: Flickable.StopAtBounds
            flickDeceleration: 5000
            highlightMoveDuration: 240
            highlightRangeMode: ListView.StrictlyEnforceRange
        }
        function categoryChanged() {

        }
    }

    Component {
        id: applyButtonDelegate
        Item {
            width: categories.width; height: 60
            Text {
                text: "Apply"
                font { family: "Helvetica"; pixelSize: 16; bold: true }
                anchors {
                    left: parent.left; leftMargin: 15
                    verticalCenter: parent.verticalCenter
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked:  Qt.quite()
            }
        }
    }

    // ScrollBar { scrollArea: list; height: list.height; width: 8; anchors.right: window.right }
    // Rectangle { x: 200; height: window.height; width: 1; color: "#cccccc" }

}
