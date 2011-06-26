import QtQuick 1.0

Rectangle {
    width:  400; height: 300

    Grid {
        id: grid1
        x: 18
        y: 12
        width: 400
        height: 400

        Text {
            id: text1
            x: 0
            y: 157
            text: centroidModel.scanType()
            font.pixelSize: 12
        }

        TextEdit {
            id: text_edit1
            x: 63
            y: 0
            width: 80
            height: 20
            text: "textEdit"
            font.pixelSize: 12
        }
    }


}
