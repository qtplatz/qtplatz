import QtQuick 1.0
import com.scienceliaison.qml 1.0

Rectangle {
    id: page
    width: 500; height: 250
    color: "#edecec"


    MouseArea {
        anchors.fill: parent
        onClicked: page.focus = false;
    }
    Grid {
        columns: 2
        spacing: 10
        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }

        Text { text: "Area/Height" }
        TextInputBox { id: item1; property string name: "Area/Height"; property string value: "value"
            KeyNavigation.tab: item2; KeyNavigation.backtab: item3; focus: true
        }
        Text { text: "Area/Height" }
        TextInputBox { id: item2; property string name: "Area/Height"; property string value: "value"
            KeyNavigation.tab: item3; KeyNavigation.backtab: item1; focus: true
        }
        Text { text: "Area/Height" }
        TextInputBox { id: item3; property string name: "Area/Height"; property string value: "value"
            KeyNavigation.tab: item1; KeyNavigation.backtab: item2; focus: true
        }
        //TextInputBox { id: search1; KeyNavigation.tab: search2; KeyNavigation.backtab: search3 }
        //TextInputBox { id: search2; KeyNavigation.tab: search3; KeyNavigation.backtab: search1 }
        //TextInputBox { id: search3; KeyNavigation.tab: search1; KeyNavigation.backtab: search2 }
    }
}

