#include <jni.h>
#include <string>
extern "C" {
#include "libavcodec/avcodec.h"
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_example_ffmpeg_MainActivity_avcodecConfiguration(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = avcodec_configuration();
    return env->NewStringUTF(hello.c_str());
}


extern "C" JNIEXPORT jint JNICALL
Java_com_example_ffmpeg_MainActivity_avcodecVersion(
        JNIEnv* env,
        jobject /* this */) {
    jint version = avcodec_version();
    return version;
}
