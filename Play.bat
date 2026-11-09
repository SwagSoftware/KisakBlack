@echo off
rem Starts the built game (build\Release, or "Play.bat Debug") against the Black Ops install recorded by setup.ps1.
set "CFG=%~1"
if "%CFG%"=="" set "CFG=Release"
set "GAME=%~dp0build\%CFG%"
if not exist "%GAME%\KisakBlack.exe" (echo Game not found in %GAME% & pause & exit /b 1)
if not exist "%~dp0gamedir.txt" (echo Run setup.ps1 first: it records where your Black Ops install is. & pause & exit /b 1)
set /p BO1_INSTALL=<"%~dp0gamedir.txt"
cd /d "%GAME%"
start "" "%GAME%\KisakBlack.exe" +set fs_cdpath "%BO1_INSTALL%"
