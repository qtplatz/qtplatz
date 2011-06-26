import QtQuick 1.0

ListView {
    id: view
    width: 300; height: 400

    model: VisualDataModel {
        model: dirModel

        delegate: Rectangle {
            width: 200; height: 25
            Text { text: filePath }

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if ( model.hasModelChildren )
                        view.model.rootIndex = view.model.modelIndex( index )
                }
            }
        }
    }
}
