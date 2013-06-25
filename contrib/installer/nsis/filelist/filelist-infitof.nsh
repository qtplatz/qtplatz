   ; LIST OF FILES INCLUDED IN RELEASE BUILD
   
   ${InstallFile} "${InBinPath}" "qtplatz.exe" bin 1 0

   ;; plugins
   ${InstallFile} "${InMSCheminfoPath}" "infitof.dll"             "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "infitof.pluginspec"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ;;;
   ;; adplugins
   ${InstallFile} "${InMSCheminfoPath}" "infirawfile.dll"          "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "infirawfile.adplugin"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
