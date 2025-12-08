package com.sh0zer.revc;

import android.os.Bundle;
import org.libsdl.app.SDLActivity;

public class GameActivity extends SDLActivity {

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
    }

    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL3",
            "openal",
            "mpg123",
            "revc"
        };
    }

    @Override
    protected String getMainFunction() {
        return "LaunchAndroid";
    }
}
