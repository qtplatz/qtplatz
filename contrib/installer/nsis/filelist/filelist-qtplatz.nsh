   ; LIST OF FILES INCLUDED IN RELEASE BUILD
   
   !define InBinPath      "${InQtPlatz}\bin"
   !define InQMLPath      "${InQtPlatz}\share"
   !define InLibPath      "${InQtPlatz}\lib\qtplatz"
   !define InNokiaPath    "${InQtPlatz}\lib\qtPlatz\plugins\Nokia"
   !define InLiaisonPath  "${InQtPlatz}\lib\qtPlatz\plugins\ScienceLiaison"
   !define InXSDPath      "${InQtPlatz}\src\plugins\xsd"
;;   !define InMSRefPath    "${InQtPlatz}\msref"
;;   !define InxsltPath     "${InQtPlatz}\qtplutz-publisher\xslt"
   
   ;* The executable and ancilliary files we're installing *
   ; ${InstallFile} InFilePath FileName OutSubDirectory IfOverWrite IfRegister

   ${InstallFile} "${InBinPath}" "qtplatz.exe" bin 1 0

   ;; plugins
   ${InstallFile} "${InNokiaPath}"   "Core.dll"              "lib\qtplatz\plugins\Nokia" 1 0
   ${InstallFile} "${InNokiaPath}"   "Core.pluginspec"       "lib\qtplatz\plugins\Nokia" 1 0
   ;;
   ${InstallFile} "${InLiaisonPath}" "ChemSpider.dll"        "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "ChemSpider.pluginspec" "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "ChemSpider.config.xml" "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ;;;
   ${InstallFile} "${InLiaisonPath}" "adtxtfactory.dll"      "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "addatafile.dll"        "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "fticr.dll"             "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "compassxtract.dll"     "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ;;
   ;; acquire plugin
   ${InstallFile} "${InLiaisonPath}" "acquire.dll"           "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "dataproc.dll"          "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "sequence.dll"          "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "servant.dll"           "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "qtwidgets.dll"         "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "acquire.config.xml"    "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "servant.config.xml"    "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "dataproc.config.xml"   "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "dataproc-mimetype.xml" "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InXSDPath}"     "config.xsd"            "lib\qtplatz\plugins\xsd"            1 0
   ;;;
   ${InstallFile} "${InLiaisonPath}" "acquire.pluginspec"    "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "dataproc.pluginspec"   "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "sequence.pluginspec"   "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}" "servant.pluginspec"    "lib\qtplatz\plugins\ScienceLiaison" 1 0
   ;;
   ;;;;;
   ;; qtplatz library modules
   ${InstallFile} "${InBinPath}"    "Aggregation.dll"        "bin" 1 0
   ${InstallFile} "${InBinPath}"    "ExtensionSystem.dll"    "bin" 1 0
   ${InstallFile} "${InBinPath}"    "Utils.dll"              "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adcontrols.dll"         "bin" 1 0
   ${InstallFile} "${InBinPath}"    "adplugin.dll"           "bin" 1 0
   ${InstallFile} "${InBinPath}"    "portfolio.dll"          "bin" 1 0

   ;;;;;
   ;; qtplatz -- CORBA Servants
   ${InstallFile} "${InLiaisonPath}"    "adbroker.dll"           "lib\qtPlatz\plugins\ScienceLiaison" 1 0
   ${InstallFile} "${InLiaisonPath}"    "adcontroller.dll"       "lib\qtPlatz\plugins\ScienceLiaison" 1 0

   !define OutQtWidgetsQMLPath     "share\qtwidgets\qml"
   ;;; QML
   ;;; ./qml
   ${InstallFile} "${InQMLPath}\qtwidgets\qml" "ProcessMethodEditor.qml"             "${OutQtWidgetsQMLPath}" 1 0

   ;;; ./qml/content:
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "BusyIndicator.qml"           "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "CaptionText.qml"             "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "CategoryDelegate.qml"        "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditCentroidMethod.qml"      "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditElementalCompMethod.qml" "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditIntegrationMethod.qml"   "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditIsotopeMethod.qml"       "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditLockMassMethod.qml"      "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditMSCalibMethod.qml"       "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditReportMethod.qml"        "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "EditTargetMethod.qml"        "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "MethodEditDelegate.qml"      "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "ScanType.qml"                "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "ScanTypeDetails.qml"         "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "ScrollBar.qml"               "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "TextInputBox.qml"            "${OutQtWidgetsQMLPath}\content" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content" "TitleText.qml"               "${OutQtWidgetsQMLPath}\content" 0 0

   ;;;./qml/content/images:
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content\images" "busy.png"              "${OutQtWidgetsQMLPath}\content\images" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content\images" "clear.png"             "${OutQtWidgetsQMLPath}\content\images" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content\images" "lineedit-bg-focus.png" "${OutQtWidgetsQMLPath}\content\images" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content\images" "lineedit-bg.png"       "${OutQtWidgetsQMLPath}\content\images" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content\images" "scantype.png"          "${OutQtWidgetsQMLPath}\content\images" 0 0
   ${InstallFile} "${InQMLPath}\qtwidgets\qml\content\images" "scrollbar.png"         "${OutQtWidgetsQMLPath}\content\images" 0 0

   ;; end of qtPlat core modules
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
   
   ;* qtPlatz-Publisher files *
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
