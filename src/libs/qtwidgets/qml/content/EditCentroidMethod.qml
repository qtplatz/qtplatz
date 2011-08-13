import QtQuick 1.0
import QtDesktop 0.1
import com.scienceliaison.qml 1.0

Rectangle {
    width:  400; height: 300
    anchors.topMargin: 16

    Column {
        width:  parent.width; height: parent.height
        spacing: 4

        Row {
            CaptionText { text: "Scan Type: " }
            ButtonRow {
                exclusive: true
                spacing: 2
                CheckBox { id: cbPropo; property int scanType: CentroidModel.ScanTypeProportional; width: 100; text: "Proportional" }
                CheckBox { id: cbConst; property int scanType: CentroidModel.ScanTypeConstant; width: 100; text: "Constant" }
                CheckBox { id: cbTof; property int scanType: CentroidModel.ScanTypeTof; width: 100; text: "TOF" }
                checkedButton:
                    centroidModel.scanType == CentroidModel.ScanTypeProportional ?
                        cbPropo : centroidMethod.scanType == CentroidModel.ScanTypeConstant ? cbConst : cbTof
                onCheckedButtonChanged: {
                    centroidModel.scanType = checkedButton.scanType
                }
            }
        }
        Column {
            spacing: 5
            Row {
                enabled: cbTof.checked ? true : false
                CaptionText { text: "Peak Width [Da]:" }
                TextInputBox { id: item1; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                    value: centroidModel.peakwidth_tof_in_da
                    onAccepted: {
                        value = text
                    }
                }

                CaptionText { text: "at m/z:"; width: 40 }
                TextInputBox { id: item2; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                    value: centroidModel.peakwidth_tof_at_mz
                    onAccepted: {
                        value = text
                    }
                }
            }
            Row {
                enabled: cbPropo.checked ? true : false
                CaptionText { text: "Proportional [ppm]:" }
                TextInputBox { id: item3; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    value: centroidModel.peakwidth_propo_in_ppm
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }
            }
            Row {
                enabled: cbConst.checked ? true : false
                CaptionText { text: "Constant [Da]:" }
                TextInputBox { id: item4; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    value: centroidModel.peakwidth_const_in_da
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }
            }
        }

        Row {
            CaptionText { text: "Area/Height:" }
            ButtonRow {
                exclusive: true
                spacing: 2
                CheckBox { id: area; width: 100; text: "Area" }
                CheckBox { id: height; width: 100; text: "Height" }
                checkedButton: centroidModel.areaHeight == CentroidModel.Area ? area : height
                onCheckedButtonChanged: centroidModel.areaHeight = area.checked ? CentroidModel.Area : CentroidModel.Height
            }
        }

        Grid {
            columns: 2; spacing: 5

            CaptionText { text: "Peak Centroid fraction [%]:" }
            TextInputBox { id: item10; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                value: centroidModel.peak_centroid_fraction
                onAccepted: {
                    value = text
                    console.debug("TextInputBos::onAccepted: " + text)
                }
            }

            CaptionText { text: "Baseline width [Da]:" }
            TextInputBox { id: item11; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                value: centroidModel.baseline_width
                onAccepted: {
                    value = text
                    console.debug("TextInputBos::onAccepted: " + text)
                }
            }
        }
    }
}
