   ; LIST OF FILES INCLUDED IN RELEASE BUILD
   
   !define InBinPath          "${InQtPlatz}\bin"
   !define InQMLPath          "${InQtPlatz}\share"
   !define InLibPath          "${InQtPlatz}\lib\qtplatz"
   !define InQtProjectPath    "${InQtPlatz}\lib\qtplatz\plugins\QtProject"
   !define InMSCheminfoPath   "${InQtPlatz}\lib\qtPlatz\plugins\MS-Cheminformatics"
   !define InXSDPath          "${InQtPlatz}\src\plugins\xsd"
   
   ;* The executable and ancilliary files we're installing *
   ; ${InstallFile} InFilePath FileName OutSubDirectory IfOverWrite IfRegister
   ;; qtplatz library modules
   ${InstallFile} "${InBinPath}"    "Aggregation.dll"        "bin" 1 0
   ${InstallFile} "${InBinPath}"    "ExtensionSystem.dll"    "bin" 1 0
   ${InstallFile} "${InBinPath}"    "Utils.dll"              "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adchem.dll"             "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adcontrols.dll"         "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adextension.dll"        "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adorbmgr.dll"           "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adplugin.dll"           "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adsequence.dll"         "bin" 1 0
   ${InstallFile} "${InBinPath}"    "chromatogr.dll"         "bin" 1 0
   ${InstallFile} "${InBinPath}"    "portfolio.dll"          "bin" 1 0
   ;;;;;
   ${InstallFile} "${InBinPath}" "qtplatz.exe" bin 1 0
   ;;
   ;; plugins
   ${InstallFile} "${InQtProjectPath}"   "Core.dll"              "lib\qtplatz\plugins\QtProject" 1 0
   ${InstallFile} "${InQtProjectPath}"   "Core.pluginspec"       "lib\qtplatz\plugins\QtProject" 1 0
   ;;
   ${InstallFile} "${InMSCheminfoPath}" "acquire.dll"             "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "batchproc.dll"           "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "chemistry.dll"           "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "dataproc.dll"            "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "sequence.dll"            "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "servant.dll"             "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "qtwidgets.dll"           "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "qtwidgets2.dll"          "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ;;;
   ;; adplugins
   ${InstallFile} "${InMSCheminfoPath}" "adtextfile.dll"          "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "addatafile.dll"          "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "fticr.dll"               "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ;;
   ${InstallFile} "${InMSCheminfoPath}" "acquire.pluginspec"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "batchproc.pluginspec"    "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "chemistry.pluginspec"    "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "dataproc.pluginspec"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "sequence.pluginspec"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "servant.pluginspec"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
;;
   ${InstallFile} "${InMSCheminfoPath}" "acquire.config"          "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "dataproc.config"         "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InXSDPath}"        "config.xsd"              "lib\qtplatz\plugins\xsd"            1 0
;;;
   ${InstallFile} "${InMSCheminfoPath}" "adbroker.adplugin"       "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "adcontroller.adplugin"   "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "addatafile.adplugin"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "adtextfile.adplugin"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "fticr.adplugin"          "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "infirawfile.adplugin"    "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "qtwidgets.adplugin"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "qtwidgets2.adplugin"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ;;
   ;;;;;
   ;; qtplatz -- CORBA Servants
   ${InstallFile} "${InMSCheminfoPath}"    "adbroker.dll"           "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}"    "adcontroller.dll"       "lib\qtplatz\plugins\MS-Cheminformatics" 1 0

   ;
   ; licence
   !define RDBASE $%RDBASE%
   ${InstallFile} "${RDBASE}"    "license.txt"                 "rdkit" 1 0
