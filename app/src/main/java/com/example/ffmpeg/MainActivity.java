package com.example.ffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.content.res.AssetManager;

public class MainActivity extends AppCompatActivity {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static String TAG = "ffmpeg-app";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        // Example of a avcodec test
        TextView tv = findViewById(R.id.sample_text);
        Log.d(TAG, "this is avcodecConfiguration:"+avcodecConfiguration());
        tv.setText("Avcodec Version:"+avcodecVersion());

        // Example of use openSLES play mp3 on asset
        AssetManager asset= getAssets();
        playAudioByOpenSL(asset, "许巍 - 旅行.mp3");
    }

    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String avcodecConfiguration();
    public native int avcodecVersion();
    public native int playAudioByOpenSL(Object assetManager, String filename);
}
