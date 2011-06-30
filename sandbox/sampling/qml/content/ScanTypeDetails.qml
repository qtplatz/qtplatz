import QtQuick 1.0

Rectangle {
    width: parent.width; height: parent.height

    VisualDataModel {
        id: scanTypeTof
        model: ListModel {
            ListElement { name: "Peak width[Da]"; value: "0.1" }
            ListElement { name: "at m/z[Da]"; value: "500" }
        }
        delegate: Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            EditTextItem {
            }
        }

    }

    VisualDataModel {
        id: scanTypeProportional
        model: ListModel {
            ListElement { name: "Peak width[ppm]"; value: "10" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            EditTextItem {   }
        }
    }

    VisualDataModel {
        id: scanTypeConstant
        model: ListModel {
            ListElement { name: "Peak width[Da]"; value: "1.0" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            EditTextItem {   }
        }
    }

    states: [
        State {
            name: "scanTypeProportional"
            PropertyChanges { target: scanTypeList; model: scanTypeProportional }
        },
        State {
            name: "scanTypeConstant"
            PropertyChanges { target: scanTypeList; model: scanTypeConstant  }
        }
    ]

    Rectangle {
        id: scanTypeDetailsRect
        width: parent.width; height: parent.height
        color: "lightblue"
        ListView {
            id: scanTypeList
            anchors.fill: parent
            width: parent.width; height: parent.height
            model: scanTypeTof
        }
    }
}
