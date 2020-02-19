package com.example.ffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.util.Log;
import android.widget.TextView;
import android.content.res.AssetManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static String TAG = "ffmpeg-app";
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btPlay = findViewById(R.id.bt_play);
        Button btPause = findViewById(R.id.bt_pause);
        Button btStop = findViewById(R.id.bt_stop);
        //绑定监听
        btPlay.setOnClickListener(this);
        btPause.setOnClickListener(this);
        btStop.setOnClickListener(this);

        // Example of use openSLES play mp3 on asset
        AssetManager asset= getAssets();
        initAudioByOpenSL(asset, "许巍 - 旅行.mp3");

        // Example of a avcodec test
        TextView tv = findViewById(R.id.sample_text);
        Log.d(TAG, "this is avcodecConfiguration:"+avcodecConfiguration());
        tv.setText("Avcodec Version:"+avcodecVersion());

    }

    //重写onClick()方法
    @Override
    public void onClick(View v) {
        Log.d(TAG,"button ......");
        if(v.getId()==R.id.bt_play){
            Toast.makeText(this,"play is clicked",Toast.LENGTH_SHORT).show();
            playAudioByOpenSL();
        }
        else if(v.getId()==R.id.bt_pause){
            Toast.makeText(this,"pause is clicked",Toast.LENGTH_SHORT).show();
            pauseAudioByOpenSL();
        }
        else if(v.getId()==R.id.bt_stop){
            Toast.makeText(this,"stop is clicked",Toast.LENGTH_SHORT).show();
            stopAudioByOpenSL();
        }
    }


    /**
     * A native method that is implemented by the 'native-lib' native library,
     * which is packaged with this application.
     */
    public native String avcodecConfiguration();
    public native int avcodecVersion();
    public native int initAudioByOpenSL(Object assetManager, String filename);
    public native int playAudioByOpenSL();
    public native void pauseAudioByOpenSL();
    public native void stopAudioByOpenSL();
}
