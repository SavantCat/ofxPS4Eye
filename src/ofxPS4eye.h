//
//  ofxPS4eye.h
//  ps4eye
//
//  Created by Hotta Katusyoshi on 2015/05/17.
//
//

#ifndef ps4eye_ofxPS4eye_h
#define ps4eye_ofxPS4eye_h

#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxCv.h"
#include "ps4eye.h"

#endif

using namespace cv;
using namespace ofxCv;

#define _ps4eyeMODEx1280_MAXfps60 0
#define _ps4eyeMODEx640_MAXfps120 1
#define _ps4eyeMODEx320_MAXfps240 2
#define _ps4eyeMODExDefault 3

class ofxPS4eye : public ofThread{
public:
    void threadedFunction();
    
    void yuv2rgb(int y, int u, int v, char *r, char *g, char *b);
    void yuyvToRgb(uint8_t *in,uint8_t *out, int size_x,int size_y);
    void convert_opencv_to_RGB(uint8_t *in,uint8_t *out, int size_x,int size_y);
    void yuv440ToGray(uint8_t *in,uint8_t *out, int size_x,int size_y);
    
    void ini(int deviceNum, int mode, int fps);
    void FrameUpdate();
    void close();
    
    bool isPS4Eye;
    bool isNewFrame;
    float camFps;
    
    class info{
    public:
        ofxCvColorImage videoImage;
        ofxCvGrayscaleImage grayImage;
        float width,height;
    }data_R,data_L;
    
private:
    ps4eye::PS4EYECam::PS4EYERef eye;
    
    int deviceNum;
    int camFrameCount;
    int camFpsLastSampleFrame;
    float camFpsLastSampleTime;
    
    unsigned char * videoFrame;
    
    uint8_t *frame_rgb_left;
    uint8_t *frame_rgb_right;
};