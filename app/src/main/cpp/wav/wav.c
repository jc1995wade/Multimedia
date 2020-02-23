#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wav.h"
#include <jni.h>
#include  <android/log.h>
#define  TAG    "openSLES"
// 定义info信息
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,TAG,__VA_ARGS__)
// 定义debug信息
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, TAG, __VA_ARGS__)
// 定义error信息
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR,TAG,__VA_ARGS__)


/* func		: endian judge
 * return	: 0-big-endian othes-little-endian
 */
int IS_LITTLE_ENDIAN(void)
{
    int __dummy = 1;
    return ( *( (unsigned char*)(&(__dummy) ) ) );
}

unsigned int readHeader(void *dst, signed int size, signed int nmemb, AAsset* asset)
{
    unsigned int n, s0, s1, err;
    unsigned char tmp, *ptr;
    if (AAsset_read(asset,dst, size) != size)
    {
        return err;
    }
    if (!IS_LITTLE_ENDIAN() && size > 1)
    {
        //debug("big-endian \n");
        ptr = (unsigned char*)dst;
        for (n=0; n<nmemb; n++)
        {
            for (s0=0, s1=size-1; s0 < s1; s0++, s1--)
            {
                tmp = ptr[s0];
                ptr[s0] = ptr[s1];
                ptr[s1] = tmp;
            }
            ptr += size;
        }
    }
    else
    {
        //debug("little-endian \n");
    }

    return err;
}

void dumpWavInfo(WAV_INFO wavInfo)
{
    debug("compressionCode:%d \n",wavInfo.header.compressionCode);
    debug("numChannels:%d \n",wavInfo.header.numChannels);
    debug("sampleRate:%d \n",wavInfo.header.sampleRate);
    debug("bytesPerSecond:%d \n",wavInfo.header.bytesPerSecond);
    debug("blockAlign:%d \n",wavInfo.header.blockAlign);
    debug("bitsPerSample:%d \n",wavInfo.header.bitsPerSample);

}

int wavInputOpen(WAV_INFO *pWav, const char *filename)
{
    signed int offset;
    WAV_INFO *wav = pWav ;

    if (wav == NULL)
    {
        debug("Unable to allocate WAV struct.\n");
        goto error;
    }
    if (wav->asset == NULL)
    {
        debug("Unable to open wav file. %s\n", filename);
        goto error;
    }

    /* RIFF标志符判断 */
    if (AAsset_read(wav->asset, &(wav->header.riffType), 4)!=4)
    {
        debug("couldn't read RIFF_ID\n");
        goto error;  /* bad error "couldn't read RIFF_ID" */
    }
    if (strncmp("RIFF", wav->header.riffType, 4))
    {
        debug("RIFF descriptor not found.\n") ;
        goto error;
    }
    LOGD("Find RIFF \n");

    /* Read RIFF size. Ignored. */
    readHeader(&(wav->header.riffSize), 4, 1, wav->asset);
    LOGD("wav->header.riffSize:%d \n",wav->header.riffSize);

    /* WAVE标志符判断 */
    if (AAsset_read(wav->asset, &(wav->header.waveType), 4)!=4)
    {
        debug("couldn't read format\n");
        goto error;  /* bad error "couldn't read format" */
    }
    if (strncmp("WAVE", wav->header.waveType, 4))
    {
        debug("WAVE chunk ID not found.\n") ;
        goto error;
    }
    LOGD("Find WAVE \n");

    /* fmt标志符判断 */
    if (AAsset_read(wav->asset, &(wav->header.formatType), 4)!=4)
    {
        debug("couldn't read format_ID\n");
        goto error;  /* bad error "couldn't read format_ID" */
    }
    if (strncmp("fmt", wav->header.formatType, 3))
    {
        debug("fmt chunk format not found.\n") ;
        goto error;
    }
    LOGD("Find fmt \n");

    readHeader(&wav->header.formatSize, 4, 1, wav->asset);  // Ignored
    LOGD("wav->header.formatSize:%d \n",wav->header.formatSize);

    /* read  info */
    readHeader(&(wav->header.compressionCode), 2, 1, wav->asset);
    readHeader(&(wav->header.numChannels), 2, 1, wav->asset);
    LOGD("wav->header.numChannels:%d \n",wav->header.numChannels);
    readHeader(&(wav->header.sampleRate), 4, 1, wav->asset);
    LOGD("wav->header.sampleRate:%d \n",wav->header.sampleRate);
    readHeader(&(wav->header.bytesPerSecond), 4, 1, wav->asset);
    readHeader(&(wav->header.blockAlign), 2, 1, wav->asset);
    readHeader(&(wav->header.bitsPerSample), 2, 1, wav->asset);

    offset = wav->header.formatSize - 16;

    /* Wav format extensible */
    if (wav->header.compressionCode == 0xFFFE)
    {
        static const unsigned char guidPCM[16] = {
                0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x00,
                0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71
        };
        unsigned short extraFormatBytes, validBitsPerSample;
        unsigned char guid[16];
        signed int i;

        /* read extra bytes */
        readHeader(&(extraFormatBytes), 2, 1, wav->asset);
        offset -= 2;

        if (extraFormatBytes >= 22)
        {
            readHeader(&(validBitsPerSample), 2, 1, wav->asset);
            readHeader(&(wav->channelMask), 4, 1, wav->asset);
            readHeader(&(guid), 16, 1, wav->asset);

            /* check for PCM GUID */
            for (i = 0; i < 16; i++) if (guid[i] != guidPCM[i]) break;
            if (i == 16) wav->header.compressionCode = 0x01;

            offset -= 22;
        }
    }
    LOGD("wav->header.compressionCode:%d \n",wav->header.compressionCode);

    /* Skip rest of fmt header if any. */
    for (;offset > 0; offset--)
    {
        AAsset_read(wav->asset, &(wav->header.formatSize), 1);
    }

#if 1
    do
    {
        /* Read data chunk ID */
        if (AAsset_read(wav->asset, &(wav->header.dataType), 4)!=4)
        {
            debug("Unable to read data chunk ID.\n");
            free(wav);
            goto error;
        }
        /* Read chunk length. */
        readHeader(&offset, 4, 1, wav->asset);

        /* Check for data chunk signature. */
        if (strncmp("data", wav->header.dataType, 4) == 0)
        {
            LOGD("Find data \n");
            wav->header.dataSize = offset;
            break;
        }

        /* Jump over non data chunk. */
        for (;offset > 0; offset--)
        {
            AAsset_read(wav->asset, &(wav->header.dataSize), 1);
        }
    } while (AAsset_getRemainingLength(wav->asset));
    LOGD("wav->header.dataSize:%d \n",wav->header.dataSize);
#endif

    /* return success */
    return 0;

/* Error path */
    error:
    if (wav)
    {
        ;
        //free(wav);
    }
    return -1;
}


#if 0
int main(int argc,char **argv)

{
	WAV_INFO wavInfo;
	char fileName[128];
	if(argc<2 || strlen(&argv[1][0])>=sizeof(fileName))
	{
		debug("argument error !!! \n");
		return -1 ;
	}
	debug("size : %d \n",sizeof(WAV_HEADER));
	strcpy(fileName,argv[1]);
	wavInputOpen(&wavInfo, fileName);
	return 0;
}
#endif