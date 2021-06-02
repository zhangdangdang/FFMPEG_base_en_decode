

#include <stdio.h>
#include<iostream>
#define __STDC_CONSTANT_MACROS

#ifdef _WIN32
 //Windows
extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/imgutils.h"
};
#else
 //Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#ifdef __cplusplus
};
#endif
#endif

//test different codec
#define TEST_H264  1
#define TEST_HEVC  0


int main(int argc, char* argv[])
{
	//AVCodec *pCodec;
	AVCodecContext *pCodecCtx = NULL;
	int i=0, ret, got_output;
	FILE *fp_in;
	FILE *fp_out;
	AVFrame *pFrame;
	AVPacket *pkt=NULL;
	int y_size;
	int framecnt = 0;

	char filename_in[] = "tseat.yuv";

#if TEST_HEVC
	AVCodecID codec_id = AV_CODEC_ID_HEVC;
	char filename_out[] = "ds.hevc";
#else
	AVCodecID codec_id = AV_CODEC_ID_H264;
	char filename_out[] = "ds.h264";
#endif


	int in_w = 640, in_h = 352;
	int framenum = 100;

	

	 AVCodec *pCodec = (AVCodec * )avcodec_find_encoder(codec_id);
	if (!pCodec) {
		printf("Codec not found\n");
		return -1;
	}
	pCodecCtx = avcodec_alloc_context3(pCodec);
	if (!pCodecCtx) {
		printf("Could not allocate video codec context\n");
		return -1;
	}
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;
	pCodecCtx->gop_size = 10;
	pCodecCtx->max_b_frames = 1;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec_id == AV_CODEC_ID_H264)
		av_opt_set(pCodecCtx->priv_data, "preset", "slow", 0);

	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		printf("Could not open codec\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	if (!pFrame) {
		printf("Could not allocate video frame\n");
		return -1;
	}
	pFrame->format = pCodecCtx->pix_fmt;
	pFrame->width = pCodecCtx->width;
	pFrame->height = pCodecCtx->height;

	ret = av_image_alloc(pFrame->data, pFrame->linesize, pCodecCtx->width, pCodecCtx->height,
		pCodecCtx->pix_fmt, 16);
	if (ret < 0) {
		printf("Could not allocate raw picture buffer\n");
		return -1;
	}
	//Input raw data
	  fopen_s(&fp_in,filename_in, "rb");
	if (!fp_in) {
		printf("Could not open %s\n", filename_in);
		return -1;
	}
	//Output bitstream
	//fp_out = fopen(filename_out, "wb");
	fopen_s(&fp_out, filename_out, "wb");
	if (!fp_out) {
		printf("Could not open %s\n", filename_out);
		return -1;
	}

	y_size = pCodecCtx->width * pCodecCtx->height;
	//Encode
	//pFrame->pts = 0;
	pkt = av_packet_alloc();
	pkt->data = NULL;    // packet data will be allocated by the encoder
	pkt->size = 0;
	pFrame->pts=0;
	while (pFrame->data[0])
	{

	
		//av_init_packet(&pkt);

		//Read raw YUV data
		if (fread(pFrame->data[0], 1, y_size, fp_in) <= 0 ||fread(pFrame->data[1], 1, y_size / 4, fp_in) <= 0 ||fread(pFrame->data[2], 1, y_size / 4, fp_in) <= 0) {	// V
			return -1;
		}
		else if (feof(fp_in)) {
			break;
		}

		pFrame->pts ++;
		//pFrame->pts= framecnt;
		/* encode the image */
		ret = avcodec_send_frame(pCodecCtx, pFrame);
		
		//ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_output);
		if (ret )
		{
			//av_log(NULL, AV_LOG_INFO, "avcodec_send_frame() encoder flushed\n");
			return ret;
		}
		/*avcodec_send_packet和avcodec_receive_frame调用关系并不一定是一对一的，
		比如一些音频数据一个AVPacket中包含了1秒钟的音频，调用一次avcodec_send_packet之后，
		可能需要调用25次 avcodec_receive_frame才能获取全部的解码音频数据，所以要做如下处理：*/
		while (avcodec_receive_packet(pCodecCtx, pkt)==0) {
			//ret = avcodec_receive_packet(pCodecCtx, pkt);
			if (ret != 0 || pkt->size > 0) {
				printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt->size);
				framecnt++;
				fwrite(pkt->data, 1, pkt->size, fp_out);
				av_packet_unref(pkt);
				
			}
			else
			{
				std::cout << "receive 失败" << std::endl;
			}
		}
		
		
	}
	av_packet_free(&pkt);
	av_frame_free(&pFrame);
	fclose(fp_out);
	avcodec_close(pCodecCtx);
	av_free(pCodecCtx);
	av_freep(&pFrame->data[0]);
	av_frame_free(&pFrame);

	return 0;
}