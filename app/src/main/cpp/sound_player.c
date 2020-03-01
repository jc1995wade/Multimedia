//
// Created by Administrator on 2020/3/1 0001.
//
#include <jni.h>
#include <stdlib.h>
#include <stdio.h>
#include <android/log.h>
#include <pthread.h>
#include <unistd.h>

//封装格式
#include "libavformat/avformat.h"
//解码
#include "libavcodec/avcodec.h"
//像素处理
#include "libswscale/swscale.h"
//重采样
#include "libswresample/swresample.h"

#define  TAG    "sound_player"
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)

#define MAX_AUDIO_FRME_SIZE 48000 * 4

JNIEXPORT void JNICALL Java_com_example_ffmpeg_PCMPlayer_nativeSound
  (JNIEnv *env, jobject jobj, jstring jstr_input)
{
	const char *input = (*env)->GetStringUTFChars(env, jstr_input, NULL);

	LOGD("%s",input);

	//打开输入文件
	AVFormatContext *pFormatCtx = avformat_alloc_context();
    int bak = 0;
	if((bak = avformat_open_input(&pFormatCtx, input, NULL, NULL)) != 0)
	{
		LOGE("%s, ret=%d", "打开文件失败!", bak);
		return;
	}

	//获取流信息
	if(avformat_find_stream_info(pFormatCtx, NULL) < 0)
	{
		LOGE("%s","获取输入文件信息失败!");
		return;
	}

	//对输入流做音视频判断，获取音频流索引位置
	int i = 0;
	int audio_stream_index = -1;
	for(; i < pFormatCtx->nb_streams; i++)
	{
		//判断是否是音频流
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		{
			audio_stream_index = i;
			break;
		}
	}
	if(audio_stream_index == -1)
	{
		LOGE("%s", "找不到音频流!");
		return;
	}

	//获取解码器
	AVCodecContext *codecCtx = pFormatCtx->streams[audio_stream_index]->codec;
	AVCodec *codec = avcodec_find_decoder(codecCtx->codec_id);
	if(codec == NULL)
	{
		LOGE("%s", "无法获取解码器");
		return;
	}

	//打开解码器
	if(avcodec_open2(codecCtx, codec, NULL) < 0)
	{
		LOGI("%s", "无法打开解码器");
		return;
	}

	//压缩数据
	AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
	//解压缩数据
	AVFrame *frame = av_frame_alloc();
	//将压缩数据转化为16bits 44100Hz PCM 统一音频采样格式与采样率
	SwrContext *swrCtx = swr_alloc();
	//----------重采样设置参数----------
	//输入采样格式
	enum AVSampleFormat in_sample_fmt = codecCtx->sample_fmt;
	//输出采样格式
	enum AVSampleFormat out_sample_fmt = AV_SAMPLE_FMT_S16;
	//输入采样率
	int in_sample_rate = codecCtx->sample_rate;
	//输出采样率
	int out_sample_rate = in_sample_rate;
	//获取输入的声道布局，根据声道个数获取声道布局 av_get_default_channel_layout(codec->channel_layouts);
	uint64_t in_ch_layout = codecCtx->channel_layout;
	//输出声道默认为立体声
	uint64_t out_ch_layout = AV_CH_LAYOUT_STEREO;
	swr_alloc_set_opts(swrCtx,
			out_ch_layout, out_sample_fmt, out_sample_rate,
			in_ch_layout, in_sample_fmt, in_sample_rate,
			0, NULL);
	swr_init(swrCtx);
	//输出的声道个数
	int nb_out_channel = av_get_channel_layout_nb_channels(out_ch_layout);
	//----------重采样设置参数----------
	//16bits 44100Hz PCM数据
	uint8_t *out_buffer = (uint8_t *)av_malloc(MAX_AUDIO_FRME_SIZE);
	//使用JNI调用Java
	//获取到SoundPlayer对象
	jclass player_class = (*env)->GetObjectClass(env, jobj);
	//AudioTrack对象
	jmethodID create_audio_track_mid = (*env)->GetMethodID(env, player_class, "createAudioTrack", "(II)Landroid/media/AudioTrack;");
	jobject audio_track = (*env)->CallObjectMethod(env, jobj, create_audio_track_mid, in_sample_rate, in_ch_layout);
	//AudioTrack的play方法
	jclass audio_track_class = (*env)->GetObjectClass(env, audio_track);
	jmethodID audio_track_play_mid = (*env)->GetMethodID(env, audio_track_class, "play", "()V");
	(*env)->CallVoidMethod(env, audio_track, audio_track_play_mid);
	//AudioTrack的write方法
	jmethodID audio_track_write_mid = (*env)->GetMethodID(env, audio_track_class, "write", "([BII)I");
	//不断读取压缩数据
	int ret, got_frame = 0, frame_count = 0;
	while(av_read_frame(pFormatCtx, packet) >= 0)
	{
		if(packet->stream_index == audio_stream_index)
		{
			ret = avcodec_decode_audio4(codecCtx, frame, &got_frame, packet);
			if(ret < 0)
			{
				LOGI("%s","解码完成");
			}
			if(got_frame)
			{
				LOGI("解码第%d帧", frame_count++);
				swr_convert(swrCtx, &out_buffer, MAX_AUDIO_FRME_SIZE, (const uint8_t **)frame->data, frame->nb_samples);
				//获取sample的大小
				int out_buffer_size = av_samples_get_buffer_size(NULL, nb_out_channel,
						frame->nb_samples ,out_sample_fmt, 1);
				//将PCM数据写入到AudioTrack中
				//out_buffer转化为byte数组
				jbyteArray audio_sample_array = (*env)->NewByteArray(env, out_buffer_size);
				jbyte *sample_byte = (*env)->GetByteArrayElements(env, audio_sample_array, NULL);
				//将out_buffer复制到sample_byte
				memcpy(sample_byte, out_buffer, out_buffer_size);
				//同步数据
				(*env)->ReleaseByteArrayElements(env, audio_sample_array, sample_byte, 0);
				//写入到AudioTrack中
				(*env)->CallIntMethod(env, audio_track, audio_track_write_mid,
						audio_sample_array, 0, out_buffer_size);
				//释放局部引用
				(*env)->DeleteLocalRef(env,audio_sample_array);
				usleep(16 * 1000);
			}
		}
		av_free_packet(packet);
	}
	av_frame_free(&frame);
	av_free(out_buffer);
	swr_free(&swrCtx);
	avcodec_close(codecCtx);
	avformat_close_input(&pFormatCtx);
	(*env)->ReleaseStringUTFChars(env, jstr_input, input);
}