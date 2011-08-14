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
                CaptionText { text: "Mass" }
                SpinBox {
                    width: 120
                }
            }

            Row {
                spacing: 4
                CaptionText { text: "Electron Mode" }
                ButtonRow {
                    exclusive: true
                    CheckBox { text: "Even" }
                    CheckBox { text: "Odd"  }
                    CheckBox { text: "Odd/Even" }
                }
            }
            Row {
                spacing: 4
                CaptionText { text: "Electron Mode:" }
                CheckBox { id: cbx; text: "in ppm" }

                CaptionText { text: "mDa"; width: 32; enabled: cbx.checked ? false : true }
                SpinBox { enabled: cbx.checked ? false : true    }

                CaptionText { text: "ppm"; width: 32; enabled: cbx.checked }
                SpinBox { enabled: cbx.checked  }

            }
            Row {
                spacing: 4
                CaptionText { text: "Double Bound Equivalent:" }
                CaptionText { text: "Minimum"; width: 60 }
                SpinBox {
                }
                CaptionText { text: "Maximum"; width: 60 }
                SpinBox {
                }
            }
        }
        //--------------------- Table ---------------------
        Column {
            anchors.top: globalScope.bottom
            anchors.topMargin: 6
            anchors.leftMargin: 8
            width: parent.width
            height: parent.height - globalScope.height
            enabled: true
            Row {
                anchors.left: parent.left; anchors.leftMargin: 8
                id: header; height: 16
                Text { text: "Composition Constrains" }
            }
            TableView {
                id: table
                width: parent.width - 16
                height: parent.height - footer.height
                model: isotopeModel
                anchors.left: parent.left
                anchors.leftMargin: 8
                focus: true

                TableColumn {  property: "Atom";     caption: "Atom"; width: 120 }
                TableColumn {  property: "Minimum";      caption: "Minimum"; width: 120 }
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
