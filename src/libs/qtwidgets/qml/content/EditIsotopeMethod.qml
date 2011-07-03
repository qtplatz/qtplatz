import QtQuick 1.0
import com.scienceliaison.qml 1.0

Rectangle {
    id: isotopeMethod

    Column {
        width:  parent.width; height: parent.height
        TitleText { title: "Isotope"; width: parent.width }
        Rectangle {
            id: isotopeGlobalRect
            width: parent.width; height: item1.height * 3.6
            //anchors.top: title.bottom; anchors.topMargin: 10
            MouseArea {
                anchors.fill: parent
                onClicked: methodDelegate.focus = false;
            }
            Grid {
                columns: 2; spacing: 5
                anchors { top: parent.top; topMargin: 2; left: parent.left; leftMargin: 40 }

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
        // <--- end global parameter --------
        Rectangle {
            width:  parent.width; height: parent.height - isotopeGlobalRect.height
            //anchors.top: isotopeGlobalRect.bottom; anchors.topMargin: 10; anchors.left: parent.left; anchors.leftMargin: 14

            ListView {
                id: formulae
                anchors.fill: parent
                model:  isotopeModel
                header: headerDelegate
                footer: footerDelegate
                delegate: formulaDelegate
                highlight: Rectangle { color: "lightsteelblue"; radius: 5; border.color: "black" }
                focus: true
                onCurrentIndexChanged: {
                    console.log( "onCurrentIndexChanged: " + currentIndex )
                }
            }

            Component {
                id: formulaDelegate
                Item  {
                    id: delegate
                    width: delegate.ListView.view.width; height: 26
                    property color textcolor: delegate.ListView.isCurrentItem ? "red" : "black"
                    Row {
                        Text { text: formula; width: 120; color: textcolor }
                        Text { text: adduct; width: 120; color: textcolor }
                        Text { text: chargeState; width: 120; color: textcolor }
                        Text { text: amounts; width: 120; color: textcolor }
                    }
                    MouseArea {
                        anchors.fill: delegate
                        onClicked: {
                            formulae.currentIndex = index
                            console.debug( "formlae on clicked: " + index )
                        }
                    }
                }
            }
            Component {
                id: headerDelegate
                Row {
                    Text { text: "formula"; width: 120 }
                    Text { text: "adduct"; width: 120 }
                    Text { text: "charge state"; width: 120 }
                    Text { text: "relative amounts"; width: 120 }
                }
            }
            Component {
                id: footerDelegate
                Item {
                    property string formula: isotopeModel.data( formulaDelegate.ListView.currentIndex )
                    property string adduct: "H"
                    property int chargeState: 1
                    property double amounts: 1.0
                    Row {
                        TextInputBox { value: formula; width: 120 }
                        TextInputBox { value: adduct; width: 120 }
                        TextInputBox { value: chargeState; width: 120 }
                        TextInputBox { value: amounts; width: 120 }
                        Rectangle {
                            id: button
                            width: 40; height:  24
                            border.color: "blue"
                            color: "lightblue"
                            Text { text: "Add"; color: "blue"; anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter } }
                            MouseArea {
                                anchors.fill: parent
                                onClicked: {
                                    console.debug( "formulae currentIndex: " + formulae.currentIndex )
                                    console.debug( "isotopeMethod " + isotopeModel.resolution )
                                    isotopeMethod.appendRow()
                                }
                            }
                        }
                    }
                }
            }

        }
    }
}
