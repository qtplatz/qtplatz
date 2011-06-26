import QtQuick 1.0

Rectangle {
    id: elementalCompMethod

    Text { text: "elementalCompMethod" }

    ListView {
        width:  200; height: 250
        id: view
        model: centroidModel

        delegate: Rectangle {
            width: 200; height: 25
            Text { text: name }

            ListView {
                width: 200; height: 50
                delegate: parent.value
                //Text { text: "sub:" + name }
            }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    centroidModel.peakMethod( "Constants" )
                    parent.color = 'green'
                }
            }
        }
    }
}
