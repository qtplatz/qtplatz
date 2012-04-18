
	;;; !define InVC90Path  "..\microsoft\Microsoft.VC90.CRT"

	${InstallFile} "${InVC90Path}"    "Microsoft.VC90.CRT.manifest" "redist\vc90" 0 0
	${InstallFile} "${InVC90Path}"    "msvcm90.dll"       "redist\vc90" 0 0
	${InstallFile} "${InVC90Path}"    "msvcr90.dll"       "redist\vc90" 0 0
	${InstallFile} "${InVC90Path}"    "msvcp90.dll"       "redist\vc90" 0 0
