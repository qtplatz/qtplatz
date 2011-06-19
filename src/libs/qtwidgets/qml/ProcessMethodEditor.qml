import QtQuick 1.0
import "content"

Rectangle {
//    width: 100
 //   height: 62
    id: window

    gradient: Gradient {
        GradientStop {
            position: 0.00;
            color: "#000000";
        }
        GradientStop {
            position: 0.95;
            color: "#ffffff";
        }
    }

    ListModel {
        id: methodModel

        ListElement { name: "Centroid" }
        ListElement { name: "MS Calibration" }
        ListElement { name: "Elemental Comp" }
        ListElement { name: "Isotope" }
        ListElement { name: "Targeting" }
        ListElement { name: "Lock mass" }
        ListElement { name: "Chromatogram" }
        ListElement { name: "Peak Id" }
        ListElement { name: "Report" }
    }

    ListModel {
        id: editModel
        ListElement { name: "Centroid" }
        ListElement { name: "MS Calibration" }
        ListElement { name: "Elemental Comp" }
        ListElement { name: "Isotope" }
        ListElement { name: "Targeting" }
        ListElement { name: "Lock mass" }
        ListElement { name: "Chromatogram" }
        ListElement { name: "Peak Id" }
        ListElement { name: "Report" }
    }

    Row {
        Rectangle {
            width: 200; height: window.height
            color: "#efefef"
            ListView {
                id: categories
                anchors.fill: parent
                model: methodModel
                footer: quitButtonDelegate
                delegate: CategoryDelegate {}
                highlight: Rectangle { color: "steelblue" }
                highlightMoveSpeed: 9999999
            }
            ScrollBar {
                scrollArea: categories; height: categories.height; width: 8
                anchors.right: categories.right
            }
        }
        ListView {
            id: list
            width: window.width - 200; height: window.heith
            model: editModel
        }
    }

    ScrollBar { scrollArea: list; height: list.height; width: 8; anchors.right: window.right }
    Rectangle { x: 200; height: window.height; width: 1; color: "#cccccc" }

}
