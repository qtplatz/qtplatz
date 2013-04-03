

   ; LIST OF FILES INCLUDED IN RELEASE BUILD
   
;;   !define InQtPath         "\Qt\4.8.0"
   !define InQtBinPath      "${InQtPath}\bin"
   !define InQtDesktopPath  "${InQtPath}\imports\QtDesktop"
   !define InQtWebkitPath   "${InQtPath}\imports\QtWebKit"
   
   ${InstallFile} "${InQtBinPath}"      "QtCLucene4.dll"        "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtCore4.dll"           "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtDeclarative4.dll"    "bin" 0 0
;  ${InstallFile} "${InQtBinPath}"      "QtDesigner4.dll"       "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtGui4.dll"            "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtHelp4.dll"           "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtMultimedia4.dll"     "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtNetwork4.dll"        "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtOpenGL4.dll"         "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtScript4.dll"         "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtScriptTools4.dll"    "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtSql4.dll"            "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtSvg4.dll"            "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtWebKit4.dll"         "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtXml4.dll"            "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "QtXmlPatterns4.dll"    "bin" 0 0
   ${InstallFile} "${InQtBinPath}"      "phonon4.dll"           "bin" 0 0

;;;;;;;;;;; QtDesktop
;;./plugin:

   ${InstallFile} "${InQtDesktopPath}\plugin"  "styleplugin.dll"     "imports\QtDesktop\plugin" 0 0

   ${InstallFile} "${InQtDesktopPath}"   "qmldir"              "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Button.qml"          "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ButtonRow.qml"       "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "CheckBox.qml"        "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ChoiceList.qml"      "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ComboBox.qml"        "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ContextMenu.qml"     "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Dial.qml"            "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Frame.qml"           "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "GroupBox.qml"        "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Menu.qml"            "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "MenuItem.qml"        "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ProgressBar.qml"     "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "RadioButton.qml"     "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ScrollArea.qml"      "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ScrollBar.qml"       "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Slider.qml"          "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "SpinBox.qml"         "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "SplitterRow.qml"     "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Switch.qml"          "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "Tab.qml"             "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "TabBar.qml"          "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "TabFrame.qml"        "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "TableColumn.qml"     "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "TableView.qml"       "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "TextArea.qml"        "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "TextField.qml"       "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ToolBar.qml"         "imports\QtDesktop" 0 0
   ${InstallFile} "${InQtDesktopPath}"   "ToolButton.qml"      "imports\QtDesktop" 0 0

   ;;;;;;;;;;;;  ./custom:
   ${InstallFile} "${InQtDesktopPath}\custom"   "qmldir"            "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "BasicButton.qml"   "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "Button.qml"        "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "ButtonColumn.qml"  "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "ButtonGroup.js"    "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "ButtonRow.qml"     "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "CheckBox.qml"      "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "ChoiceList.qml"    "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "GroupBox.qml"      "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "ProgressBar.qml"   "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "Slider.qml"        "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "SpinBox.qml"       "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "SplitterRow.qml"   "imports\QtDesktop\custom" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom"   "TextField.qml"     "imports\QtDesktop\custom" 0 0

   ${InstallFile} "${InQtDesktopPath}\custom\behaviors"   "ButtonBehavior.qml"      "imports\QtDesktop\custom\behaviors" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom\behaviors"   "ModalPopupBehavior.qml"  "imports\QtDesktop\custom\behaviors" 0 0
   ${InstallFile} "${InQtDesktopPath}\custom\private"     "ChoiceListPopup.qml"     "imports\QtDesktop\custom\private" 0 0
   ${InstallFile} "${InQtDesktopPath}\images"   "folder_new.png"      "imports\QtDesktop\images" 0 0

;;;;;;;;;;;;;;;; QtWebKit
   ${InstallFile} "${InQtWebKitPath}"   "qmldir"              "imports\QtWebKit" 0 0
   ${InstallFile} "${InQtWebKitPath}"   "qmlwebkitplugin.dll" "imports\QtWebKit" 0 0
