# Compilación de reVC para Android

Este documento explica cómo compilar reVC (GTA Vice City Reversed) para Android usando Gradle.

## Requisitos Previos

1. **Gradle 9.3+** instalado en `C:\Gradle`
2. **Android SDK** instalado en `C:\AndroidSDK`
3. **Android NDK** (versión 26.x recomendada) instalado dentro del SDK
4. **Java JDK 17+** (requerido por Gradle y Android)
5. **Git** (para que FetchContent pueda descargar dependencias)

## Configuración

### 1. Verificar rutas en `local.properties`

Edita `android/local.properties` y verifica que las rutas sean correctas:

```properties
sdk.dir=C:/AndroidSDK
```

### 2. Instalar componentes del SDK necesarios

Abre el SDK Manager y asegúrate de tener instalados:
- Android SDK Platform 34 (o superior)
- Android SDK Build-Tools 34.0.0
- Android NDK 26.x
- CMake 3.22.1

Puedes instalarlos con:
```cmd
C:\AndroidSDK\cmdline-tools\latest\bin\sdkmanager.bat "platforms;android-34" "build-tools;34.0.0" "ndk;26.1.10909125" "cmake;3.22.1"
```

## Dependencias

Las dependencias nativas se descargan y compilan automáticamente usando CMake FetchContent:

- **SDL2** (2.28.5) - desde GitHub
- **OpenAL Soft** (1.23.1) - desde GitHub  
- **mpg123** (1.32.3) - desde GitHub

No necesitas descargar ni configurar nada manualmente.

## Compilación

### Usando el script batch

```cmd
cd android
build_android.bat debug    # Compila versión debug
build_android.bat release  # Compila versión release
build_android.bat clean    # Limpia archivos de compilación
```

### Usando Gradle directamente

```cmd
cd android
C:\Gradle\bin\gradle.bat assembleDebug
```

O para release:
```cmd
C:\Gradle\bin\gradle.bat assembleRelease
```

**Nota:** La primera compilación tardará más tiempo ya que descargará SDL2, OpenAL y mpg123.

## Ubicación del APK

Después de compilar, encontrarás el APK en:
- Debug: `android/app/build/outputs/apk/debug/app-debug.apk`
- Release: `android/app/build/outputs/apk/release/app-release.apk`

## Instalación en el dispositivo

```cmd
C:\AndroidSDK\platform-tools\adb.exe install -r app\build\outputs\apk\debug\app-debug.apk
```

## Archivos del juego

Después de instalar la app, copia los archivos del juego original (GTA Vice City) a:
```
/storage/emulated/0/Android/data/com.sh0zer.revc/files/reVC/
```

Asegúrate de copiar todas las carpetas: `models`, `data`, `audio`, `TEXT`, etc.

## Solución de problemas

### Error: "SDK location not found"
Verifica que `local.properties` existe y tiene la ruta correcta al SDK.

### Error: "NDK not found"
Instala el NDK desde el SDK Manager. La versión se configura en `app/build.gradle.kts`.

### Error de compilación CMake
Verifica que CMake 3.22.1 esté instalado en el SDK.

### FetchContent falla
Asegúrate de tener Git instalado y en el PATH.

### Crash al iniciar
Verifica que los archivos del juego estén en la ubicación correcta.

## Estructura del proyecto

```
android/
├── app/
│   ├── build.gradle.kts          # Configuración del módulo app
│   ├── proguard-rules.pro        # Reglas de ProGuard
│   └── src/
│       └── main/
│           ├── AndroidManifest.xml
│           ├── cpp/
│           │   └── CMakeLists.txt  # Descarga y compila SDL2, OpenAL, mpg123
│           ├── java/com/sh0zer/revc/
│           │   ├── LauncherActivity.java
│           │   └── GameActivity.java
│           └── res/                # Recursos Android
├── build.gradle.kts              # Configuración raíz
├── settings.gradle.kts           # Configuración del proyecto
├── gradle.properties             # Propiedades de Gradle
├── local.properties              # Rutas locales (SDK)
└── build_android.bat             # Script de compilación
```
