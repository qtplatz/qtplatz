;
; qtPlutz for infiTOF NSIS installer script
;
; Toshinobu Hondo, 15 October 2010
;
; see notes below the following Configuration section

;********************************
; Configuration - App name, either MassCenter3 or whatever
;********************************

!define AppName     "QtPlatz"
!define DistName    "QtPlatz"
!define BinName     "qtplatz"
!define BrandedName "ScienceLiaison ${AppName}"
;;
!include "version.nsh"
;;
; end configuration

;********************************
; Notes
;********************************
; see install.txt

;--------------------------------
;Include Modern UI

; Essential headers
; See these header files for more info
; From the standard NSIS installation

  !include "MUI.nsh"
  !include "Sections.nsh"
  !include "WinVer.nsh"

  !include "LogicLib.nsh"
  !include ".\include\EnvVarUpdate.nsh"
  
  !include "TextFunc.nsh"
  !insertmacro un.FileReadFromEnd
  !insertmacro un.TrimNewLines

  !include "WordFunc.nsh"
  !insertmacro un.WordReplace

; NSIS contributions, local copies
  !include ".\include\IsUserAdmin.nsh"

; Our own
  var InstallLogFileVariable
  var UninstallFileVariable
  !include ".\include\InstallFileMacro.nsh"

;--------------------------------
;General

  ;Name and file
  Name "${AppName}"
  OutFile "qtplatz-${VERSION}-setup.exe"

  ;Default installation folder
  InstallDir "C:\${AppName}"
  ;InstallDir "$PROGRAMFILES\${AppName}"

  ; HKLM subkey for installation directory
  !define RegInstDirKey "Software\ScienceLiaison\${AppName}"

  ; HKLM subkey for Windows Add/Remove programs support
  !define RegUninstallKey "Software\Microsoft\Windows\CurrentVersion\Uninstall\${AppName}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKLM "${RegInstDirKey}" "Installation Directory"

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
;--------------------------------
;Pages

  !define MUI_COMPONENTSPAGE_SMALLDESC

;  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  Var STARTMENU_FOLDER
  !insertmacro MUI_PAGE_STARTMENU "${AppName}" $STARTMENU_FOLDER
  !insertmacro MUI_PAGE_INSTFILES

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"


;--------------------------------
;Installer Sections

Section "!Application" App

	SectionIn RO
	SetOutPath "$INSTDIR"
	SetShellVarContext all
; LogSet off

; Create a log file, required with InstallFileMacro(.nsh)
; get the local time, which will be written to the log file
; as the creation time of the distribution.

	${GetTime} "" "L" $0 $1 $2 $3 $4 $5 $6
	StrCpy $R1 "$1/$0/$2 $4:$5:$6"

	; Installation logging
	FileOpen $InstallLogFileVariable "InstallationLog.txt" w
	FileWrite $InstallLogFileVariable "${AppName} distribution created: $R1$\r$\n"

	; For uninstall
	FileOpen $UninstallFileVariable "${AppName}Uninstall.txt" a
	FileSeek $UninstallFileVariable 0 END
	FileWrite $UninstallFileVariable "|Comment|Uninstall file for ${AppName}$\r$\n"
	
	;; Add PATH environment variable
	;;; ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR\bin"

	${EnvVarUpdate} $0 "QML_IMPORT_PATH" "A" "HKLM" "$INSTDIR\imports"

;-------------------------------- define path -------------------------
	!define InQtPlatz   "..\..\..\..\qtplatz"
	!define InQtPath    "C:\Qt\4.8.0\vc9"
	!define InVC90Path  "C:\Program Files (x86)\Microsoft Visual Studio 9.0\VC\redist\x86\Microsoft.VC90.CRT"
;-------------------------------

	!include "filelist\filelist-tao.nsh"
	!include "filelist\filelist-qt.nsh"
	!include "filelist\filelist-ecrion.nsh"
	!include "filelist\filelist-microsoft.nsh"
	!include "filelist\filelist-qtplatz.nsh"
;;; -- optional --
	!include "filelist\filelist-bruker.nsh"

	; Store installation folder and shortcut folder
   	WriteRegStr HKLM "${RegInstDirKey}" "Installation Directory" $INSTDIR
	WriteRegStr HKCU "${RegInstDirKey}" "ShortcutFolder" $STARTMENU_FOLDER

SectionEnd

;!include "..\Externals.nsi"    ; sort of a subroutine, if needed

