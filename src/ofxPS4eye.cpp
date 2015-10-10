//
//  ofxPS4eye.cpp
//  ps4eye
//
//  Created by Hotta Katusyoshi on 2015/05/17.
//
//

#include "ofxPS4eye.h"
using namespace ps4eye;

/*--------------------------------------------------------*\
 |    yuv2rgb                                               |
 \*--------------------------------------------------------*/
void ofxPS4eye::yuv2rgb(int y, int u, int v, char *r, char *g, char *b)
{
    int r1, g1, b1;
    int c = y-16, d = u - 128, e = v - 128;
    
    r1 = (298 * c           + 409 * e + 128) >> 8;
    g1 = (298 * c - 100 * d - 208 * e + 128) >> 8;
    b1 = (298 * c + 516 * d           + 128) >> 8;
    
    // Even with proper conversion, some values still need clipping.
    
    if (r1 > 255) r1 = 255;
    if (g1 > 255) g1 = 255;
    if (b1 > 255) b1 = 255;
    if (r1 < 0) r1 = 0;
    if (g1 < 0) g1 = 0;
    if (b1 < 0) b1 = 0;
    
    *r = r1 ;
    *g = g1 ;
    *b = b1 ;
}

/*--------------------------------------------------------*\
 |    yuyvToRgb                                             |
 \*--------------------------------------------------------*/

//large cost
//輝度のみ取得
void ofxPS4eye::yuyvToRgb(uint8_t *in,uint8_t *out, int size_x,int size_y)
{
    
    int i;
    unsigned int  *pixel_16=(unsigned int*)in;;     // for YUYV
    unsigned char *pixel_24=out;                    // for RGB
    int y, u, v, y2;
    char r, g, b;
    
    for (i=0; i< (size_x*size_y/2) ; i++)
    {
        // read YuYv from newBuffer (2 pixels) and build RGBRGB in pBuffer
        
        // v  = ((*pixel_16 & 0x000000ff));
        // y  = ((*pixel_16 & 0x0000ff00)>>8);
        // u  = ((*pixel_16 & 0x00ff0000)>>16);
        // y2 = ((*pixel_16 & 0xff000000)>>24);
        
        y2  = ((*pixel_16 & 0x000000ff));
        u  = ((*pixel_16 & 0x0000ff00)>>8);
        y  = ((*pixel_16 & 0x00ff0000)>>16);
        v = ((*pixel_16 & 0xff000000)>>24);
        
        yuv2rgb(y, u, v, &r, &g, &b);            // 1st pixel
        
        *pixel_24++ = r;
        *pixel_24++ = g;
        *pixel_24++ = b;
        
        yuv2rgb(y2, u, v, &r, &g, &b);            // 2nd pixel
        
        *pixel_24++ = r;
        *pixel_24++ = g;
        *pixel_24++ = b;
        
        pixel_16++;
    }
}

/*--------------------------------------------------------*\
 |    yuyvToRgb Mat version                                 |
 \*--------------------------------------------------------*/
void ofxPS4eye::convert_opencv_to_RGB(uint8_t *in,uint8_t *out, int size_x,int size_y)
{
    cout << sizeof(in) << endl;
    cv::Mat yuv(size_y,size_x,CV_8UC4 ,in);
    
    cv::Mat rgb(size_y,size_x,CV_8UC3, out);
    
    cv::cvtColor(yuv, rgb, CV_YUV2RGB);
}


/*--------------------------------------------------------*\
 |    yuv440ToGray light verstion                                             |
 \*--------------------------------------------------------*/
void ofxPS4eye::yuv440ToGray(uint8_t *in,uint8_t *out, int size_x,int size_y){
    unsigned int  *pixel_16=(unsigned int*)in;;     // for YUYV
    unsigned char *pixel_24=out;
    
    static int i;
    
    for (i=0; i< (size_x*size_y/2) ;i++)
    {
        //y  = ((*pixel_16 & 0x000000ff));
        *pixel_24++ = ((*pixel_16 & 0x000000ff));
        *pixel_24++ = ((*pixel_16 & 0x00ff0000)>>16);
        pixel_16++;
    }
}

