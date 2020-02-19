#include <jni.h>
#include <sys/types.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include "SLES/OpenSLES.h"
#include "SLES/OpenSLES_Android.h"
#include  <android/log.h>

#define  TAG    "openSLES"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

//混音器
static SLObjectItf outputMixObject = NULL;//用SLObjectItf创建混音器接口对象
static SLEnvironmentalReverbItf outputMixEnvironmentalReverb = NULL;////创建具体的混音器对象实例
static SLEnvironmentalReverbSettings reverbSettings = SL_I3DL2_ENVIRONMENT_PRESET_STONECORRIDOR;
//播放器
static SLObjectItf playerObject = NULL;//用SLObjectItf创建播放器接口对象
static SLPlayItf playerPlay = NULL;//创建具体的播放器对象实例
static SLVolumeItf playerVolume;

static SLObjectItf engineObject = NULL;//用SLObjectItf声明引擎接口对象
static SLEngineItf engineEngine = NULL;//声明具体的引擎对象实例


void createEngine()
{
    SLresult result;//返回结果
    LOGD("hello world openSLES");
    result = slCreateEngine(&engineObject, 0, NULL, 0, NULL, NULL);//第一步创建引擎
    (void)result;
    result = (*engineObject)->Realize(engineObject, SL_BOOLEAN_FALSE);//实现（Realize）engineObject接口对象
    (void)result;
    result = (*engineObject)->GetInterface(engineObject, SL_IID_ENGINE, &engineEngine);//通过engineObject的GetInterface方法初始化engineEngine
    (void)result;
}

JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_initAudioByOpenSL(
        JNIEnv *env,
        jobject instance,
        jobject assetManager,
        jstring filename) {
    //release();
    const char *utf8 = (*env)->GetStringUTFChars(env, filename, NULL);
    LOGD("filename=%s", utf8);
    // use asset manager to open asset by filename
    AAssetManager* mgr = AAssetManager_fromJava(env, assetManager);
    AAsset* asset = AAssetManager_open(mgr, utf8, AASSET_MODE_UNKNOWN);
    (*env)->ReleaseStringUTFChars(env, filename, utf8);
    // open asset as file descriptor
    off_t start, length;
    int fd = AAsset_openFileDescriptor(asset, &start, &length);
    AAsset_close(asset);
    LOGD("fd=%d", fd);
    SLresult result;

    //第一步，创建引擎
    createEngine();

    //第二步，创建混音器
    const SLInterfaceID mids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean mreq[1] = {SL_BOOLEAN_FALSE};
    result = (*engineEngine)->CreateOutputMix(engineEngine, &outputMixObject, 1, mids, mreq);
    (void)result;
    result = (*outputMixObject)->Realize(outputMixObject, SL_BOOLEAN_FALSE);
    (void)result;
    result = (*outputMixObject)->GetInterface(outputMixObject, SL_IID_ENVIRONMENTALREVERB, &outputMixEnvironmentalReverb);
    if (SL_RESULT_SUCCESS == result) {
        result = (*outputMixEnvironmentalReverb)->SetEnvironmentalReverbProperties(outputMixEnvironmentalReverb, &reverbSettings);
        (void)result;
    }
    //第三步，设置播放器参数和创建播放器
    // 1、 配置 audio source
    SLDataLocator_AndroidFD loc_fd = {SL_DATALOCATOR_ANDROIDFD, fd, start, length};
    SLDataFormat_MIME format_mime = {SL_DATAFORMAT_MIME, NULL, SL_CONTAINERTYPE_UNSPECIFIED};
    SLDataSource audioSrc = {&loc_fd, &format_mime};

    // 2、 配置 audio sink
    SLDataLocator_OutputMix loc_outmix = {SL_DATALOCATOR_OUTPUTMIX, outputMixObject};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    // 创建播放器
    const SLInterfaceID ids[3] = {SL_IID_SEEK, SL_IID_MUTESOLO, SL_IID_VOLUME};
     const SLboolean req[3] = {SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE, SL_BOOLEAN_TRUE};
    result = (*engineEngine)->CreateAudioPlayer(engineEngine, &playerObject, &audioSrc, &audioSnk, 3, ids, req);
    (void)result;

    // 实现播放器
    result = (*playerObject)->Realize(playerObject, SL_BOOLEAN_FALSE);
    (void)result;

    // 得到播放器接口
    result = (*playerObject)->GetInterface(playerObject, SL_IID_PLAY, &playerPlay);
    (void)result;

    // 得到声音控制接口
    result = (*playerObject)->GetInterface(playerObject, SL_IID_VOLUME, &playerVolume);
    (void)result;

    LOGD("设置播放音量");
    //设置播放音量 （100 * -50：静音 ）
    (*playerVolume)->SetVolumeLevel(playerVolume, 20 * -50);
}

JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_playAudioByOpenSL(
        JNIEnv *env,
        jobject instance) {
    SLresult result;

    //设置播放状态
    if (NULL != playerPlay) {
        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PLAYING);
        LOGD("Stop result=%d", result);
    }
}

JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_pauseAudioByOpenSL(
        JNIEnv *env,
        jobject instance) {
    SLresult result;

    //设置播放状态
    if (NULL != playerPlay) {
        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_PAUSED);
        (void)result;
        LOGD("Stop result=%d", result);
    }
}

JNIEXPORT void JNICALL
Java_com_example_ffmpeg_MainActivity_stopAudioByOpenSL(
        JNIEnv *env,
        jobject instance) {
    SLresult result;

    //设置播放状态
    if (NULL != playerPlay) {
        result = (*playerPlay)->SetPlayState(playerPlay, SL_PLAYSTATE_STOPPED);
        (void)result;
        LOGD("Stop result=%d", result);
    }
}
