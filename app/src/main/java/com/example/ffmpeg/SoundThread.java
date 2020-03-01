package com.example.ffmpeg;

import android.util.Log;

public class SoundThread extends Thread {
    private static String TAG = "ffmpeg-app";
    public void run(){
        System.out.println("MyThread running");
        PCMPlayer player = new PCMPlayer();
        Log.d(TAG,"button .add data.....");
        player.nativeSound("/sdcard/lx.mp3");
        Log.d(TAG,"button .add data end.....");
    }
}
