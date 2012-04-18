   ;;
   ;;;;;;;;;;;; MSI LabView & Averager driver files
   ;;
   ;;   !define InMSIDllPath     "..\..\MSI\DLL"
   ;;   !define InMSILabViewPath "..\..\MSI\LabView"
   ;;   !define InMSIDrvPath     "..\..\MSI\Driver"

   !define InMSIPath        "..\..\..\qtplatz\lib\qtplatz\plugins\MSI"
   !define InMSIQMLPath     "..\..\..\qtplatz\share\MSI\qml"
   !define OutMSIQMLPath    "share\MSI\qml"
   ;; This is original dll that controls averager
   ;;   ${InstallFile} "${InMSIDLLPath}" "tofdll2.dll"          "MSI\DLL" 0 0
   ;; 
   ;; and this is fake
   ;;  ${InstallFile} "${InMSILabViewPath}" "tofdll.dll"       "MSI\LabView" 1 0
   ;;
   ;;;;;;;;;;;;;;;;;;;;;;;
   ;; driver
   ;;   ${InstallFile} "${InMSIDrvPath}" "TofDigUsbIf.inf"      "MSI\EZUSB" 1 0
   ;;   ${InstallFile} "${InMSIDrvPath}" "TofDigUsbIf.cat"      "MSI\EZUSB" 1 0
   ;;   ${InstallFile} "${InMSIDrvPath}" "TofDigUsbIf_x64.cat"  "MSI\EZUSB" 1 0
   ;;   ${InstallFile} "${InMSIDrvPath}" "libusb0.dll"          "MSI\EZUSB" 1 0
   ;;   ${InstallFile} "${InMSIDrvPath}" "libusb0_x64.dll"      "MSI\EZUSB" 1 0
   ;;   ${InstallFile} "${InMSIDrvPath}" "libusb0_x64.sys"      "MSI\EZUSB" 1 0
   ;;
   ;;;;;;;;;;;;;;;;;;;;;;;
   ;; MSI model InfiTOF 
   ${InstallFile} "${InMSIPath}"     "infitofspectrometer.dll" "lib\qtplatz\plugins\MSI" 1 0
   ${InstallFile} "${InMSIPath}"     "InfiTof.dll"             "lib\qtplatz\plugins\MSI" 1 0
   ${InstallFile} "${InMSIPath}"     "InfiTof.pluginspec"      "lib\qtplatz\plugins\MSI" 1 0

   ;;;;;;;;;;;;;;;;;;;;;;;
   ;;; QML
   
   ${InstallFile} "${InMSIQMLPath}"          "frontbezel.qml"           "${OutMSIQMLPath}" 1 0
   ${InstallFile} "${InMSIQMLPath}"          "frontpanel.qml"           "${OutMSIQMLPath}" 1 0
   ${InstallFile} "${InMSIQMLPath}"          "sideframe.qml"            "${OutMSIQMLPath}" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "AnalyzerTune.qml"        "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "BusyIndicator.qml"       "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "CaptionText.qml"         "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "CategoryDelegate.qml"    "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "EISourceTune.qml"        "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "MethodEditDelegate.qml"  "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "ScrollBar.qml"           "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "Spinner.qml"             "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "TextInputBox.qml"        "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "TitleText.qml"           "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "VoltageActual.qml"       "${OutMSIQMLPath}\content" 1 0
   ${InstallFile} "${InMSIQMLPath}\content"  "VoltageInputBox.qml"     "${OutMSIQMLPath}\content" 1 0
   ;;; images
   ${InstallFile} "${InMSIQMLPath}\content\images" "busy.png"         "${OutMSIQMLPath}\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "clear.png"        "${OutMSIQMLPath}\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "folder_new.png"   "${OutMSIQMLPath}\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "header.png"       "${OutMSIQMLPath}\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "lineedit-bg-focus.png" "${OutMSIQMLPath}\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "lineedit-bg.png" "lib\qtplatz\plugins\MSI\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "page.png"        "lib\qtplatz\plugins\MSI\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "panel.png"       "lib\qtplatz\plugins\MSI\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "scrollbar.png"   "lib\qtplatz\plugins\MSI\content\images" 1 0
   ${InstallFile} "${InMSIQMLPath}\content\images" "selectedrow.png" "lib\qtplatz\plugins\MSI\content\images" 1 0

;; obsolete driver
;;   ${InstallFile} "${InMSIPath}"    "infitofcontroller.dll"   "lib\qtplatz\plugins\MSI" 1 0
;;   ${InstallFile} "${InMSIPath}"    "infitoftune.dll"         "lib\qtplatz\plugins\MSI" 1 0
;;   ${InstallFile} "${InMSIPath}"    "infitoftune.pluginspec"  "lib\qtplatz\plugins\MSI" 1 0

   ;; End of MSI modules
