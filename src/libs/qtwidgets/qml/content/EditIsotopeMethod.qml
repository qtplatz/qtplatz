import QtQuick 1.0
import QtDesktop 0.1
import com.scienceliaison.qml 1.0

Rectangle {
    id: isotopeMethod

    Column {
        width:  parent.width; height: parent.height
        Column {
            id: isotopeGlobalRect
            spacing: 4
            Row {
                CaptionText { text: "Polarity: " }
                ButtonRow {
                    exclusive: true
                    CheckBox { id: positive; text: "Positive: " }
                    CheckBox { id: negative; text: "Negative: " }
                    checkedButton: isotopeModel.polarityPositive ? positive : negative
                    onCheckedButtonChanged: isotopeModel.polarityPostive = positive.checked ? true : false
                }
            }

            Row {
                spacing: 8
                CaptionText { text: "Resolution [Da]:" }
                SpinBox {
                    singleStep: 0.01
                    value: isotopeModel.resolution
                    onValueChanged: isotopeModel.resolution = value
                }
                CheckBox {
                    text: "Use Electron Mass:"
                    checked: isotopeModel.useElectronMass ? true : false
                    onCheckedChanged: isotopeModel.useElectronMass = checked
                }
            }
        }
        // <--- end global parameter --------
        Column {
            anchors.top: isotopeGlobalRect.bottom
            anchors.topMargin: 6
            width: parent.width
            height: parent.height - isotopeGlobalRect.height
            enabled: true
            TableView {
                id: table
                width: parent.width - 16
                height: parent.height - footer.height
                model: isotopeModel
                //enabled: linear.checked ? false : true
                focus: true

                TableColumn {  property: "formula";     caption: "Formula"; width: 120 }
                TableColumn {  property: "adduct";      caption: "adduct"; width: 120 }
                TableColumn {  property: "chargeState"; caption: "charge"; width: 120 }
                TableColumn {  property: "amounts";     caption: "amounts"; width: 120 }

                itemDelegate:
                    TextInput {
                        width: parent.width
                        anchors.margins: 4
                        anchors.left: parent.left
                        anchors.verticalCenter: parent.verticalCenter
                        text: itemValue ? itemValue : ""
                        color: itemSelected ? "lightcyan" : "navy"

                        onFocusChanged: {
                            table.currentIndex = rowIndex
                        }
                        onAccepted: {
                            analyzerModel.segments.setProperty( rowIndex, itemProperty, text )
                        }
                }
                onCurrentIndexChanged: {
                    console.debug( "currentIndex: " + currentIndex )
                }
            }

            Row {
                id: footer
                width: parent.width
                height: 40
                Button { text: "+"; onClicked: isotopeModel.appendRow( table.currentIndex ) }
                Button { text: "-"; onClicked: isotopeModel.removeRow( table.currentIndex ) }
            }
        }
        //------------------------------------
    }
}
