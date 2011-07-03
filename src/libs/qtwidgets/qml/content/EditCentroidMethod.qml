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

    Column {
        width:  parent.width; height: parent.height
        TitleText { title: "Centroid" }

        ScanType {
            id: scanType
            width: parent.width; height: 70
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
            width: parent.width; height: 70
            //anchors.top:  scanType.bottom; anchors.topMargin: 4
        }

        Rectangle {
            id: methodDelegate
            width: parent.width; height: parent.height - scanType.height - scanTypeDetails.height
            //anchors.top: scanTypeDetails.bottom
            MouseArea {
                anchors.fill: parent
                onClicked: methodDelegate.focus = false;
            }
            Grid {
                columns: 2; spacing: 5
                anchors { top: parent.top; left: parent.left; leftMargin: 40; topMargin: 10 }

                CaptionText { text: "Area/Height:" }
                TextInputBox { id: item1; KeyNavigation.tab: item2; KeyNavigation.backtab: item3; focus: true
                    value: centroidModel.areaHeight == CentroidModel.Area ? "Area" : "Height"
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }

                CaptionText { text: "Peak Centroid fraction [%]:" }
                TextInputBox { id: item2; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                    value: centroidModel.peak_centroid_fraction
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }

                CaptionText { text: "Baseline width [Da]:" }
                TextInputBox { id: item3; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    value: centroidModel.baseline_width
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }
            }
        }
    }
}
