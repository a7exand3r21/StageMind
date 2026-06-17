@echo off
setlocal EnableExtensions

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"

set "ROOT=%~dp0.."
set "BUILD_DIR=%~2"
if "%BUILD_DIR%"=="" set "BUILD_DIR=%ROOT%\build-%CONFIG%"
call :package_one "StageMindNode" "StageMind Node"
if errorlevel 1 exit /b %errorlevel%

echo Packaged runtime VST3:
echo   "%ROOT%\dist\VST3\StageMind Node.vst3"
echo Add this folder to FL Studio Plugin Manager once:
echo   "%ROOT%\dist\VST3"
exit /b 0

:package_one
set "TARGET=%~1"
set "PRODUCT=%~2"
set "SOURCE=%BUILD_DIR%\%TARGET%_artefacts\%CONFIG%\VST3\%PRODUCT%.vst3"
set "DEST=%ROOT%\dist\VST3\%PRODUCT%.vst3"
set "BINARY=Contents\x86_64-win\%PRODUCT%.vst3"
set "MODULEINFO=Contents\Resources\moduleinfo.json"

if not exist "%SOURCE%\%BINARY%" (
    echo Built VST3 was not found:
    echo   "%SOURCE%"
    echo Build it first with:
    echo   scripts\build-and-package-vst3.cmd %CONFIG%
    exit /b 1
)

if not exist "%DEST%\Contents\x86_64-win" mkdir "%DEST%\Contents\x86_64-win"
if errorlevel 1 exit /b %errorlevel%

if not exist "%DEST%\Contents\Resources" mkdir "%DEST%\Contents\Resources"
if errorlevel 1 exit /b %errorlevel%

copy /Y "%SOURCE%\%BINARY%" "%DEST%\%BINARY%" >nul
if errorlevel 1 exit /b %errorlevel%

copy /Y "%SOURCE%\%MODULEINFO%" "%DEST%\%MODULEINFO%" >nul
if errorlevel 1 exit /b %errorlevel%
exit /b 0
