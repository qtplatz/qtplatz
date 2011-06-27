import QtQuick 1.0

Rectangle {
    width: parent.width; height: parent.height

    VisualDataModel {
        id: scanTypeTof
        model: ListModel {
            ListElement { name: "Peak width[Da]" }
            ListElement { name: "at m/z[Da]" }
        }
        delegate: Rectangle {
            height: 20; width: parent.width
            color: scanTypeDetailsRect.color
            Text { text: name }
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

            Text { text: name }
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
            Text { text: name }
        }
    }

    states: [
        // In state 'scanTypeProportional', move the image to center
        State {
            name: "scanTypeProportional"
            PropertyChanges { target: scanTypeDetailsRect.scanTypeList; model: scanTypeProportional }
        },

        // In state 'scanTypeConstant', move the image to right
        State {
            name: "scanTypeConstant"
            PropertyChanges { target: scanTypeDetailsrect.scanTypeList; model: scanTypeConstant  }
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
