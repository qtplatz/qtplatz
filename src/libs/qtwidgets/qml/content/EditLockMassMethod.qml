import QtQuick 1.0

Rectangle {
    id: lockmassMethod

    Column {
        width: parent.width; height: parent.height

        TitleText { width: parent.width; title: "Lock Mass" }

        Rectangle {
            width: parent.width; height: 200
            TextInputBox { value: "To be added..." }
        }
    }

}
