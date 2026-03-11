@echo off
setlocal

:: ─── Arduino-CLI Consistent Build Script ─────────────────────
:: Compiles arduino_main with pinned core and library versions
:: to ensure identical binary output across all team machines.
:: ──────────────────────────────────────────────────────────────

set CONFIG=--config-file arduino-cli.yaml
set FQBN=arduino:megaavr:uno2018
set SKETCH=arduino_main\arduino_main.ino
set CORE=arduino:megaavr@1.8.8
set LIB=ArduinoBLE@1.5.0
set FLASH_LIMIT=48640

:: ─── Check arduino-cli is installed ──────────────────────────
where arduino-cli >nul 2>nul
if %errorlevel% neq 0 (
    echo.
    echo ERROR: arduino-cli not found on PATH.
    echo.
    echo Install it with one of:
    echo   winget install ArduinoSA.CLI
    echo   https://arduino.github.io/arduino-cli/latest/installation/
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

echo [2/4] Installing core %CORE%...
arduino-cli core install %CORE% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install core %CORE%.
    exit /b 1
)

echo [3/4] Installing library %LIB%...
arduino-cli lib install %LIB% %CONFIG%
if %errorlevel% neq 0 (
    echo ERROR: Failed to install library %LIB%.
    exit /b 1
)

echo [4/4] Compiling %SKETCH%...
echo.
arduino-cli compile --fqbn %FQBN% %SKETCH% %CONFIG% --warnings default 2>&1
if %errorlevel% neq 0 (
    echo.
    echo ============================================
    echo   BUILD FAILED
    echo ============================================
    exit /b 1
)

echo.
echo ============================================
echo   BUILD SUCCEEDED
echo   Flash limit: %FLASH_LIMIT% bytes
echo ============================================
echo.
echo If the sketch size above is under %FLASH_LIMIT% bytes,
echo it will fit on the Arduino Uno WiFi Rev.2.

exit /b 0
