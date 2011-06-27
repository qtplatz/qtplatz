import QtQuick 1.0

Rectangle {
    width: parent.width; height: parent.height

    VisualDataModel {
        id: scanTypeTof
        model: ListModel {
            ListElement { name: "Peak width[Da]"; property string value: "0.1" }
            ListElement { name: "at m/z[Da]"; property string value: "500" }
        }
        delegate: Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            EditTextItem {
                property string caption: name
                property string value: value
            }
        }
    }

    VisualDataModel {
        id: scanTypeProportional
        model: ListModel {
            ListElement { name: "Peak width[ppm]" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            EditTextItem {
                property string caption: name
                property string value: value
            }
        }
    }

    VisualDataModel {
        id: scanTypeConstant
        model: ListModel {
            ListElement { name: "Peak width[Da]" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            EditTextItem {
                property string caption: name
                property string value: value
            }
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
