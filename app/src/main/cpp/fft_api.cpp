//
//  main.cpp
//  MayerFFT
//
//  Created by Andrew Robertson on 10/03/2015.
//  Copyright (c) 2015 Andrew Robertson. All rights reserved.
//
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include  <android/log.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
extern "C" {
#include "wav/wav.h"
}


#define  TAG    "openSLES"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#include "Mayer_FFT.c"

void rearranger(const int n, float* buffer) {
    float real[n/2+1];
    float imag[n/2+1];
    for (int i = 0; i <= n/2; i++)
        real[i] = buffer[i];
    for (int i = 1; i < n/2; i++)
        imag[i] = -1*buffer[n-i];
    imag[n/2+1] = 0.f;
    imag[0] = 0.f;
    for (int i = 0; i <= n/2; i++)
        LOGD("fft[%i]: %f + %f j\n", i, real[i], imag[i]);
}

int fft_api(int n, float *buffer, float *real, float *imag) {
    // insert code here...
    LOGD("Simple FFT example using Ron Mayer library\n");
    for (int i = 0; i < n; i++) {
        imag[i] = 0;
        real[i] = buffer[i];
    }
    
    mayer_fft(n, &real[0], &imag[0]);//complex fft
    float amplitude;
    LOGD("\ncomplex fft call\n");
    for (int i = 0; i < n; i++) {
        amplitude = sqrt(real[i] * real[i] +imag[i] * imag[i]);
        LOGD("real[%i] %f : %f, amplitude=%f\n", i, \
                real[i], imag[i], amplitude);
    }

    return 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_fftTest(
        JNIEnv* env,
        jobject /* this */) {
    ;
}

#define NFFT 64
#define READ_LENGTH  sizeof(short)*NFFT
extern "C" JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_readFileData(
        JNIEnv *env,
        jobject instance,
        jobject assetManager,
        jstring filename) {
    const char *utf8 = env->GetStringUTFChars(filename, NULL);
    LOGD("filename=%s", utf8);
    // use asset manager to open asset by filename
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    AAsset* asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);
    env->ReleaseStringUTFChars(filename, utf8);

    AAsset_seek(asset,0,SEEK_SET);
    off_t remain = AAsset_getRemainingLength(asset);
    WAV_INFO wavInfo;
    wavInfo.asset = asset;
    wavInputOpen(&wavInfo, utf8);
    LOGD("data start =%ld", remain - wavInfo.header.dataSize);
    AAsset_seek(asset, remain - wavInfo.header.dataSize,SEEK_SET);

#if 1
    //读取
    short *read_buff = (short *)malloc(READ_LENGTH);
    memset(read_buff, 0, READ_LENGTH);
    AAsset_read(asset, read_buff, READ_LENGTH);

    float rbuf[NFFT];
    for (int i=0; i<NFFT; i++) {
        rbuf[i] = read_buff[i]/32767.0;
        LOGD("rbuf[%d]=%f", i, rbuf[i]);
    }

    float *real = (float *)malloc(sizeof(float) * NFFT);
    float *imag = (float *)malloc(sizeof(float) * NFFT);
    fft_api(NFFT, rbuf, real, imag);

    free(read_buff);
    free(real);
    free(imag);
    //剩余数据长度
    //remain=AAsset_getRemainingLength(asset);
    LOGD("end 1");
#endif
    AAsset_close(asset);
    LOGD("end 2");
#if 0
    // open asset as file descriptor
    off_t start = 0, length = 0;
    int fileLength = 1024;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    AAsset_close(asset);
    LOGD("start=%d, lenght=%d", start, length);
    lseek(fd, 0, SEEK_SET);
    LOGD("fd=%d", fd);
    unsigned char *dataBuffer = (unsigned char*)malloc(fileLength);
    memset(dataBuffer, 0, fileLength);
    int ret = 0;
    ret = read(fd, dataBuffer, fileLength);
    LOGD("ret=%d", ret);

    for(int i=0; i<10; i++) {
        LOGD("read_%d  %x %x %x %x", i, dataBuffer[0+i], dataBuffer[1+i], dataBuffer[2+i], dataBuffer[3+i]);
    }
#endif

}