Section "-hidden from user"

  ; Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ; Create start/programs shortcut
  CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${DistName}.lnk" "$INSTDIR\bin\${BinName}.exe" 
  ;;; CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\device_emulator.lnk" "$INSTDIR\bin\device_emulator.exe" 

  ; Create Desktop shortcut
  CreateShortCut "$DESKTOP\${DistName}.lnk" "$INSTDIR\bin\${BinName}.exe"

  CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  WriteRegStr HKLM "${RegUninstallKey}" "DisplayName" "${BrandedName}"
  WriteRegStr HKLM "${RegUninstallKey}" "UninstallString" "$INSTDIR\Uninstall.exe"

  IfErrors MainErrors NoMainErrors

  MainErrors:
	MessageBox MB_OK "Unexpected error creating shortcuts and/or registry keys"

  NoMainErrors:

  FileClose $UninstallFileVariable

SectionEnd


;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ; Unregistering DLLs -- note that we use ExecWait regsvr32 rather than NSIS's
  ; built-in UnRegDll.  Otherwise the uninstaller will start trying to
  ; delete dlls while the regserver is still trying to unregister them.

  SetShellVarContext all

  SetOutPath "$INSTDIR"

  StrCpy $R0 "${AppName}Uninstall.txt"
  
  ${if} ${FileExists} $R0
    ; Strange syntax, but it works.  Puts each line in %9, calls un.RemoveFile
    ${un.FileReadFromEnd} $R0 un.RemoveFile
    Goto DoneFiles
  ${else}
    MessageBox MB_ICONSTOP|MB_ICONEXCLAMATION "${AppName} uninstall failed, uninstall file ($R0) not found"
    goto Fail
  ${endif}

DoneFiles:
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\${AppName}Uninstall.txt"
  Delete "$INSTDIR\InstallationLog.txt"

  RMDir "$INSTDIR"

  ReadRegStr $STARTMENU_FOLDER HKCU "Software\${AppName}" "ShortcutFolder"
  StrCmp $STARTMENU_FOLDER "" DontDelete
  RMDir /r "$SMPROGRAMS\$STARTMENU_FOLDER\${AppName}"

   ;; Add PATH environment variable
   ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR\bin"

   ${un.EnvVarUpdate} $0 "QML_IMPORT_PATH" "R" "HKLM" "$INSTDIR\imports"

DontDelete:
   DeleteRegKey HKCU ${RegUninstallKey}
   DeleteRegKey HKLM "${RegUninstallKey}"

   RMDir /r "$SMPROGRAMS\$STARTMENU_FOLDER\${AppName}"

Fail:

SectionEnd


; the main file unregister-er and remover
Function un.RemoveFile

  ClearErrors
  ${un.TrimNewLines} $9 $1

  ; skip lines containing a comment token
  ${un.WordReplace} $1 "|Comment|" "" "E+" $R0
  ${if} $R0 != "1"
     goto DoneFileLoop
  ${endif}

  ; do not delete files that are not overwritten
  ${un.WordReplace} $1 "|NoOverWrite|" "" "E+" $R0
  ${if} $R0 != "1"
     goto DoneFileLoop
  ${endif}

  ${un.WordReplace} $1 "|Unregister|" "" "E+" $R0
  ${if} $R0 != "1"
     ${if} ${FileExists} $R0
        ExecWait 'regsvr32 /u /s "$R0"'
     ${endif}
  ${else}
     StrCpy $R0 $1
  ${endif}

  ${if} ${FileExists} $R0
    ClearErrors
    Delete "$R0"
    ${if} ${Errors}
       MessageBox MB_ICONSTOP "Error deleting $R0"
    ${endif}
  ${endif}

DoneFileLoop:
  ClearErrors
  StrCpy $0 " "
  Push $0

FunctionEnd


Function .onInit

; Future for Vista
;   RequestExecutionLevel admin

   ; For XP and previous
   Call IsUserAdmin
   Pop $0
   ${if} $0 != "true"
      MessageBox MB_OK|MB_ICONSTOP "Administrative privileges are required to install ${AppName}"
      Quit
   ${endif}

   ; Prevent multiple instances of the installer running
   System::Call 'kernel32::CreateMutexA(i 0, i 0, t "${AppName}") i .r1 ?e'
   Pop $0
   ${if} $0 != 0
      MessageBox MB_OK|MB_ICONEXCLAMATION "The ${AppName} installer is already running."
      Abort
   ${endif}

	; Check to make sure the version of Windows is supported
   ${if} ${AtLeastWinXP}
     goto VersionOK
   ${endif}
   
  	MessageBox MB_OK "You are using an unsupported version of Windows.  ${AppName} requires Windows XP."
   Abort

VersionOK:

FunctionEnd
