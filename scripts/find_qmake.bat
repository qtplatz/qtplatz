::!cmd.exe
@echo off
set "drv=C:"

set "msvc=msvc2022_64"
set hints=\Qt\6.7.3 ^
        \Qt\6.5.3 ^
        \Qt\5.15.2 ^

setlocal enabledelayedexpansion

(for %%a in (%hints%) do (
     if exist "%drv%%%a\%msvc%\bin\qmake.exe" ( set QMAKE="%drv%%%a\%msvc%\bin\qmake.exe" & goto found )
))
goto done

:found
echo ---------------- find_qmake.bat::found %QMAKE%

set "%~1=%QMAKE%"

:done
