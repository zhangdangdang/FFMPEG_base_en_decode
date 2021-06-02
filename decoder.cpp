/**
 * 最简单的基于FFmpeg的解码器
 * Simplest FFmpeg Decoder
 *
 * 雷霄骅 Lei Xiaohua
 * leixiaohua1020@126.com
 * 中国传媒大学/数字电视技术
 * Communication University of China / Digital TV Technology
 * http://blog.csdn.net/leixiaohua1020
 *
 * 本程序实现了视频文件的解码(支持HEVC，H.264，MPEG2等)。
 * 是最简单的FFmpeg视频解码方面的教程。
 * 通过学习本例子可以了解FFmpeg的解码流程。
 * This software is a simplest video decoder based on FFmpeg.
 * Suitable for beginner of FFmpeg.
 *
 */



#include <stdio.h>
#include <iostream>
#include <fstream>
using namespace std;
#define __STDC_CONSTANT_MACROS
//#define SWS_BICUBIC           4
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
};


int main(int argc, char* argv[])
{
	AVFormatContext	*pFormatCtx= nullptr;
	int				i, videoindex;
	//AVCodecContext	*pCodecCtx = nullptr;
	//AVCodec			*pCodec = nullptr;
	//AVFrame	*pFrame = nullptr,*pFrameYUV = nullptr;
	//uint8_t *out_buffer;
	//AVPacket *packet;
	int y_size;
	int ret, got_picture;
	//struct SwsContext *img_convert_ctx;
	//输入文件路径
	char filepath[]="屌丝男士.mov";

	int frame_cnt;

	//av_register_all();
	/*加载socket库以及网络加密协议相关的库，为后续使用网络相关提供支持*/
	avformat_network_init();//网络功能的代码
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx,filepath,NULL,NULL)!=0){
		printf("Couldn't open input stream.\n");
		return -1;
	}
	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		printf("Couldn't find stream information.\n");
		return -1;
	}
	av_dump_format(pFormatCtx, 0, filepath, 0);
	videoindex=-1;
	
	/*for(i=0; i<pFormatCtx->nb_streams; i++) 
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			videoindex=i;
			break;
		}
	if(videoindex==-1){
		printf("Didn't find a video stream.\n");
		return -1;
	}

	pCodecCtx=pFormatCtx->streams[videoindex]->codec;
	pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
	if(pCodec==NULL){
		printf("Codec not found.\n");
		return -1;
	}*/
	//获取视频流 获取视频流的数组编号，判断是否是视频
	videoindex = av_find_best_stream(pFormatCtx, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	if (videoindex < 0) {
		//av_log(nullptr, AV_LOG_ERROR, "Can not find video stream!\n");
		avformat_close_input(&pFormatCtx);
		return -1;
	}

	AVCodecParameters* videoParameters = pFormatCtx->streams[videoindex]->codecpar;//一般streams数组有两个，一个视频流一个音频流
	//查找编码器
	const AVCodec* pCodec = avcodec_find_decoder(videoParameters->codec_id);
	if (!pCodec) {
		//av_log(nullptr, AV_LOG_ERROR, "Find video decoder failed!\n");
		avformat_close_input(&pFormatCtx);
		return -1;
	}

	// 创建一个视频编码器上下文
	AVCodecContext* pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		//av_log(nullptr, AV_LOG_ERROR, "Create audio_ctx failed!\n");
		avformat_close_input(&pFormatCtx);
		return -1;
	}

	// 将原始的视频参数复制到新创建的上下文中
	ret = avcodec_parameters_to_context(pCodecCtx, videoParameters);
	if (ret < 0) {
		//av_log(nullptr, AV_LOG_ERROR, "videoParameters to context failed!\n");
		avformat_close_input(&pFormatCtx);
		return -1;
	}


	// 打开视频频解码器
	if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
		printf("Could not open codec.\n");
		return -1;
	}
	/*
	 * 在此处添加输出视频信息的代码
	 * 取自于pFormatCtx，使用fprintf()
	 */
	printf("duration %d\n", pFormatCtx->duration);
	printf("iformat %s\n", pFormatCtx->iformat->name);
	printf("iformat %d*%d\n", pFormatCtx->streams[videoindex]->codecpar->width, pCodecCtx->height);

	AVFrame	*pFrame=av_frame_alloc();//AVFrame装的是解码后的yuv图像数据
	AVFrame	*pFrameYUV=av_frame_alloc();
	uint8_t *out_buffer=(uint8_t *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,1));
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer, AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height,1);
	AVPacket *packet= av_packet_alloc();//pack装的是h264的数据
	if (packet == NULL) {
		printf("packet Error.\n");
	}
	//av_init_packet(packet);
	//Output Info-----------------------------
	printf("--------------- File Information ----------------\n");
	//av_dump_format(pFormatCtx,0,filepath,0);
	printf("-------------------------------------------------\n");
	struct SwsContext *img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);

	frame_cnt=0;
	
	 //fstream outfile("t.h264", ios::out| ios::binary);
	//ofstream out("tst.h264", ios::out);
	 
	FILE *fp=nullptr ;
	FILE* fp_yuv = nullptr;
	fopen_s(&fp,"tseat.264","wb+");
	fopen_s(&fp_yuv, "tseat.yuv", "wb+");
	if (fp_yuv == 0) {//检查文件是否正确打开 文件如果被其他文件占用，会打开失败，如果不判断，会造成下面代码运行时报错
		printf("erro point fp_yuv");
		return -1;
	}
	if (fp == 0) {//检查文件是否正确打开 文件如果被其他文件占用，会打开失败，如果不判断，会造成下面代码运行时报错
		printf("erro point fp");
		return -1;
	}
	while(av_read_frame(pFormatCtx, packet)>=0){
		if(packet->stream_index==videoindex){
			fwrite(packet->data,1,packet->size,fp);
			/*for(int i=0;i<packet->size;i++)
			out.put((char)packet->data) ;*///尝试使用ofstream写失败
			//out.write((char*)packet->data, packet->size);
				/*
				 * 在此处添加输出H264码流的代码
				 * 取自于packet，使用fwrite()
				 */
			//ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (packet == NULL) {
				printf("packet Error.\n"); 
				return -1;
			}
			ret = avcodec_send_packet(pCodecCtx, packet);// 发送数据到ffmepg，放到解码队列中
			//将成功的解码队列中取出1个frame
			got_picture = avcodec_receive_frame(pCodecCtx, pFrame); //got_picture = 0 success, a frame was returned
			//fflush(stdout);
			if(ret < 0){
				printf("Decode Error.\n");
				return -1;
			}
			if(!got_picture){//md sb瞎jb改api 返回got——picture为0才是对的，以前可能是返回一
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, 
					pFrameYUV->data, pFrameYUV->linesize);
				//printf("Decoded frame index: %d\n",frame_cnt);
				if (pFrameYUV->data[0]) {
					std::cout << "无数据" << std::endl;
				}
				fwrite(pFrameYUV->data[0], 1,pCodecCtx->width*pCodecCtx->height, fp_yuv);
				fwrite(pFrameYUV->data[1], 1, pCodecCtx->width*pCodecCtx->height/4, fp_yuv);//YUV 4:2 : 0采样，每四个Y共用一组UV分量
				fwrite(pFrameYUV->data[2], 1, pCodecCtx->width*pCodecCtx->height/4, fp_yuv);
				printf("Succeed to decode frame: %5d\tsize:%5d\n", frame_cnt, pFrameYUV->width);
				/*
				 * 在此处添加输出YUV的代码
				 * 取自于pFrameYUV，使用fwrite()
				 */

				frame_cnt++;

			}
			av_frame_unref(pFrame);
		}
		av_packet_unref(packet);
	}
	//out.close();
	sws_freeContext(img_convert_ctx);
	fclose(fp);
	fclose(fp_yuv);
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

