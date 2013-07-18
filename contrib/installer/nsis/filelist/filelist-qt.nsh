

    ; LIST OF FILES INCLUDED IN RELEASE BUILD
   
    !define InQtBinPath      "${InQtPath}\bin"
    !define InQtPlatforms    "${InQtPath}\plugins\platforms"

;;---- plugins
    ${InstallFile} "${InQtPlatforms}"    "qminimal.dll"      "plugins\platforms" 0 0
    ${InstallFile} "${InQtPlatforms}"    "qoffscreen.dll"    "plugins\platforms" 0 0
    ${InstallFile} "${InQtPlatforms}"    "qwindows.dll"      "plugins\platforms" 0 0
   
    ${InstallFile} "${InQtBinPath}"      "libEGL.dll"            "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "libGLESv2.dll"         "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "icuin51.dll"           "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "icuuc51.dll"           "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "icudt51.dll"           "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5CLucene.dll"        "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Core.dll"           "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Gui.dll"            "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Help.dll"           "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Multimedia.dll"     "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Network.dll"        "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5OpenGL.dll"         "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5PrintSupport.dll"   "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Script.dll"         "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Sql.dll"            "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Svg.dll"            "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Widgets.dll"        "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5Xml.dll"            "bin" 0 0
    ${InstallFile} "${InQtBinPath}"      "Qt5XmlPatterns.dll"    "bin" 0 0
