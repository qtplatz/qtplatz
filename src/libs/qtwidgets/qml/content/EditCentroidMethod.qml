import QtQuick 1.0

Rectangle {
    id: centroidMethod
    width: 400
    height: 200
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#ffffff"
        }

        GradientStop {
            position: 1
            color: "#000000"
        }
    }
    border.color: "#000000"
    opacity: 0.8
    smooth: true
    Text {
        x: 12
        y: 0
        text: "Centroid Method"
        style: Text.Sunken
        font.pointSize: 24
        opacity: 0.5
        font.family: "Vivaldi"
        horizontalAlignment: Text.AlignHCenter
    }
    /*
    ListModel {
        id: itemModel
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
    ListView {
                anchors.fill: parent
                model: itemModel
                footer: applyButtonDelegate
                delegate: CategoryDelegate {}
                highlight: Rectangle { color: "steelblue" }
                highlightMoveSpeed: 999999
                onCurrentIndexChanged: {
                    console.log( "onCurrentIndexChanged " + currentIndex )
                    editListView.currentIndex = currentIndex
                }
            }
    }
    */
}
