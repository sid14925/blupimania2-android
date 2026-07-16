package org.blupi.blupimania2;

import org.libsdl.app.SDLActivity;

public class BlupiActivity extends SDLActivity {
    @Override
    protected String[] getLibraries() {
        return new String[] {
            "SDL2",
            "blupimania2"
        };
    }
}
