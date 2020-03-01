package com.example.ffmpeg;

import androidx.appcompat.app.AppCompatActivity;

import android.animation.FloatArrayEvaluator;
import android.os.Bundle;
import android.util.Log;
import android.widget.RelativeLayout;
import android.widget.TextView;
import android.content.res.AssetManager;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.Toast;

import com.github.mikephil.charting.charts.LineChart;
import com.github.mikephil.charting.components.YAxis;
import com.github.mikephil.charting.data.Entry;
import com.github.mikephil.charting.data.LineData;
import com.github.mikephil.charting.data.LineDataSet;

import java.util.ArrayList;

public class MainActivity extends AppCompatActivity implements View.OnClickListener {

    // Used to load the 'native-lib' library on application startup.
    static {
        System.loadLibrary("native-lib");
    }

    private static String TAG = "ffmpeg-app";
    private static int sequence= 8192;
    private LineChart mLineChar;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        Button btPlay = findViewById(R.id.bt_play);
        Button btPause = findViewById(R.id.bt_pause);
        Button btStop = findViewById(R.id.bt_stop);
        Button btAddData = findViewById(R.id.bt_add_data);
        Button btUpdate = findViewById(R.id.bt_update);

        mLineChar = (LineChart) findViewById(R.id.line_chart);


        // button 绑定监听
        btPlay.setOnClickListener(this);
        btPause.setOnClickListener(this);
        btStop.setOnClickListener(this);
        btAddData.setOnClickListener(this);
        btUpdate.setOnClickListener(this);

        //Example of use openSLES play mp3 on asset
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
        else if(v.getId()==R.id.bt_add_data){
            SoundThread sThread = new SoundThread();
            //检测读写权限
            PermisionUtils.verifyStoragePermissions(this);
            sThread.start();
        }
        else if(v.getId()==R.id.bt_update){
            initBarChart();
        }
    }

    /**
     * 初始化柱形图控件属性
     */
    private void initBarChart() {
        mLineChar.setBackgroundColor(0xffffff);
        mLineChar.setDescription("hello");
        mLineChar.setDrawGridBackground(true);

        AssetManager fasset= getAssets();
        float[] arr = new float[sequence/2];

        // FFTW3
        //testFftw3Fourier(fasset, "one_sample_44k.pcm", arr, 8192/2);
        testFftw3Fourier(fasset, "four_sample_44k.pcm", arr, sequence);

        // setting data
        ArrayList<Entry> valsComp1 = new ArrayList<Entry>();
        for(int i=0; i<sequence/2; i+=2){
            valsComp1.add(new Entry(arr[i], i));
        }


        LineDataSet setComp1 = new LineDataSet(valsComp1, "频率分析");
        setComp1.setAxisDependency(YAxis.AxisDependency.LEFT);

        ArrayList<LineDataSet> dataSets = new ArrayList<LineDataSet>();
        dataSets.add(setComp1);


        ArrayList<String> xVals = new ArrayList<String>();
        for(int j=0; j<sequence/2; j+=2){
            xVals.add(String.format("%dHz", (int)(j*5.3833/2)));
        }

        LineData data1 = new LineData(xVals, setComp1);
        mLineChar.setData(data1);
        mLineChar.invalidate(); // refresh


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
    public native int fftTest();
    public native void readFileData(Object assetManager, String filename);
    public native void testFftw3Fourier(Object assetManager, String filename, float[] javaArray, int num);
}
