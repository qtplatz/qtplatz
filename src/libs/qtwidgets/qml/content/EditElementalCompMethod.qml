import QtQuick 1.0

Rectangle {
    id: elementalCompMethod
    gradient: Gradient {
        GradientStop {
            position: 0.00;
            color: "#ffffff";
        }
        GradientStop {
            position: 0.99;
            color: "#6680ff";
        }
        GradientStop {
            position: 1.00;
            color: "#000000";
        }
    }
    border.color: "#000000"
    opacity: 0.8
    smooth: true
    Text {
        x: 12
        y: 0
        text: "Elemental Composition Method"
        style: Text.Sunken
        font.pointSize: 24
        opacity: 0.5
        font.family: "Vivaldi"
        horizontalAlignment: Text.AlignHCenter
    }
}
