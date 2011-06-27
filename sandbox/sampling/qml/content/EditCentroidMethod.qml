import QtQuick 1.0

Rectangle {
    width:  400; height: 300

    ScanType {
        id: scanType
        width: parent.width; height: 95
        state: "scanTypeTof"

        onStateChanged: {
            console.debug( "state:" + state )
            scanTypeDetails.state = state
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
            ListElement { name: "Area/Height"; value: "Area" }
            ListElement { name: "Peak Centroid Fraction [%]"; value: "50" }
            ListElement { name: "Baseline width[Da]"; value: "500.0" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            EditTextItem {
                property string caption: name
                //property string value: value
            }
        }
    }

    Rectangle {
        id: methodDetail
        width: parent.width; height: parent.height - scanType.height - scanTypeDetails.height
        anchors.top: scanTypeDetails.bottom
        ListView {
            anchors.fill: parent
            model: centroidModel
        }
    }


}
