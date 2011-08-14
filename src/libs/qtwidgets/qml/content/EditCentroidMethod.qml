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
                SpinBox {
                    id: item1
                    KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                    width: 60
                    minimumValue: 0.01
                    maximumValue: 1.00
                    singleStep: 0.1
                    value: centroidModel.peakwidth_tof_in_da
                    onValueChanged: centroidModel.peakwidth_tof_in_da = value
                }

                CaptionText { text: "at m/z:"; width: 50 }
                SpinBox { id: item2; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                    width: 60
                    minimumValue: 0
                    maximumValue: 2000
                    singleStep: 50
                    value: centroidModel.peakwidth_tof_at_mz
                    onValueChanged: centroidModel.peakwidth_tof_at_mz = value
                }
            }
            Row {
                enabled: cbPropo.checked ? true : false
                CaptionText { text: "Proportional [ppm]:" }
                SpinBox { id: item3; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    width: 60
                    minimumValue: 0
                    maximumValue: 1000
                    singleStep: 10
                    value: centroidModel.peakwidth_propo_in_ppm
                    onValueChanged: centroidModel.peakwidth_propo_in_ppm = value
                }
            }
            Row {
                enabled: cbConst.checked ? true : false
                CaptionText { text: "Constant [Da]:" }
                SpinBox { id: item4; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    width: 60
                    minimumValue: 0.01
                    maximumValue: 10.0
                    singleStep: 0.1
                    value: centroidModel.peakwidth_const_in_da
                    onValueChanged: centroidModel.peakwidth_const_in_da = value
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

        Row {
            CaptionText { text: "Peak Centroid fraction [%]:" }
            SpinBox { id: item10; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                minimumValue: 10
                maximumValue: 90
                singleStep: 10
                value: centroidModel.peak_centroid_fraction
                onValueChanged: centroidModel.peak_centroid_fraction = value
            }
        }

        Row {
            CaptionText { text: "Baseline width [Da]:" }
            SpinBox { id: item11; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                minimumValue: 20
                maximumValue: 1000
                singleStep: 10
                value: centroidModel.baseline_width
                onValueChanged: centroidModel.baseline_width = value
            }
        }
    }
}
