

; InstallFileMacro.nsh
; note that labels are not allowed in macros called more than once

;*** prerequisites
; perform SetOutPath "$INSTDIR"
; perform AllowSkipFiles off
; provide RegSvr32.exe first

; add gloval var InstallLogFileVariable
; perform FileOpen $InstallLogFileVariable LogFileName w
; when done, FileClose $InstallLogFileVariable

; add global var UninstallFileVariable
; perform FileOpen $UninstallFileVariable UninstallFileName a
; when done, FileClose $UninstallFileVariable

; note that SetOverWriteFlag is compile time, not run time

!ifndef InstallFileMacro_Included
!define InstallFileMacro_Included
  
!include "LogicLib.nsh"

; file functions and the single macro being used
!include "FileFunc.nsh"
!insertmacro GetTime

; Get file version and creation date and time, write to installation log file
; requires InstallLogFileVariable file variable

var FileVersion
var FileDate

!define GetFileDetails "!insertmacro GetFileDetails"
!macro GetFileDetails InFile

   ; file version
   GetDllVersion "${InFile}" $0 $1
   IntOp $2 $0 / 0x00010000
   IntOp $3 $0 & 0x0000FFFF
   IntOp $4 $1 / 0x00010000
   IntOp $5 $1 & 0x0000FFFF
   StrCpy $FileVersion "$2.$3.$4.$5"

   ; get the creation time of the file
	${GetTime} "${InFile}" "C" $0 $1 $2 $3 $4 $5 $6
	; $0="12"       day
	; $1="10"       month
	; $2="2004"     year
	; $3="Tuesday"  day of week name
	; $4="2"        hour
	; $5="32"       minute
	; $6="03"       seconds

   ${if} ${Errors}
      StrCpy $FileDate "error with date/time"
   ${else}
      StrCpy $FileDate "$1/$0/$2 $4:$5:$6"
   ${endif}

!macroend

; Macro to actually provide files, that is, invoke the NSIS "File" command
; also creates a file of file names used by the uninstall process
; requires UninstallFileVariable

var UnregisterFlag
var NoOverWriteFlag
var OutDirectory

!define InstallFile "!insertmacro InstallFile"
!macro InstallFile InFilePath FileName OutSubDirectory IfOverWrite IfRegister

;messagebox mb_ok "inst: $INSTDIR   out: $OUTDIR"
   ${if} ${OutSubDirectory} != "0"
      StrCpy $OutDirectory "$INSTDIR\${OutSubDirectory}"
      CreateDirectory "$OutDirectory"
   ${else}
      StrCpy $OutDirectory "$INSTDIR"
   ${endif}

   ClearErrors
   ${if} ${IfOverWrite} = "1"
      SetOverWrite on
      File "/oname=$OutDirectory\${FileName}" "${InFilePath}\${FileName}"
   ${else}
      SetOverWrite off
      File "/oname=$OutDirectory\${FileName}" "${InFilePath}\${FileName}"
   ${endif}

   ${if} ${Errors}
      MessageBox MB_OK "Warning: File command $OutDirectory\${FileName} failed"
   ${endif}

   ${GetFileDetails} "$OutDirectory\${FileName}"

   ${if} ${IfRegister} = "1"
      ClearErrors
      ExecWait 'regsvr32 /s "$OutDirectory\${FileName}"'
      ${if} ${Errors}
         MessageBox MB_OK "Warning: DLL registry of $OutDirectory\${FileName} failed"
      ${endif}
   ${endif}

   ${if} ${IfRegister} = "2"
      ClearErrors
      ExecWait '"$OutDirectory\${FileName}" /regserver'
      ${if} ${Errors}
         MessageBox MB_OK "Warning: DLL registry of $OutDirectory\${FileName} failed"
      ${endif}
   ${endif}

   ${if} ${IfRegister} = "3"
      ClearErrors
      ExecWait '"$OutDirectory\${FileName}"'
      ${if} ${Errors}
         MessageBox MB_OK "Warning: DLL registry of $OutDirectory\${FileName} failed"
      ${endif}
   ${endif}

   ${if} ${IfRegister} = "4"
      ClearErrors
      ExecWait '"$OutDirectory\${FileName}" /Q'
      ${if} ${Errors}
         MessageBox MB_OK "Warning: DLL registry of $OutDirectory\${FileName} failed"
      ${endif}
   ${endif}

   ;${if} ${IfRegister} = "5"
      ;ClearErrors
      ;ExecWait '"$OutDirectory\${FileName}"'
      ;${if} ${Errors}
         ;MessageBox MB_OK "Warning: DLL registry of $OutDirectory\${FileName} failed"
      ;${endif}
   ;${endif}

   ${if} ${IfRegister} = "1"
      StrCpy $UnregisterFlag "|Unregister|"
   ${else}
      StrCpy $UnregisterFlag ""
   ${endif}

   ${if} ${IfOverWrite} = "1"
      StrCpy $NoOverWriteFlag ""
   ${else}
      StrCpy $NoOverWriteFlag "|NoOverWrite|"
   ${endIf}

   FileWrite $InstallLogFileVariable "File: $OutDirectory\${FileName} Version: $FileVersion $FileDate$\r$\n"

   FileWrite $UninstallFileVariable "$OutDirectory\${FileName}$UnregisterFlag$NoOverWriteFlag$\r$\n"

!macroend

!endif ; !ifndef InstallFileMacro_Included