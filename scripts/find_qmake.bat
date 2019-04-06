::!cmd.exe
@echo off
set "drv=C:"
set "msvc=msvc2017_64"
set hints=\Qt\5.12.2 \Qt\5.12.1 \Qt\5.12.0 ^
          \Qt\5.11.2 \Qt\5.11.1 \Qt\5.11.0 ^
          \Qt\5.10.1 ^
	  \Qt\5.9.3 ^
	  \Qt\5.9.2 ^
	  \Qt\5.9.1 ^
	  \Qt\5.9 ^
	  \Qt\5.8 ^
	  \Qt\5.7

(for %%a in (%hints%) do (
     set QMAKE=
     if exist "%drv%%%a\%msvc%\bin\qmake.exe" ( set "QMAKE=%drv%%%a\%msvc%\bin\qmake.exe" & goto found )
))

:found
set %~1=%QMAKE%
