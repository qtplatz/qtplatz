   ; LIST OF FILES INCLUDED IN RELEASE BUILD
   
   !define InBinPath          "${InQtPlatz}\bin"
   !define InQMLPath          "${InQtPlatz}\share"
   !define InLibPath          "${InQtPlatz}\lib\qtplatz"
   !define InQtProjectPath    "${InQtPlatz}\lib\qtplatz\plugins\QtProject"
   !define InMSCheminfoPath   "${InQtPlatz}\lib\qtPlatz\plugins\MS-Cheminformatics"
   !define InXSDPath          "${InQtPlatz}\src\plugins\xsd"
;;   !define InMSRefPath    "${InQtPlatz}\msref"
;;   !define InxsltPath     "${InQtPlatz}\qtplutz-publisher\xslt"
   
   ;* The executable and ancilliary files we're installing *
   ; ${InstallFile} InFilePath FileName OutSubDirectory IfOverWrite IfRegister
   ;; qtplatz library modules
   ${InstallFile} "${InBinPath}"    "Aggregation.dll"        "bin" 1 0
   ${InstallFile} "${InBinPath}"    "ExtensionSystem.dll"    "bin" 1 0
   ${InstallFile} "${InBinPath}"    "Utils.dll"              "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adcontrols.dll"         "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adorbmgr.dll"           "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adextension.dll"        "bin" 1 0
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
   ${InstallFile} "${InMSCheminfoPath}" "compassxtract.dll"       "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ;;
   ${InstallFile} "${InMSCheminfoPath}" "acquire.pluginspec"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "dataproc.pluginspec"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "sequence.pluginspec"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "servant.pluginspec"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
;;
   ${InstallFile} "${InMSCheminfoPath}" "acquire.config.xml"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "servant.config.xml"      "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
   ${InstallFile} "${InMSCheminfoPath}" "dataproc.config.xml"     "lib\qtplatz\plugins\MS-Cheminformatics" 1 0
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
   ;* msref files *
;;   ${InstallFile} "${InMSRefPath}" "AglientTOFMix.ref" msref 1 0
;;   ${InstallFile} "${InMSRefPath}" "CH3COONa.ref" msref 1 0
;;   ${InstallFile} "${InMSRefPath}" "CHOONa.ref" msref 1 0
;;   ${InstallFile} "${InMSRefPath}" "PEG-Na.ref" msref 1 0

;;   ${InstallFile} "${InMSRefPath}" "API_Agilent_Lock.lock_mass" msref 1 0
;;   ${InstallFile} "${InMSRefPath}" "CHOONa.lock_mass" msref 1 0
;;   ${InstallFile} "${InMSRefPath}" "LockMass.lock_mass" msref 1 0
;;   ${InstallFile} "${InMSRefPath}" "sulfa_drug_311.lock_mass" msref 1 0
   
;;   ${InstallFile} "${InMSRefPath}" "MassCalibrationReferenceDefinitions.xsd" msref 1 0
   
   ;* qtplatz-Publisher files *
;;   ${InstallFile} "${InMC3PublisherPath}" "qtplutz-publisher.exe" bin 1 0
;;   ${InstallFile} "${InMC3PublisherPath}" "qtpluts-processor.exe" bin 1 0

   ;* xslt files *
;;   ${InstallFile} "${InxsltPath}" "qtplatz-calibrationdata_fo.xsl"      xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-calibrationreport_fo.xsl"    xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-chromatogram.xsl"            xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-controlmethod_fo.xsl"        xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-controlmethod_html.xslt"     xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-controlmethodReport_fo.xsl"  xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-data_fo.xsl"                 xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-dataAcquisition_fo.xsl"      xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-dataAnalysis_fo.xsl"         xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-dataAnalysisOptions_fo.xsl"  xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-massSpectrum.xsl"            xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-processMethod_fo.xsl"        xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-publisher.spp"               xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz-sequenceReport_fo.xsl"       xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "qtplatz--utils.xsl"                  xslt 1 0
;;   ${InstallFile} "${InxsltPath}" "svg-utils.xslt"                      xslt 1 0
