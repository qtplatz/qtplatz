import QtQuick 1.0

Rectangle {
    width: parent.width
    height: parent.height
    color: "#9f9f9f"

    border.color: "#000000"
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
        spacing: 20
        Text {
            id: caption
            anchors.verticalCenter: parent.verticalCenter
            font.pointSize: 10
            text: parent.parent.caption + ":"
            horizontalAlignment: Text.AlignRight
        }
        TextEdit {
            id: value
            font.pointSize: 10
            text: parent.parent.caption
            horizontalAlignment: Text.AlignLeft
        }
    }
}
