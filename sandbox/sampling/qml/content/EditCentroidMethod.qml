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

    ScanTypeDetails {
        id: scanTypeDetails
        width: parent.width; height: 60
        anchors.top:  scanType.bottom
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
            color: methodDetail.color
            Text { text: name }
        }
    }

    Rectangle {
        id: methodDetail
        width: parent.width; height: parent.height - scanType.height - scanTypeDetails.height
        anchors.top: scanTypeDetails.bottom
        color: "lightgray"
        ListView {
            anchors.fill: parent
            model: centroidModel
        }
    }


}
