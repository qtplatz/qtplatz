import QtQuick 1.0
import QtDesktop 0.1
import com.scienceliaison.qml 1.0

Rectangle {
    id: elementalCompMethod

    Column {
        width: parent.width; height: parent.height
        spacing: 2
        Column {
            id: globalScope

            Row {
                spacing: 4
                CaptionText { text: "Polynomial [degree]:" }
                SpinBox {
                    minimumValue: 1
                    maximumValue: 5
                    singleStep: 1
                    width: 60
                    value: 3
                }

                CaptionText { text: "Mass Tolerance [Da]:" }
                SpinBox {
                    minimumValue: 0.001
                    maximumValue: 10.000
                    value: 0.2
                    singleStep: 0.1
                }
            }
            Row {
                spacing: 4
                CaptionText { text: "Mass range [Da]:" }
                CaptionText { text: "Low"; width: 80 }
                SpinBox {
                    minimumValue: 1
                    maximumValue: 10000
                    value: 100
                    singleStep: 1
                }
                CaptionText { text: "High"; width: 80 }
                SpinBox {
                    minimumValue: 1
                    maximumValue: 10000
                    value: 1000
                    singleStep: 1
                }
            }
        }
        //--------------------- Table ---------------------
        Column {
            anchors.top: globalScope.bottom
            anchors.topMargin: 6
            anchors.leftMargin: 8
            width: parent.width
            height: parent.height - globalScope.height - 16
            enabled: true
            Row {
                anchors.left: parent.left; anchors.leftMargin: 8
                id: header; height: 16
                Text { text: "Mass Reference" }
            }
            TableView {
                id: table
                width: parent.width - 16
                height: parent.height - footer.height
                model: isotopeModel
                anchors.left: parent.left
                anchors.leftMargin: 8
                focus: true

                TableColumn {  property: "formula"; caption: "Formula"; width: 120 }
                TableColumn {  property: "mass";    caption: "mass"; width: 120 }
                TableColumn {  property: "Maximum"; caption: "Maximum"; width: 120 }

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
                height: 32
                Button { text: "+"; onClicked: isotopeModel.appendRow( table.currentIndex ) }
                Button { text: "-"; onClicked: isotopeModel.removeRow( table.currentIndex ) }
            }
        }
        //--------------- end of Composition Constrains --------------
    }

}


