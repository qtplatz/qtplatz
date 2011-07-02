import QtQuick 1.0

Rectangle {
    width: parent.width; height: parent.height
    states: [
        State {
            name: "scanTypeProportional"
            PropertyChanges { target: scanTypeView; currentIndex: 1 }
        },
        State {
            name: "scanTypeConstant"
            PropertyChanges { target: scanTypeView; currentIndex: 2 }
        }
    ]

    VisualItemModel {
        id: scanTypeModel
        Rectangle {
            width: scanTypeView.width; height: scanTypeView.height
            Grid {
                columns: 2; spacing: 5
                CaptionText { text: "Peak width[Da]" }
                TextInputBox { id: item1; KeyNavigation.tab: item2; KeyNavigation.backtab: item1
                    value: centroidModel.peakwidth_tof_in_da
                }
                CaptionText { text: "at m/z [Da]" }
                TextInputBox { id: item2; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    value: centroidModel.peakwidth_tof_at_mz
                }
            }
        }
        Rectangle {
            width: scanTypeView.width; height: scanTypeView.height
            Grid {
                columns: 2; spacing: 5
                width: scanTypeView.width; height: scanTypeView.height
                CaptionText { text: "Peak width[ppm]" }
                TextInputBox { id: item3; KeyNavigation.tab: item2; KeyNavigation.backtab: item3
                    value: centroidModel.peakwidth_propo_in_ppm
                }
            }
        }
        Rectangle {
            width: scanTypeView.width; height: scanTypeView.height
            Grid {
                columns: 2; spacing: 5
                width: scanTypeView.width; height: scanTypeView.height
                CaptionText { text: "Peak width[Da]" }
                TextInputBox { id: item4; KeyNavigation.tab: item4; KeyNavigation.backtab: item4
                    value: centroidModel.peakwidth_const_in_da
                }
            }
        }
    }
    ListView {
        width: parent.width; height: parent.height
        id: scanTypeView
        anchors { fill: parent; bottomMargin: 2 }
        model: scanTypeModel
        preferredHighlightBegin: 0; preferredHighlightEnd: 0
        highlightRangeMode: ListView.StrictlyEnforceRange
        orientation: ListView.Horizontal
        snapMode: ListView.SnapOneItem; flickDeceleration: 500
        cacheBuffer: 0
        clip: true
    }
}
