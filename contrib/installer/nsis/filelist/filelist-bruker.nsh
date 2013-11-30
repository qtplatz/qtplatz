
    !define InBrukerPath      "C:\Program Files (x86)\Bruker Daltonik\3.1.2"
   
    ${InstallFile} "${InBrukerPath}"    "Bruker Daltonics CompassXtract.msi"    "Bruker\3.1.2" 0 0
    ${InstallFile} "${InBrukerPath}"    "Data1.cab"                             "Bruker\3.1.2" 0 0
    ${InstallFile} "${InBrukerPath}"    "CompassXtract 3.1.2 Release Notes.pdf" "Bruker\3.1.2" 0 0
    ${InstallFile} "${InMSCheminfoPath}" "compassxtract.adplugin"               "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
    ${InstallFile} "${InMSCheminfoPath}" "compassxtract.dll"                    "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
