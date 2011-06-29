import QtQuick 1.0
import com.scienceliaison.qml 1.0

Rectangle {
    width:  400; height: 300

    function scanTypeState() {
        console.log( "function scanTypeState():" + centroidModel.scanType )
        if ( centroidModel.scanType  == CentroidModel.ScanTypeProportional ) {
            return 'scanTypeProportional'
        } else if ( centroidModel.scanType == CentroidModel.ScanTypeConstant ) {
            return 'scanTypeConstant'
        } else {
            return ''
        }
    }

    ScanType {
        id: scanType
        width: parent.width; height: 95
        state: scanTypeState()
        onStateChanged: {
            scanTypeDetails.state = state

            if ( state == 'scanTypeProportional' )
                centroidModel.scanType = CentroidModel.ScanTypeProportional
            else if ( state == 'scanTypeConstant' )
                centroidModel.scanType = CentroidModel.ScanTypeConstant
            else
                centroidModel.scanType = CentroidModel.ScanTypeTof
        }
    }

    ScanTypeDetails {
        id: scanTypeDetails
        width: parent.width; height: 60
        anchors.top:  scanType.bottom
    }

    VisualDataModel {
        id: centroidListModel
        model: ListModel {
            ListElement { name: "Area/Height"; value: "Area" }
            ListElement { name: "Peak Centroid Fraction [%]"; value: "50" }
            ListElement { name: "Baseline width[Da]"; value: "500" }
        }
        delegate:  Rectangle {
            height: 20; width: parent.width
            EditTextItem {   }
        }
    }

    Rectangle {
        id: methodDelete
        width: parent.width; height: parent.height - scanType.height - scanTypeDetails.height
        anchors.top: scanTypeDetails.bottom
        ListView {
            anchors.fill: parent
            model: centroidListModel
        }
    }


}
