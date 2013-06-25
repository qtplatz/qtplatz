   ;;;;;;;;;;;;;;;;;
   ; ACE_wrapeprs files

   !define ACE_ROOT $%ACE_ROOT%
   !define InTAOLibPath "${ACE_ROOT}\lib"
   !define InTAOBinPath "${ACE_ROOT}\bin"
   !define InTAONSPath  "..\..\..\ACE_wrappers\TAO\orbsvcs\Naming_Service\Release"

   ;* ACE-wrappers files for Windows *
   ${InstallFile} "${InTAOLibPath}"     "ACE.dll"                "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO.dll"                "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO_AnytypeCode.dll"    "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO_CodecFactory.dll"   "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO_Codeset.dll"        "bin" 0 0
;;   ${InstallFile} "${InTAOLibPath}"     "TAO_CosNaming.dll"      "bin" 0 0
;;   ${InstallFile} "${InTAOLibPath}"     "TAO_Compression.dll"    "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO_PI.dll"             "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO_PortableServer.dll" "bin" 0 0
;;   ${InstallFile} "${InTAOLibPath}"     "TAO_RTCORBA.dll"        "bin" 0 0
   ${InstallFile} "${InTAOLibPath}"     "TAO_Utils.dll"          "bin" 0 0
;   ${InstallFile} "${InTAOLibPath}"     "TAO_ZIOP.dll"           "bin" 0 0
   ;${InstallFile} "${InTAONSPath}"      "NT_Naming_Service.exe"  "bin\orbsvcs" 0 0
   ;${InstallFile} "."                   "nameservice.bat"        "bin" 1 0
   ;;;
   ;; NamingService dependent files below
   ;${InstallFile} "${InTAOLibPath}"     "TAO_CosNaming_Serv.dll" "bin\orbsvcs" 0 0
   ;${InstallFile} "${InTAOLibPath}"     "TAO_CosNaming_Skel.dll" "bin\orbsvcs" 0 0
   ;${InstallFile} "${InTAOLibPath}"     "TAO_Svc_Utils.dll"      "bin\orbsvcs" 0 0
   ;${InstallFile} "${InTAOLibPath}"     "TAO_IORTable.dll"       "bin\orbsvcs" 0 0
   ; Both NamingService & client
   ;${InstallFile} "${InTAOLibPath}"     "TAO_Messaging.dll"      "bin" 0 0
   ;${InstallFile} "${InTAOLibPath}"     "TAO_Valuetype.dll"      "bin" 0 0
   ;; Only for NamingService client
   ;${InstallFile} "${InTAOLibPath}"     "TAO_Strategies.dll"     "bin" 0 0
   ;;;
   ;; Utils
;   ${InstallFile} "${InTAOLibPath}"      "TAO_Catior_i.dll"       "bin" 1 0
;   ${InstallFile} "${InTAOBinPath}"     "tao_catior.exe"         "bin" 1 0
;   ${InstallFile} "${InTAOBinPath}"     "tao_nsadd.exe"          "bin" 1 0
;   ${InstallFile} "${InTAOBinPath}"     "tao_nsdel.exe"          "bin" 1 0
;   ${InstallFile} "${InTAOBinPath}"     "tao_nslist.exe"         "bin" 1 0
