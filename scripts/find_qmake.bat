::!cmd.exe
@echo off
set "drv=C:"
::set "msvc=msvc2017_64"
set "msvc=msvc2019_64"
set hints=\Qt\5.15.2 ^
          \Qt\5.15.1 ^
          \Qt\5.15.0 ^
    	  \Qt\5.14.1 ^
    	  \Qt\5.12.7 ^
          \Qt\5.11.2 ^
          \Qt\5.10.1 ^
	  \Qt\5.9.3 ^

set QMAKE=

echo %hist%
(for %%a in (%hints%) do (
     if exist "%drv%%%a\%msvc%\bin\qmake.exe" ( set "QMAKE=%drv%%%a\%msvc%\bin\qmake.exe" & goto found )
))
goto done

:found
set %~1=%QMAKE%

:done
