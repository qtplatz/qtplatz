import QtQuick 1.0

Rectangle {
    width: parent.width
    height: parent.height
    color: "#9f9f9f"
    signal focusChanged()
    signal accepted( string text )
    signal enterd()
    signal exited()

    border.color: "#000000"
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 20
        Text {
            width: 120;
            id: caption
            font.pointSize: 10
            text: name + ":"
            horizontalAlignment: Text.AlignRight
        }
        TextInput {
            width: 60;
            id: edit
            font.pointSize: 10
            focus: true
            text: value
            horizontalAlignment: Text.AlignLeft
            onAccepted: {
                parent.parent.accepted( text )
            }
        }
    }
}
