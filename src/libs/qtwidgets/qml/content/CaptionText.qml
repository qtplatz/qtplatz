import QtQuick 1.0

Text {
    width: 160;
    height: parent.height
    id: caption
    font.pointSize: 12
    text: name + ":"
    horizontalAlignment: Text.AlignRight
    verticalAlignment: Text.AlignVCenter
}

