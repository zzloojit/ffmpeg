
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include <libavutil/channel_layout.h>
#include <libavutil/common.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libswscale/swscale.h>
#include <libavutil/samplefmt.h>
#include <stdlib.h>
#include <assert.h>

static AVCodec *codec;
static AVCodecContext *context = NULL;
static AVFrame *frame;
static AVFrame *avf;
static AVPacket avpkt;

static void pgm_save(unsigned char *buf, int wrap, int xsize, int ysize,
                     char *filename)
{
    FILE *f;
    int i;

    f = fopen(filename,"w");
    fprintf(f, "P5\n%d %d\n%d\n", xsize, ysize, 255);
    for (i = 0; i < ysize; i++)
        fwrite(buf + i * wrap, 1, xsize, f);
    fclose(f);
}

static AVFrame *alloc_picture (enum PixelFormat pix_fmt, int width , int height)
{
  AVFrame *picture;
  uint8_t *picture_buf;
  int size;
  
  picture = av_frame_alloc();
  if (!picture)
    return NULL;
  
  size = avpicture_get_size (pix_fmt, width , height);
  picture_buf = (uint8_t *) av_malloc(size);
  if (!picture_buf) {
    av_free (picture);
    return NULL;
  }

  avpicture_fill ((AVPicture *)picture, picture_buf, pix_fmt, width , height);
  return picture;
}

static int toRGB (AVFrame* dst, AVFrame* src)
{
  struct SwsContext* pSwsCxtYUV = sws_getContext (src->width, src->height,
						  context->pix_fmt,src->width,
						  src->height, PIX_FMT_RGB32,
						  SWS_BICUBIC, NULL, NULL, NULL);
  int iret = sws_scale (pSwsCxtYUV, (const uint8_t* const *)src->data, src->linesize, 0, src->height, dst->data, dst->linesize);
  return iret;
}

int init_decode()
{
  avcodec_register_all();
  codec = avcodec_find_decoder(AV_CODEC_ID_H264);
  if (!codec ) {
    assert (0);
  }
  context = avcodec_alloc_context3(codec);
  if (!context) {
    assert (0);
  }
  
  if (avcodec_open2(context, codec, NULL) < 0) {
    assert (0);
  }
  av_init_packet (&avpkt);  
  frame = av_frame_alloc();
  avf = alloc_picture (PIX_FMT_RGB32, 1280, 800);//frame->width, frame->height);
  return 0;
}

char* decode_frame(char* inbuf, size_t size)
{
  int len = 0, got_frame = 0;
  static id = 0;
  char tmp_buf [256];

  avpkt.size = size;
  avpkt.data = inbuf;
  len = avcodec_decode_video2 (context, frame, &got_frame, &avpkt);
  if (len < 0) {
    assert (0);
  }
  
  if (got_frame) {
    id++;
    snprintf(tmp_buf, sizeof(tmp_buf), "/tmp/%d.pgm", id);
    pgm_save (frame->data[0], frame->linesize[0],
	      context->width, context->height, tmp_buf);
    toRGB(avf, frame);
  }
  
  assert (avpkt.size =len);
  printf("frame format is %d\n", frame->format);
  //sws_scale (pSwsCxt, frame->data, 
  return avf->data[0];
}

/* int decode_free(void); */
/* { */
/*   av_freep(&avf->data[0]); */
/*   av_free (avf); */
/* } */
