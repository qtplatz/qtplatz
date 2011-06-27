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
