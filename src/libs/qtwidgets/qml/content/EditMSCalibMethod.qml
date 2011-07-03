import QtQuick 1.0

Rectangle {
    id: mscalibMethod

    Column {
        width: parent.width; height: parent.height

        TitleText { width: parent.width; title: "Mass Reference for Calibration" }

        Rectangle {
            width: parent.width; height: 200
            TextInputBox { value: "To be added..." }
        }
    }

}

