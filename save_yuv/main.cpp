#include <iostream>
#include <string>
#include <unistd.h>
#include <stdlib.h>
using namespace std;

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavdevice/avdevice.h"
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
}

void ffmpegOpenVideoFile(const char * filepath)
{
    //�򿪷�װ��ʽ���ļ�
    AVFormatContext * pFormatContext = avformat_alloc_context();

    /*
     * param1:��װ��ʽ��������
     *param2: ��Ƶ�ļ���ַ
     * param3:ָ�������װ��ʽ
     * param4:ָ��Ĭ�ϵ�������Ϣ
    */
    int ret = avformat_open_input(&pFormatContext,filepath,NULL,NULL);
    if(ret != 0)
        cout<< "open input file failed "<<endl;
}
void ffmpegDecodecWithFilePath(const char * filepath)
{
    cout<<"version:"<<avcodec_version()<<endl;
    // ��һ����������
    // av_register_all() 4.0.0֮ǰ�Ľӿ�
    // �°��API���˴�����Ҫע���齨
    // ���磺���������������ȵȡ�

    // �ڶ������򿪷�װ��ʽ->���ļ�
    // ���磺.mp4��.mov��.wmv�ļ��ȵ�...
    // avformat_open_input();
    //�򿪷�װ��ʽ���ļ�
    AVFormatContext * pFormatContext = avformat_alloc_context();
    int ret = avformat_open_input(&pFormatContext,filepath,NULL,NULL);
    if(ret != 0)
    {
        cout<< "open input file failed "<<endl;
        return ;
    }
    av_dump_format(pFormatContext, 0, filepath, 0); //�����Ƶ��Ϣ
    // ��������������Ƶ��
    // �������Ƶ���룬��ô������Ƶ�����������Ƶ���룬��ô�Ͳ�����Ƶ��
    // avformat_find_stream_info();
    // AVProgram ��Ƶ�������Ϣ
    ret = avformat_find_stream_info(pFormatContext, NULL);
    if(ret < 0)
    {
        cout << "find video stream failed "<<endl;
    }
    /*
     * ���Ĳ���������Ƶ������
     * 1.������Ƶ������λ��
     *
    */
    int streamIndex =0,i;
    for(i=0; i< streamIndex; i++)
    {
        // �ж���������
        // �ɵĽӿ� formatContext->streams[i]->codec->codec_type
        // 4.0.0�Ժ��¼���������������codec
        // codec -> codecpar
        enum AVMediaType mediaType = pFormatContext->streams[i]->codecpar->codec_type;
        if(mediaType == AVMEDIA_TYPE_VIDEO)  //��Ƶ��
        {
            streamIndex =i;
            break;
        }
        else if(mediaType == AVMEDIA_TYPE_AUDIO)
        {
            //��Ƶ��
        }
        else
        {
            //������
        }
    }
    /*
     * 2.������Ƶ����������ȡ������������
     * �ɵĽӿڣ��õ������ģ�pFormatContext->streams[i]->codec
     * 4.0.0�Ժ��¼�������ʹ���codec
     * codec-codecpar �˴����½ӿڲ���Ҫ������
    */
    AVCodecParameters * avcodecParameters = pFormatContext->streams[streamIndex]->codecpar;
    enum AVCodecID codecId = avcodecParameters->codec_id;

    /*
     * 3.���ݽ����������ģ���ý�����ID��Ȼ����ҽ�����
     * avvodec_find_encoder(enum AVCodecId id) ������
    */
    AVCodec * codec = avcodec_find_decoder(codecId);
    /*
     * ���岽���򿪽�����
     * avcodec_open2()
     * �ɽӿ�ֱ��ʹ��codec��Ϊ�����Ĵ���
     * pFormatContext->streams[avformat_stream_index]->codec������
     * �½ӿ�����
    */
    AVCodecContext * avCodecContext = avcodec_alloc_context3(NULL);
    if(avCodecContext == NULL)
    {
        //����������������ʧ��
        cout<<"create condectext failed "<<endl;
        return;
    }
    // avcodec_parameters_to_context(AVCodecContext *codec, const AVCodecParameters *par)
    // ���µ�API�е� codecpar ת�� AVCodecContext
    avcodec_parameters_to_context(avCodecContext, avcodecParameters);
    ret = avcodec_open2(avCodecContext,codec,NULL);
    if(ret < 0)
    {
        cout << "open decoder failed "<< endl;
        return;
    }
    cout << "decodec name: "<< codec->name<< endl;

    /*
     * ����������ȡ��Ƶѹ������->ѭ����ȡ
     * av_read_frame(AVFoematContext *s, AVPacket *packet)
     * s: ��װ��ʽ������
     * packet:һ֡��ѹ������
    */
    AVPacket *avPacket = (AVPacket *)av_mallocz(sizeof(AVPacket));
    AVFrame *avFrameIn = av_frame_alloc();  //���ڴ�Ž���֮�����������
    // sws_getContext(int srcW, int srcH, enum AVPixelFormat srcFormat, int dstW, int dstH, enum AVPixelFormat dstFormat, int flags, SwsFilter *srcFilter, SwsFilter *dstFilter, const double *param)
    // ԭʼ����
    // scrW: ԭʼ��ʽ���
    // scrH: ԭʼ��ʽ�߶�
    // scrFormat: ԭʼ���ݸ�ʽ
    // Ŀ������
    // dstW: Ŀ���ʽ���
    // dstH: Ŀ���ʽ�߶�
    // dstFormat: Ŀ�����ݸ�ʽ
    // ������Assertion desc failed at src/libswscale/swscale_internal.h:668
    // ���������ǻ�ȡԪ���ݵĸ߶�������
    struct SwsContext *swsContext = sws_getContext(avcodecParameters->width,
                                                   avcodecParameters->height,
                                                   avCodecContext->pix_fmt,
                                                   avcodecParameters->width,
                                                   avcodecParameters->height,
                                                   AV_PIX_FMT_YUV420P,
                                                   SWS_BITEXACT, NULL, NULL, NULL);
    //����������
    //����һ��YUV420��Ƶ�������ݸ�ʽ��������һ֡���ݣ�
    AVFrame *pAVFrameYUV420P = av_frame_alloc();
    //����������������->yuv420����
     //�õ�YUV420P��������С
     // av_image_get_buffer_size(enum AVPixelFormat pix_fmt, int width, int height, int align)
     //pix_fmt: ��Ƶ�������ݸ�ʽ����->YUV420P��ʽ
     //width: һ֡��Ƶ�������ݿ� = ��Ƶ��
     //height: һ֡��Ƶ�������ݸ� = ��Ƶ��
     //align: �ֽڶ��뷽ʽ->Ĭ����1
     int bufferSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P,
                                                avCodecContext->width,
                                                avCodecContext->height,
                                                1);
     cout << "size:"<<bufferSize<<endl;
     //����һ���ڴ�ռ�
     uint8_t *outBuffer = (uint8_t *)av_malloc(bufferSize);
     //��pAVFrameYUV420P->�������
     // av_image_fill_arrays(uint8_t **dst_data, int *dst_linesize, const uint8_t *src, enum AVPixelFormat pix_fmt, int width, int height, int align)
     //dst_data: Ŀ��->�������(pAVFrameYUV420P)
     //dst_linesize: Ŀ��->ÿһ�д�С
     //src: ԭʼ����
     //pix_fmt: Ŀ��->��ʽ����
     //width: ��
     //height: ��
     //align: �ֽڶ��뷽ʽ
     av_image_fill_arrays(pAVFrameYUV420P->data,
                          pAVFrameYUV420P->linesize,
                          outBuffer,
                          AV_PIX_FMT_YUV420P,
                          avCodecContext->width,
                          avCodecContext->height,
                          1);
     FILE * fileYUV420P = fopen("out.yuv","wb+");
     int currentIndex=0,ySize,uSize,vSize;

     while(av_read_frame(pFormatContext,avPacket) >=0)
     {
        //�ж��ǲ�����Ƶ
         if(avPacket->stream_index == streamIndex)
         {
             //��ȡÿһ֡���ݣ��������һ֡����
             //����֮��õ���Ƶ����������->YUV
             // avcodec_send_packet(AVCodecContext *avctx, AVPacket *pkt)
             // avctx: ������������
             // pkt: ��ȡ�����ݰ�
             // ��ȡһ֡����
             avcodec_send_packet(avCodecContext,avPacket);

             //����
             ret = avcodec_receive_frame(avCodecContext,avFrameIn);
             if(ret == 0)  //����ɹ�
             {
                //�˴��޷���֤��Ƶ�����ظ�ʽһ����YUV��ʽ
                 //�������������һ֡���ݣ�ͳһת����ΪYUV
                 // sws_scale(struct SwsContext *c, const uint8_t *const *srcSlice, const int *srcStride, int srcSliceY, int srcSliceH, uint8_t *const *dst, const int *dstStride)
                  // SwsContext *c: ��Ƶ���ظ�ʽ��������
                  // srcSlice: ԭʼ��Ƶ��������
                  // srcStride: ԭ����ÿһ�еĴ�С
                  // srcSliceY: ���뻭��Ŀ�ʼλ�ã�һ���0��ʼ
                  // srcSliceH: ԭʼ���ݵĳ���
                  // dst: �������Ƶ��ʽ
                  // dstStride: ����Ļ����С
                 sws_scale(swsContext,(const uint8_t * const *)avFrameIn->data,
                           avFrameIn->linesize,
                           0,
                           avCodecContext->height,
                           pAVFrameYUV420P->data,
                           pAVFrameYUV420P->linesize);
                 //��ʽһ��ֱ����ʾ��Ƶ����ȥ
                 //��ʽ����д��yuv�ļ���ʽ
                 //5����yuv420p����д��.yuv�ļ���
                 //5.1 ����YUV��С
                 //����һ��ԭ��?
                 //Y��ʾ������
                 //UV��ʾ��ɫ��
                 //�й���
                 //YUV420P��ʽ�淶һ��Y�ṹ��ʾһ������(һ�����ض�Ӧһ��Y)
                 //YUV420P��ʽ�淶����4�����ص��Ӧһ��(U��V: 4Y = U = V)
                 ySize = avCodecContext->width * avCodecContext->height;
                 uSize = ySize/4;
                 vSize = ySize/4;
                 fwrite(pAVFrameYUV420P->data[0],1,ySize,fileYUV420P);
                 fwrite(pAVFrameYUV420P->data[1],1,uSize,fileYUV420P);
                 fwrite(pAVFrameYUV420P->data[2],1,vSize,fileYUV420P);
                 currentIndex++;
                 cout <<"��ǰ���� "<<currentIndex<<"֡"<<endl;
             }
         }
     }
     /*
      * �رս�����
      *
     */
     av_packet_free(&avPacket);
     fclose(fileYUV420P);
     av_frame_free(&avFrameIn);
     av_frame_free(&pAVFrameYUV420P);
     free(outBuffer);
     avcodec_close(avCodecContext);
     avformat_free_context(pFormatContext);
}
int main()
{
    ffmpegDecodecWithFilePath("F:/ffmpeg/program/first_project/1528.mp4") ;

    return 0;
}
