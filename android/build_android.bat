@echo off
REM Script para compilar reVC para Android usando Gradle

REM Configuración de rutas
set GRADLE_HOME=C:\Gradle
set ANDROID_SDK_ROOT=C:\AndroidSDK
set ANDROID_HOME=C:\AndroidSDK
set PATH=%GRADLE_HOME%\bin;%ANDROID_SDK_ROOT%\platform-tools;%PATH%

echo ======================================
echo   reVC Android Build Script
echo ======================================
echo.

REM Verificar que Gradle existe
if not exist "%GRADLE_HOME%\bin\gradle.bat" (
    echo ERROR: Gradle no encontrado en %GRADLE_HOME%
    echo Instala Gradle y ajusta la ruta GRADLE_HOME
    exit /b 1
)

REM Verificar que el SDK existe
if not exist "%ANDROID_SDK_ROOT%" (
    echo ERROR: Android SDK no encontrado en %ANDROID_SDK_ROOT%
    echo Instala el Android SDK y ajusta la ruta ANDROID_SDK_ROOT
    exit /b 1
)

REM Ir al directorio del proyecto Android
cd /d "%~dp0"

echo Usando Gradle: %GRADLE_HOME%
echo Usando Android SDK: %ANDROID_SDK_ROOT%
echo.

REM Verificar argumento
if "%1"=="" (
    echo Uso: build_android.bat [debug^|release^|clean]
    echo.
    echo Comandos:
    echo   debug   - Compila version debug
    echo   release - Compila version release
    echo   clean   - Limpia archivos de compilacion
    echo.
    set BUILD_TYPE=debug
) else (
    set BUILD_TYPE=%1
)

if "%BUILD_TYPE%"=="clean" (
    echo Limpiando proyecto...
    call "%GRADLE_HOME%\bin\gradle.bat" clean
    goto :end
)

if "%BUILD_TYPE%"=="release" (
    echo Compilando version RELEASE...
    call "%GRADLE_HOME%\bin\gradle.bat" assembleRelease
) else (
    echo Compilando version DEBUG...
    call "%GRADLE_HOME%\bin\gradle.bat" assembleDebug
)

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ======================================
    echo   Compilacion exitosa!
    echo ======================================
    echo.
    if "%BUILD_TYPE%"=="release" (
        echo APK generado en: app\build\outputs\apk\release\
    ) else (
        echo APK generado en: app\build\outputs\apk\debug\
    )
) else (
    echo.
    echo ======================================
    echo   Error en la compilacion
    echo ======================================
)

:end
pause