void ofxPS4eye::ini(int deviceNum,int mode,int fps){
    
    isPS4Eye              = false;
    camFrameCount         = 0;
    camFpsLastSampleFrame = 0;
    camFpsLastSampleTime  = 0;
    camFps                = 0;
    
    std::vector<PS4EYECam::PS4EYERef> devices( PS4EYECam::getDevices() );
    
    if(devices.size() > 0)
    {
        cout << "Found devices : " << devices.size() << endl;
        
        eye = devices.at(deviceNum);
        bool res = eye->init(mode, fps);
        eye->start();
        eye->set_led_off();
        
        data_R.width  = data_L.width  = eye->getWidth();
        data_R.height = data_L.height = eye->getHeight();
        
        frame_rgb_left = new uint8_t[eye->getWidth()*eye->getHeight()];
        frame_rgb_right = new uint8_t[eye->getWidth()*eye->getHeight()];
        
        //memset(frame_rgb_left, 0, eye->getWidth()*eye->getHeight()*3);
        //memset(frame_rgb_right, 0, eye->getWidth()*eye->getHeight()*3);
        
        memset(frame_rgb_left, 0, eye->getWidth()*eye->getHeight());
        memset(frame_rgb_right, 0, eye->getWidth()*eye->getHeight());
        
        //ofxCvGrayImage
        data_R.grayImage.setFromPixels(frame_rgb_right, eye->getWidth(), eye->getHeight());
        data_L.grayImage.setFromPixels(frame_rgb_right, eye->getWidth(), eye->getHeight());
        data_R.MatImage = data_R.grayImage.getCvImage();
        data_L.MatImage = data_L.grayImage.getCvImage();
        
        videoFrame 	= new unsigned char[eye->getWidth()*eye->getHeight()*3];
        
        startThread();
        isPS4Eye = true;
    }else{
        cout << "Noting divices..." << endl;
    }
}

void ofxPS4eye::threadedFunction(){
    while (isThreadRunning() != 0) {
        if(PS4EYECam::updateDevices() == false)
            break;
        FrameUpdate();
        
        sleep(0);
    }
}

void ofxPS4eye::FrameUpdate(){
    if(isPS4Eye){
        static eyeframe * frame;
        isNewFrame = eye->isNewFrame();
        if(isNewFrame)
        {
            frame=eye->getLastVideoFramePointer();

            //Right Eye
            yuv440ToGray(frame->videoRightFrame,frame_rgb_right,eye->getWidth(), eye->getHeight());
            //ofxCvGrayImage
            data_R.grayImage.setFromPixels(frame_rgb_right, eye->getWidth(), eye->getHeight());
            data_R.MatImage = data_R.grayImage.getCvImage();
            
            //Left Eye
            yuv440ToGray(frame->videoLeftFrame,frame_rgb_left,eye->getWidth(), eye->getHeight());
            //ofxCvGrayImage
            data_L.grayImage.setFromPixels(frame_rgb_left, eye->getWidth(), eye->getHeight());
            data_L.MatImage = data_L.grayImage.getCvImage();
        }
        
        camFrameCount += isNewFrame ? 1: 0;
        float timeNow = ofGetElapsedTimeMillis();
        if( timeNow > camFpsLastSampleTime + 1000 ) {
            uint32_t framesPassed = camFrameCount - camFpsLastSampleFrame;
            camFps = (float)(framesPassed / ((timeNow - camFpsLastSampleTime)*0.001f));
            camFpsLastSampleTime = timeNow;
            camFpsLastSampleFrame = camFrameCount;
        }
    }
}

void ofxPS4eye::close(){
    if(isPS4Eye){
        stopThread();
        // You should stop before exiting
        // otherwise the app will keep working
        if(eye)
        {
            cout << "Shutdown begin wait..." << std::endl;
            eye->shutdown();
            
            delete[] frame_rgb_left;
            delete[] frame_rgb_right;
        }
    }
}
