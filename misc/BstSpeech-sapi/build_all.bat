@echo off
setlocal

echo Bestspeech SAPI5 Build
echo.

set BUILD_DIR_X86=build_x86
set BUILD_DIR_X64=build_x64
set OUTPUT_DIR=output

if not exist %BUILD_DIR_X86% mkdir %BUILD_DIR_X86%
if not exist %BUILD_DIR_X64% mkdir %BUILD_DIR_X64%
if not exist %OUTPUT_DIR% mkdir %OUTPUT_DIR%
if not exist %OUTPUT_DIR%\x64 mkdir %OUTPUT_DIR%\x64

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo ERROR: vswhere.exe not found. Please install Visual Studio 2022 or later.
    exit /b 1
)

for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do (
    set "VSINSTALLDIR=%%i\"
)

if not defined VSINSTALLDIR (
    echo ERROR: Visual Studio installation not found.
    exit /b 1
)

echo Found Visual Studio at: %VSINSTALLDIR%
echo.

echo Building x86 version...
echo.

call "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" x86
if errorlevel 1 (
    echo ERROR: Failed to setup x86 environment.
    exit /b 1
)

cmake -A Win32 -S . -B %BUILD_DIR_X86%
if errorlevel 1 (
    echo ERROR: CMake x86 configuration failed.
    exit /b 1
)

cmake --build %BUILD_DIR_X86% --config Release
if errorlevel 1 (
    echo ERROR: x86 build failed.
    exit /b 1
)

echo Building x64 version...
echo.

call "%VSINSTALLDIR%\VC\Auxiliary\Build\vcvarsall.bat" x64
if errorlevel 1 (
    echo ERROR: Failed to setup x64 environment.
    exit /b 1
)

cmake -A x64 -S . -B %BUILD_DIR_X64%
if errorlevel 1 (
    echo ERROR: CMake x64 configuration failed.
    exit /b 1
)

cmake --build %BUILD_DIR_X64% --config Release
if errorlevel 1 (
    echo ERROR: x64 build failed.
    exit /b 1
)

echo Copying results to output directory...
echo.

copy /Y "%BUILD_DIR_X86%\bin\Release\BestspeechSAPI.dll" "%OUTPUT_DIR%\"
copy /Y "%BUILD_DIR_X86%\bin\Release\BestspeechServer.exe" "%OUTPUT_DIR%\"
copy /Y "%BUILD_DIR_X86%\bin\Release\b32_tts.dll" "%OUTPUT_DIR%\"

copy /Y "%BUILD_DIR_X64%\bin\Release\BestspeechSAPI.dll" "%OUTPUT_DIR%\x64\"

echo Building installer...
echo.

if not exist "%BUILD_DIR_X86%\bin\Release\x64" mkdir "%BUILD_DIR_X86%\bin\Release\x64"
copy /Y "%OUTPUT_DIR%\x64\BestspeechSAPI.dll" "%BUILD_DIR_X86%\bin\Release\x64\"

cd %BUILD_DIR_X86%

cpack -G INNOSETUP -C Release --config CPackConfig.cmake

echo Post-processing InnoSetup script...
cmake -DISS_FILE="%CD%\_CPack_Packages\win32\INNOSETUP\ISScript.iss" -P "..\installer\post_process_iss.cmake"

echo Recompiling installer with custom code...
if exist "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" (
    "C:\Program Files (x86)\Inno Setup 6\ISCC.exe" "_CPack_Packages\win32\INNOSETUP\ISScript.iss"
) else (
    echo Warning: InnoSetup compiler not found in default locations
)

if errorlevel 1 (
    echo ERROR: Installer build failed.
    cd ..
    exit /b 1
)
cd ..

copy /Y "%BUILD_DIR_X86%\_CPack_Packages\win32\INNOSETUP\BestspeechSAPI_Setup.exe" "%OUTPUT_DIR%\"
if not exist "%OUTPUT_DIR%\BestspeechSAPI_Setup.exe" (
    copy /Y "%BUILD_DIR_X86%\BestspeechSAPI_Setup.exe" "%OUTPUT_DIR%\"
)

echo Build completed successfully!
endlocal
