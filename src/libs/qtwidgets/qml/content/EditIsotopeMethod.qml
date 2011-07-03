import QtQuick 1.0

Rectangle {
    id: isotopeMethod

    Column {
        width:  parent.width; height: parent.height
        TitleText { title: "Isotope"; width: parent.width }
        Rectangle {
            id: isotopeGlobalRect
            width: parent.width; height: 140
            anchors.top: title.bottom; anchors.topMargin: 20
            MouseArea {
                anchors.fill: parent
                onClicked: methodDelegate.focus = false;
            }
            Grid {
                columns: 2; spacing: 5
                anchors { top: parent.top; horizontalCenter: parent.horizontalCenter }

                CaptionText { text: "Polarity:" }
                TextInputBox { id: item1; KeyNavigation.tab: item2; KeyNavigation.backtab: item3; focus: true
                    value: isotopeModel.polarityPositive ? "Positive" : "Negative"
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }

                CaptionText { text: "Use Electron Mass:" }
                TextInputBox { id: item2; KeyNavigation.tab: item3; KeyNavigation.backtab: item1
                    value: isotopeModel.useElectronMass
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }

                CaptionText { text: "Resolution [Da]:" }
                TextInputBox { id: item3; KeyNavigation.tab: item1; KeyNavigation.backtab: item2
                    value: isotopeModel.resolution
                    onAccepted: {
                        value = text
                        console.debug("TextInputBos::onAccepted: " + text)
                    }
                }
            }
        }
        Rectangle {
            width: parent.width; height: isotopeGlobalRect.height
            anchors.top: isotopeGlobalRect.bottom; anchors.topMargin: 10
            Row {
                id: header
                width: parent.width; height: 24
                Text { text: "formula" } Text { text: "adduct" } Text { text: "charge" }
            }
            Repeater {
                model:  isotopeModel
                anchors.top: header.bottom
                width: parent.width; height: isotopeMethod.height - isotopeGlobalRect.height - header.height
                delegate: Row {
                    width: parent.width
                    Text { text: formula }
                    Text { text: adduct }
                    Text { text: chargeState }
                }
            }
        }

    }
}
