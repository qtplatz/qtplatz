import QtQuick 1.0

Rectangle {
    id: targetMethod
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#ffffff"
        }

        GradientStop {
            position: 1
            color: "#000000"
        }
    }
    border.color: "#000000"
    opacity: 0.8
    smooth: true
    Text {
        x: 12
        y: 0
        text: "Target Method"
        style: Text.Sunken
        font.pointSize: 24
        opacity: 0.5
        font.family: "Vivaldi"
        horizontalAlignment: Text.AlignHCenter
    }
}
