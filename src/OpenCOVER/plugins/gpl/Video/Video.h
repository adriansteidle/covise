#ifndef _Video_NODE_PLUGIN_H
#define _Video_NODE_PLUGIN_H

#include <util/common.h>

#include <OpenVRUI/sginterface/vruiActionUserData.h>

#include <OpenVRUI/coMenuItem.h>
#include <OpenVRUI/coMenu.h>
#include <cover/coTabletUI.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>

#include <cover/VRViewer.h>
#include <cover/coVRPluginSupport.h>

#include <cover/coVRMSController.h>
#include <cover/coVRPluginSupport.h>

#include <config/CoviseConfig.h>

#include <util/byteswap.h>
#include <util/coFileUtil.h>

#ifndef M_PI
#define M_PI 3.1415926535897931
#endif

#define MAX_NUMBER_OF_WINDOWS_OF_SLAVES 512

namespace vrui
{
class coRowMenu;
class coSubMenuItem;
class coCheckboxMenuItem;
class coButtonMenuItem;
class coPotiMenuItem;
class coVRLabel;
class coTrackerButtonInteraction;
}
using namespace vrui;
using namespace opencover;

class WINAVIPlugin;
class FFMPEGPlugin;
class VideoPlugin;

class SysPlugin: public coTUIListener
{
public:
    virtual ~SysPlugin();

    VideoPlugin *myPlugin = nullptr;
    coTUIComboBox *selectCodec = nullptr;
    coTUIComboBox *selectParams = nullptr;
    coTUIEditIntField *bitrateField = nullptr;
    std::string filterList;

    virtual void Menu(int row) = 0;
    virtual void changeFormat(coTUIElement *, int row) = 0;
    virtual void checkFileFormat(const string &name) = 0;
    virtual bool videoCaptureInit(const string &filename, int format, int RGBFormat) = 0;
    virtual void init_GLbuffers() = 0;
    virtual void close_all(bool stream = false, int format = 0) = 0;
    virtual void videoWrite(int format = 0) = 0;
};

class VideoPlugin : public coVRPlugin, public coTUIListener
{
public:
    VideoPlugin();
    ~VideoPlugin();
    bool init();

    bool update();
    void postFrame();
    void preSwapBuffers(int windowNumber);
    void setTimestep(int t);

    coTUILabel *capPos;
    coTUILabel *capArea;
    coTUILabel *videoSize;
    coTUILabel *errorLabel;
    coTUILabel *fileErrorLabel;
    coTUITab *VideoTab;
    coTUIEditField *fileNameField;
    coTUIToggleButton *captureButton;
    coTUIToggleButton *sizeButton;
    coTUIComboBox *GLformatButton;
    coTUIEditIntField *xPosField;
    coTUIEditIntField *yPosField;
    coTUIEditIntField *widthField;
    coTUIEditIntField *heightField;
    coTUIEditIntField *outWidthField;
    coTUIEditIntField *outHeightField;
    coTUILabel *showFrameLabel;
    coTUILabel *hostLabel;
    coTUIEditIntField *showFrameCountField;
    coTUIComboBox *selectFormat;
    coTUIComboBox *selectFrameRate;
    coTUIPopUp *messageBox;
    coTUIFileBrowserButton *fileNameBrowser;
    coTUIToggleButton *captureAnimButton;
    coTUILabel *frameRate;
    coTUILabel *cfpsLabel;
    coTUIEditFloatField *cfpsEdit;
    coTUIToggleButton *captureHostButton[MAX_NUMBER_OF_WINDOWS_OF_SLAVES];
    WINAVIPlugin *sysPlugWinAvi;
    FFMPEGPlugin *sysPlugFfmpeg;
    SysPlugin *sysPlug;

    void tabletEvent(coTUIElement *);
    void tabletReleaseEvent(coTUIElement *);
    void key(int type, int keySym, int mod);
    void cfpsHide(bool hidden);
    void guiToRenderMsg(const char *msg);
    virtual void message(int toWhom, int type, int length, const void *data);

    friend class WINAVIPlugin;
    friend class FFMPEGPlugin;

private:
    void fillFilenameField(const string &name, bool browser, bool changeName = true);
    bool opt_frame_size(int w, int h);
    void stopCapturing();

    bool resize;
    bool sizeError;
    bool fileError;
    int inWidth, inHeight, outWidth, outHeight;
    uint8_t *pixels;
    int time_base;
    GLenum GL_fmt;
    double starttime;
    double recordingTime;
    int recordingFrames;
    int frameCount;
    bool wasAnimating = false;
    bool captureActive;
    bool captureAnimActive;
    int captureAnimFrame;
    bool waitForFrame;
    string helpText;
    string filename;
    string stereoEye;
    int hostIndex;
    string hostName;
    int rowcount;
    bool recordFromGui_;
    int bitrate_;
    bool changeFilename_;
};
#endif
