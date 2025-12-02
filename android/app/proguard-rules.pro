# Add project specific ProGuard rules here.
# By default, the flags in this file are appended to flags specified
# in /sdk/tools/proguard/proguard-android.txt

# For native methods
-keepclasseswithmembernames class * {
    native <methods>;
}

# Keep the launcher activity
-keep class com.sh0zer.revc.** { *; }

# Keep SDL
-keep class org.libsdl.app.** { *; }
