import QtQuick 1.0

Rectangle {
    id: elementalCompMethod

    Column {
        width: parent.width; height: parent.height

        TitleText { width: parent.width; title: "Elemental Composition" }

        Rectangle {
            width: parent.width; height: 200
            TextInputBox { value: "To be added..." }
        }
    }

}
