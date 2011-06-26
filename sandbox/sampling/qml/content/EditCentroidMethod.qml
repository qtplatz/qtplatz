import QtQuick 1.0

Rectangle {
    width:  400; height: 300

    ScanType {
        id: scanType
        width: parent.width; height: 95
        state: "scanTypeTof"

        onStateChanged: {
            console.debug( "state:" + state )
        }
    }

    VisualDataModel {
        id: scanTypeTof
        model: ListModel {
            ListElement { name: "Peak width[Da]" }
            ListElement { name: "at m/z[Da]" }
        }
        delegate: Rectangle {
            height: 25; width: parent.width
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
            Text { text: name }
        }
    }

    VisualDataModel {
        id: centroidModel
        model: ListModel {
            ListElement { name: "Area/Height" }
            ListElement { name: "Peak Centroid Fraction [%]" }
            ListElement { name: "Baseline width[Da]" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            Text { text: name }
        }
    }

    Rectangle {
        id: scanTypeDetail
        width: parent.width; height: 50
        anchors.top: scanType.bottom
        color: "lightgray"
        ListView {
            anchors.fill: parent
            width: parent.width; height: parent.height
            model: scanTypeTof
        }
    }
    Rectangle {
        width: parent.width; height: parent.height - scanType.height - scanTypeDetail.height
        anchors.top: scanTypeDetail.bottom
        color: "lightgray"
        ListView {
            anchors.fill: parent
            model: centroidModel
        }
    }


}
