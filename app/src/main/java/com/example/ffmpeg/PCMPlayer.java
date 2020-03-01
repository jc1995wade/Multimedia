package com.example.ffmpeg;
import android.media.AudioFormat;
import android.media.AudioManager;
import android.media.AudioTrack;


public class PCMPlayer {
    static {
        System.loadLibrary("native-lib");
    }
    public native void nativeSound(String input);

    public AudioTrack createAudioTrack(int sampleRateInHz, int channelConfig) {
        //44100HZ 16bits 立体声
        //int sampleRateInHz = 44100;
        int audioFormat = AudioFormat.ENCODING_PCM_16BIT;
        //int channelConfig = AudioFormat.CHANNEL_IN_STEREO;
        int channelDefaultConfig;
        if(channelConfig == 1){
            channelDefaultConfig = android.media.AudioFormat.CHANNEL_OUT_MONO;
        }else if(channelConfig == 2){
            channelDefaultConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
        }else{
            channelDefaultConfig = android.media.AudioFormat.CHANNEL_OUT_STEREO;
        }
        int bufferSizeInBytes = AudioTrack.getMinBufferSize(sampleRateInHz, channelConfig, audioFormat);
        AudioTrack audioTrack = new AudioTrack(AudioManager.STREAM_MUSIC,
                sampleRateInHz, channelDefaultConfig, audioFormat, bufferSizeInBytes, AudioTrack.MODE_STREAM);
        return audioTrack;
    }
}
