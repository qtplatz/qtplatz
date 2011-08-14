import QtQuick 1.0
import QtDesktop 0.1
import "content"
import com.scienceliaison.qml 1.0

Rectangle {
    id: window

    SystemPalette { id: syspal }
    QStyleItem { id: styleitem }
    color: syspal.window

    ToolBar {
        id: toolbar
        width: parent.width
        height: 20
        Row {
            spacing: 2
            anchors.verticalCenter: parent.verticalCenter
            ToolButton{
                iconSource: "qrc:files/images/folder_new.png"
                anchors.verticalCenter: parent.verticalCenter
            }
        }
        CheckBox {
            id: enabledCheck
            text: "Enabled"
            checked: true
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    TabFrame {
        id:frame
        focus:true
        position: "North"
        tabbar: TabBar { parent: frame; focus:true; KeyNavigation.tab: button1
            //tabHSpace: 200
            //tabWidth: 200
        }

        property int margins : styleitem.style == "mac" ? 8 : 0
        anchors.top: toolbar.bottom
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.left
        anchors.margins: margins


        Tab {
            id: tabCentroid
            title: "Centroid"
            EditCentroidMethod {
                anchors.fill: parent
                id: centroidMethod
            }
        }
        Tab {
            id: tabIsotope
            title: "Isotope"
            EditIsotopeMethod {
                anchors.fill: parent
                anchors.topMargin: 16
                id: isotopeMethod
            }
        }
        Tab {
            id: tabMsCalib
            title: "MS Calib"
            EditMSCalibMethod {
                id: msCalibMethod
                anchors.fill: parent
                anchors.topMargin: 16
            }
        }
        Tab {
            id: tabElementalComp
            title: "Elemental Comp."
            EditElementalCompMethod {
                id: elementalCompMethod
                anchors.fill: parent
                anchors.topMargin: 16
            }
        }
        Tab {
            id: tabTarget
            title: "Target"
            EditTargetMethod {
                id: targetMethod
                anchors.fill: parent
            }
        }
        Tab {
            id: tabLockmass
            title: "Lock mass"
            EditLockMassMethod {
                id: lockMassMethod
                anchors.fill: parent
            }
        }
    }

}
