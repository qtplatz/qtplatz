import QtQuick 1.0

Rectangle {
    id: page
    width: 500; height: 250
    color: "#edecec"

    MouseArea {
        anchors.fill: parent
        onClicked: {
            page.focus = false;
            controller.fake()
        }
    }
    Column {
        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
        spacing: 10

        SearchBox { id: search1; KeyNavigation.tab: search2; KeyNavigation.backtab: search3; focus: true }
        SearchBox { id: search2; KeyNavigation.tab: search3; KeyNavigation.backtab: search1 }
        SearchBox { id: search3; KeyNavigation.tab: search1; KeyNavigation.backtab: search2 }
    }
}

