@echo off
setlocal EnableExtensions

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"

if not exist "%VSWHERE%" (
    echo vswhere.exe was not found. Install Visual Studio Build Tools with the C++ workload.
    exit /b 1
)

set "VSINSTALL="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do (
    set "VSINSTALL=%%I"
)

if not defined VSINSTALL (
    echo Visual Studio Build Tools with MSVC x64/x86 tools was not found.
    exit /b 1
)

set "VCVARS=%VSINSTALL%\VC\Auxiliary\Build\vcvars64.bat"
set "VSCMAKE=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin"
set "VSNINJA=%VSINSTALL%\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja"

if not exist "%VCVARS%" (
    echo vcvars64.bat was not found at "%VCVARS%".
    exit /b 1
)

call "%VCVARS%" x64 >nul
if errorlevel 1 exit /b %errorlevel%

if exist "%VSCMAKE%\cmake.exe" set "PATH=%VSCMAKE%;%PATH%"
if exist "%VSNINJA%\ninja.exe" set "PATH=%VSNINJA%;%PATH%"

if "%~1"=="" (
    echo Developer environment ready: %VSINSTALL%
    cmd /k
) else (
    %*
)

exit /b %errorlevel%
