#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <fftw3/fftw3.h>
#include  <android/log.h>
#define  TAG    "fftw3"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)


JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_testFftw3Fourier (
        JNIEnv * env,
        jobject instance,
        jobject assetManager,
        jstring filename,
        jfloatArray * javaArray,
        jint  num) {
    const char *utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
    LOGD("filename=%s", utf8);
    // use asset manager to open asset by filename
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    AAsset* asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);
    (*env)->ReleaseStringUTFChars(env, filename, utf8);

    AAsset_seek(asset,0,SEEK_SET);

    unsigned int N = num;   // 441000/8192 = 5.3833Hz
                            // 82*5.3833Hz = 441.43Hz
    fftwf_complex *in, *out;  // float[i][0] float[i][1]
    fftwf_plan p;
    in = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * N);
    out = (fftwf_complex*) fftwf_malloc(sizeof(fftwf_complex) * N);
    short *read_buff = (short *)malloc(sizeof(short)*N);
    AAsset_read(asset, read_buff, N);

    int len = N;
    for(int i = 0; i < len; i++)
    {
        in[i][0] = read_buff[i]/32767.0;
        in[i][1] = 0;
        //LOGD("%f%+fi\n", in[i][0], in[i][1]);
    }
    p = fftwf_plan_dft_1d(N, in, out, FFTW_FORWARD, FFTW_ESTIMATE);
    fftwf_execute(p); /* repeat as needed */

    float *farray = (float *)malloc(sizeof(float)*N/2);
    (*env)->GetFloatArrayRegion(env,javaArray, 0, N/2, farray);

    // 输出幅度谱
    float max_val = 0;
    int max_position = 0;
    for(int i = 0; i < N/2; i++)
    {
        farray[i] = sqrt(out[i][0] * out[i][0] + out[i][1] * out[i][1]);
        if (max_val < farray[i]) {  // 记录最大幅度值
            max_val = farray[i];
            max_position = i;
        }
        //LOGD("len=%d, lent=%.2f ",i, lent);
        //usleep(1000*2);
    }
    LOGD("Freq=%d  Amplitude=%f",max_position, max_val);

    //写回数据到java
    //参数1：待赋值数组；参数2：赋值起始位置；参数3：赋值长度；参数4：数据源
    (*env)->SetFloatArrayRegion(env,javaArray,0, N/2, farray);

    fftwf_destroy_plan(p);
    fftwf_free(in);
    fftwf_free(out);
    AAsset_close(asset);
    free(farray);
}