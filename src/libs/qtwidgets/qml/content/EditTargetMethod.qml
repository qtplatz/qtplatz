import QtQuick 1.0

Rectangle {
    id: targetMethod

    Column {
        width: parent.width; height: parent.height

        TitleText { width: parent.width; title: "The Targetting" }

        Rectangle {
            width: parent.width; height: 200
            TextInputBox { value: "To be added..." }
        }
    }

}

