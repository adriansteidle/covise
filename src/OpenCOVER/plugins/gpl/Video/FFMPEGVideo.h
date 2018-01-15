#ifndef FFMPEGVIDEO_H
#define FFMPEGVIDEO_H

#include <osgViewer/GraphicsWindow>
#include <mutex>
#include <future>
#include <memory>

extern "C" {
#ifdef HAVE_FFMPEG_SEPARATE_INCLUDES
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libavutil/mathematics.h>
#include <libswscale/swscale.h>
#define HAVE_SWSCALE_H
#else
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/avutil.h>
#define AV_VERSION_INT(a, b, c) (a << 16 | b << 8 | c)
#ifdef LIBAVCODEC_VERSION_INT
#if (LIBAVCODEC_VERSION_INT > AV_VERSION_INT(51, 9, 0))
#include <ffmpeg/swscale.h>
#define HAVE_SWSCALE_H
#endif
#endif
#endif
};

#include <xercesc/dom/DOM.hpp>

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(56, 34, 1)
typedef PixelFormat AVPixelFormat;

#ifndef AV_PIX_FMT_RGB32
#define AV_PIX_FMT_RGB32 PIX_FMT_RGB32
#endif
#define AV_PIX_FMT_RGB24 PIX_FMT_RGB24
#define AV_PIX_FMT_YUV420P PIX_FMT_YUV420P
#define AV_PIX_FMT_YUVJ420P PIX_FMT_YUVJ420P

#define AV_CODEC_ID_NONE CODEC_ID_NONE
#define AV_CODEC_ID_RAWVIDEO CODEC_ID_RAWVIDEO
#define AV_CODEC_ID_FFV1 CODEC_ID_FFV1
#define AV_CODEC_ID_JPEGLS CODEC_ID_JPEGLS
#define AV_CODEC_ID_MJPEG CODEC_ID_MJPEG
#define AV_CODEC_ID_MPEG2VIDEO CODEC_ID_MPEG2VIDEO
#define AV_CODEC_ID_MPEG4 CODEC_ID_MPEG4
#define AV_CODEC_ID_FLV1 CODEC_ID_FLV1
#define AV_CODEC_ID_DVVIDEO CODEC_ID_DVVIDEO

#define AV_CODEC_FLAG_QSCALE CODEC_FLAG_QSCALE
#define AV_CODEC_FLAG_LOW_DELAY CODEC_FLAG_LOW_DELAY

#define av_frame_alloc avcodec_alloc_frame
#endif

#include "Video.h"

//#define USE_CODECPAR

struct CodecListEntry
{
    CodecListEntry(AVCodec *codec);

    AVCodec *codec = nullptr;
    std::vector<std::string> profiles;
};
typedef std::list<CodecListEntry> AVCodecList;

typedef struct
{
    std::string name;
    std::string fps;
    int constFrames;
    int avgBitrate;
    int maxBitrate;
    int width;
    int height;
} VideoParameter;

class FFMPEGPlugin: public SysPlugin
{
public:
    FFMPEGPlugin();
    ~FFMPEGPlugin();


    friend class VideoPlugin;

private:
    std::map<AVOutputFormat *, AVCodecList> formatList;
    std::list<VideoParameter> VPList;

    void tabletEvent(coTUIElement *);
    void Menu(int row);
    void ParamMenu(int row);
    void hideParamMenu(bool hide);
    void ListFormatsAndCodecs(const std::string &);
    void FillComboBoxSetExtension(int selection, int row);
    void changeFormat(coTUIElement *, int row);
    void checkFileFormat(const string &name);
    bool videoCaptureInit(const string &filename, int format, int RGBFormat);
    void videoWrite(int format = 0);
    bool FFMPEGInit(AVOutputFormat *outfmt, AVCodec *codec, const string &filename, bool test_codecs = false);
    void FFMPEGFormat(bool registerFormat);
    void init_GLbuffers();
    bool open_codec(AVCodec *codec);
    bool open_video(AVCodec *codec);
    void unInitialize();
    void close_video();
    void close_all(bool stream = false, int format = 0);
    AVFrame *alloc_picture(AVPixelFormat pix_fmt, int width, int height);
    bool add_video_stream(AVCodec *codec, int w, int h, int frame_base, int bitrate, int maxBitrate);
    AVFrame *SwConvertScale(int width, int height);
    int readParams();
    void loadParams(int);
    void saveParams();
    void addParams();
    void fillParamComboBox(int);
    void sendParams();
    int getParams();

    AVPixelFormat capture_fmt;
    AVOutputFormat *fmt;
    AVFormatContext *oc;
    AVStream *video_st;
    AVCodecContext *codecCtx;
#ifdef USE_CODECPAR
    AVCodecParameters *codecPar;
#endif
    AVPacket pkt;

    AVFrame *picture, *inPicture, *outPicture;
#ifdef HAVE_SWSCALE_H
    SwsContext *swsconvertctx;
#else
    ImgReSampleContext *imgresamplectx;
#endif
    uint8_t *video_outbuf;
    int video_outbuf_size;
    uint8_t *mirroredpixels;
    int linesize;

    coTUIEditField *paramNameField;
    coTUILabel *paramErrorLabel;
    coTUILabel *paramLabel;
    coTUILabel *bitrateLabel;
    coTUILabel *maxBitrateLabel;
    coTUIEditIntField *maxBitrateField;
    coTUIToggleButton *saveButton;

    xercesc::DOMImplementation *impl;
    std::unique_ptr<std::future<bool>> encodeFuture;
};

#endif
