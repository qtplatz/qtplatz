import QtQuick 1.0

Rectangle {
    id: integrationMethod

    Column {
        width: parent.width; height: parent.height

        TitleText { width: parent.width; title: "Chromatography peak integration" }

        Rectangle {
            width: parent.width; height: 200
            TextInputBox { value: "To be added..." }
        }
    }

}
