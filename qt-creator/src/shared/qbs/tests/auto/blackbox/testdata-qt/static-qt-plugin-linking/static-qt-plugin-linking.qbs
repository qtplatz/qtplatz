Product {
    name: "p"
    Probe {
        id: staticQtChecker
        property bool staticQt: Qt.core.staticBuild
        configure: {
            found = staticQt;
            if (found)
                console.info("Qt is static");
        }
    }

    Group {
        condition: type.includes("application")
        files: "main.cpp"
    }

    Group {
        condition: type.includes("staticlibrary")
        files: "lib.cpp"
    }

    Depends { name: "Qt.core" }
    Depends { name: "Qt.gui" }
    Depends { name: "Qt.qminimal"; condition: Qt.core.staticBuild }
}
