import QtQuick 1.0

Rectangle {
    id: elementalCompMethod

    function scanTypeName( x ) {
        if ( x == centroidModel.ScanTypeProportional )
            return "Proportional"
        else if ( x == centroidModel.ScanTypeConstant )
            return "Constant"
        else
            return "Tof"
    }

    Text { text: "elementalCompMethod: " + scanTypeName( centroidModel.scanType ) }

    ListView {
        width:  200; height: 250
        id: view
    }
}
