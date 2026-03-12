@echo off
setlocal enabledelayedexpansion

:: ─── Arduino-CLI Build Script ──────────────────────────────────
:: Compiles and uploads firmware for both boards:
::   - arduino_main (Arduino Uno WiFi Rev.2)
::   - circuit_playground (Adafruit Circuit Playground Classic)
::
:: Usage:
::   build.bat arduino              - compile Arduino only
::   build.bat arduino upload       - compile + upload Arduino
::   build.bat playground           - compile Circuit Playground only
::   build.bat playground upload    - compile + upload Circuit Playground
:: ────────────────────────────────────────────────────────────────

set CONFIG=--config-file arduino-cli.yaml

:: ─── Check arguments ───────────────────────────────────────────
if /i "%~1"=="arduino"    goto :check_cli
if /i "%~1"=="playground" goto :check_cli

echo.
echo Usage:
echo   build.bat arduino              compile Arduino firmware
echo   build.bat arduino upload       compile + upload to Arduino
echo   build.bat playground           compile Circuit Playground firmware
echo   build.bat playground upload    compile + upload to Circuit Playground
echo.
exit /b 1

:: ─── Check arduino-cli is installed ────────────────────────────
:check_cli
where arduino-cli >nul 2>nul
if %errorlevel% neq 0 (
    echo.
    echo ERROR: arduino-cli not found on PATH.
    echo.
    echo Install it with:
    echo   winget install ArduinoSA.CLI
    echo.
    echo Then restart your terminal and run this script again.
    exit /b 1
)

echo [1/4] Updating board index...
arduino-cli core update-index %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to update board index.
    exit /b 1
)

if /i "%~1"=="playground" goto :setup_playground

:: ═══════════════════════════════════════════════════════════════
::  ARDUINO UNO WIFI REV.2
:: ═══════════════════════════════════════════════════════════════
:setup_arduino
set FQBN=arduino:megaavr:uno2018
set SKETCH=arduino_main\arduino_main.ino
set CORE=arduino:megaavr@1.8.8
set LIB_SERVO=Servo@1.2.2
set FLASH_LIMIT=48640
set LOCAL_LIB=--library lib\ArduinoBLE
set BOARD_NAME=Arduino Uno WiFi Rev.2

echo [2/4] Installing core %CORE%...
arduino-cli core install %CORE% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install core %CORE%.
    exit /b 1
)

echo [3/4] Installing libraries...
arduino-cli lib install %LIB_SERVO% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install library %LIB_SERVO%.
    exit /b 1
)

echo [4/4] Compiling %SKETCH%...
echo.
arduino-cli compile --fqbn %FQBN% %SKETCH% %CONFIG% %LOCAL_LIB% --warnings default ^
    --build-property "compiler.c.extra_flags=-ffunction-sections -fdata-sections -DARDUINOBLE_PERIPHERAL_ONLY" ^
    --build-property "compiler.cpp.extra_flags=-ffunction-sections -fdata-sections -DARDUINOBLE_PERIPHERAL_ONLY" ^
    --build-property "compiler.c.elf.extra_flags=-Wl,--gc-sections" 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ============================================
    echo   BUILD FAILED  [%BOARD_NAME%]
    echo ============================================
    exit /b 1
)

echo.
echo ============================================
echo   BUILD SUCCEEDED  [%BOARD_NAME%]
echo   Flash limit: %FLASH_LIMIT% bytes
echo ============================================

goto :maybe_upload

:: ═══════════════════════════════════════════════════════════════
::  CIRCUIT PLAYGROUND CLASSIC
:: ═══════════════════════════════════════════════════════════════
:setup_playground
set FQBN=adafruit:avr:circuitplay32u4cat
set SKETCH=circuit_playground\circuit_playground.ino
set CORE=adafruit:avr@1.4.15
set LIB_CP=Adafruit Circuit Playground@1.11.3
set FLASH_LIMIT=28672
set LOCAL_LIB=
set BOARD_NAME=Circuit Playground Classic

