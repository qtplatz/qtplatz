::!cmd.exe
@echo off

set "drv=C:"
set "msvc=msvc2022_64"

set hints=\Qt\6.10.1 ^
        \Qt\6.7.3 ^
        \Qt\6.5.3 ^
        \Qt\5.15.2 ^

setlocal EnableDelayedExpansion

(for %%a in (%hints%) do (
     if exist "%drv%%%a\%msvc%\bin\qmake.exe" ( set QMAKE="%drv%%%a\%msvc%\bin\qmake.exe" & goto found )
))

goto done

:found
echo ---------------- find_qmake.bat::found QMAKE=%QMAKE%
set "arg1=%~1"

:done

endlocal & set "%arg1=%QMAKE%"

::@echo off
::set "QMAKE=C:\Windows\System32\notepad.exe"
::set "varName=%~1"
::set "!varName!=!QMAKE!"
::endlocal & set "%~1=%QMAKE%"
