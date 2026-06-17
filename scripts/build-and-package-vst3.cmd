@echo off
setlocal EnableExtensions

set "CONFIG=%~1"
if "%CONFIG%"=="" set "CONFIG=Release"

set "ROOT=%~dp0.."
set "BUILD_DIR=%ROOT%\build-%CONFIG%"
set "JUCE_SOURCE=%ROOT%\build\_deps\juce-src"

if exist "%JUCE_SOURCE%\CMakeLists.txt" (
    call "%~dp0dev-cmd.cmd" cmake -S "%ROOT%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG% "-DFETCHCONTENT_SOURCE_DIR_JUCE=%JUCE_SOURCE%"
    if errorlevel 1 exit /b %errorlevel%
) else (
    call "%~dp0dev-cmd.cmd" cmake -S "%ROOT%" -B "%BUILD_DIR%" -G Ninja -DCMAKE_BUILD_TYPE=%CONFIG%
    if errorlevel 1 exit /b %errorlevel%
)

call "%~dp0dev-cmd.cmd" cmake --build "%BUILD_DIR%" --target StageMindNode_VST3
if errorlevel 1 exit /b %errorlevel%

call "%~dp0package-vst3.cmd" "%CONFIG%" "%BUILD_DIR%"
exit /b %errorlevel%
