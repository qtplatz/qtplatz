import QtQuick 1.0

Rectangle {
    id: isotopeMethod
    Text {
        text: "Isotope Method"
    }
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
        text: "Isotope Method"
        style: Text.Sunken
        font.pointSize: 24
        opacity: 0.5
        font.family: "Vivaldi"
        horizontalAlignment: Text.AlignHCenter
    }

}