echo [2/4] Installing cores...
:: Circuit Playground Classic depends on arduino:avr as base platform
arduino-cli core install arduino:avr@1.8.6 %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install base core arduino:avr.
    exit /b 1
)
arduino-cli core install %CORE% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install core %CORE%.
    exit /b 1
)

echo [3/4] Installing libraries...
arduino-cli lib install "%LIB_CP%" %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install library %LIB_CP%.
    exit /b 1
)

echo [4/4] Compiling %SKETCH%...
echo.
arduino-cli compile --fqbn %FQBN% %SKETCH% %CONFIG% --warnings default 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ============================================
    echo   BUILD FAILED  [%BOARD_NAME%]
    echo ============================================
    exit /b 1
)

echo.
echo ============================================
echo   BUILD SUCCEEDED  [%BOARD_NAME%]
echo   Flash limit: %FLASH_LIMIT% bytes
echo ============================================

goto :maybe_upload

:: ═══════════════════════════════════════════════════════════════
::  UPLOAD (shared by both boards)
:: ═══════════════════════════════════════════════════════════════
:maybe_upload
if /i not "%~2"=="upload" goto :done

echo.
echo Detecting connected boards...
echo.

:: Write board list to temp file for parsing
set "TMPFILE=%TEMP%\arduino_boards_%RANDOM%.txt"
arduino-cli board list --format text %CONFIG% > "%TMPFILE%" 2>&1

:: Count boards (skip header line)
set BOARD_COUNT=0
set HEADER_SKIPPED=0
for /f "usebackq tokens=1,* delims= " %%a in ("%TMPFILE%") do (
    if !HEADER_SKIPPED!==0 (
        set HEADER_SKIPPED=1
    ) else (
        :: Only count lines that start with COM or /dev (actual ports)
        echo %%a | findstr /r "^COM ^/dev" >nul 2>&1
        if !errorlevel!==0 (
            set /a BOARD_COUNT+=1
            set "PORT_!BOARD_COUNT!=%%a"
            set "INFO_!BOARD_COUNT!=%%b"
        )
    )
)

:: No boards found
if %BOARD_COUNT%==0 (
    echo   No boards detected.
    echo.
    echo   Make sure your %BOARD_NAME% is:
    echo     - Plugged in via USB
    echo     - Drivers are installed
    echo     - Not in use by another program ^(Arduino IDE, Serial Monitor^)
    del "%TMPFILE%" >nul 2>&1
    exit /b 1
)

:: Single board — auto-select
if %BOARD_COUNT%==1 (
    set "SELECTED_PORT=!PORT_1!"
    echo   Found board on !SELECTED_PORT!
    echo   !INFO_1!
    goto :upload
)

:: Multiple boards — show menu
echo   Detected boards:
echo.
for /l %%i in (1,1,%BOARD_COUNT%) do (
    echo     %%i^) !PORT_%%i! - !INFO_%%i!
)
echo.
set /p CHOICE="  Select your %BOARD_NAME% (1-%BOARD_COUNT%): "

:: Validate choice
set "SELECTED_PORT="
if %CHOICE% geq 1 if %CHOICE% leq %BOARD_COUNT% (
    set "SELECTED_PORT=!PORT_%CHOICE%!"
)

if "%SELECTED_PORT%"=="" (
    echo   Invalid selection.
    del "%TMPFILE%" >nul 2>&1
    exit /b 1
)

:upload
del "%TMPFILE%" >nul 2>&1
echo.
echo Uploading to %SELECTED_PORT%  [%BOARD_NAME%]...
arduino-cli upload --fqbn %FQBN% --port %SELECTED_PORT% %SKETCH% %CONFIG% 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ============================================
    echo   UPLOAD FAILED  [%BOARD_NAME%]
    echo ============================================
    echo Make sure the board is plugged in and not
    echo in use by another program.
    exit /b 1
)
echo.
echo ============================================
echo   UPLOAD SUCCEEDED  [%BOARD_NAME%]  [%SELECTED_PORT%]
echo ============================================

:done
exit /b 0
