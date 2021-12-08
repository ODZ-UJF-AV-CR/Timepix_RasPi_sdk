/**
 * Copyright (C) 2021 ADVACAM
 * @author     Pavel Hudecek <pavel.hudecek@advacam.com>
 * 
 * Examples of frame-based measuring
 */

/* Used API functions:

pxcGetLastError
pxcInitialize
pxcGetDevicesCount
pxcGetDeviceName
pxcGetDeviceChipID

pxcSetTimepix3Mode

pxcMeasureSingleFrameTpx3
pxcGetMeasuredFrameTpx3
pxcMeasureMultipleFrames
pxcMeasureMultipleFramesWithCallback

pxcRegisterAcqEvent
pxcMeasureContinuousTest
pxcAbortMeasurement

requires: pxcapi.h, pxcore.dll, minipix.dll, pixet.ini, link with pxcore.lib
*/

#include "pxcapi.h"
#include <cstring>

#define ERRMSG_BUFF_SIZE    512
#define ENTER_ON            true
#define ENTER_OFF           false

// primary use to show function name, return code, last error message and optional enter
void printErrors(const char* fName, int rc, bool enter) { // ===================================
    char errorMsg[ERRMSG_BUFF_SIZE];
    pxcGetLastError(errorMsg, ERRMSG_BUFF_SIZE);
    if (errorMsg[0]>0) {
        printf("%s %d err: %s", fName, rc, errorMsg);
    } else {
        printf("%s %d err: ---", fName, rc);
    }
    if (enter) printf("\n");
}

// simple logaritmic text view of frame 256x256 pixels (short/double) to 64x32 chars
void showFrame(unsigned short *frame, bool isFloat) { // =======================================
    for (int y=0; y<32; y++) {
        for (int x=0; x<64; x++) {

            int val=0;
            // 4x8 pixels to single int
            for (int yy=0; yy<8; yy++) for (int xx=0; xx<4; xx++) {
                if (isFloat) {
                    val += (int)((double *)frame)[x*4+xx + y*256*8 + yy*256];
                } else {
                    val += frame[x*4+xx + y*256*8 + yy*256];
                }
            }

            if        (val>100000) {
                printf("O");
            } else if (val>10000) {
                printf("o");
            } else if (val>1000) {
                printf("#");
            } else if (val>100) {
                printf("*");
            } else if (val>10) {
                printf("+");
            } else if (val>0) {
                printf(".");
            } else {
                printf(" ");
            }
        }
        printf("|\n");
    }
}
/*
typedef struct { // structure for userData, that is using in mmfCallback and mcCallback
    unsigned    di;     // device index
    int         opm;    // operation mode
    unsigned    cnt=0;  // frames count (only for stop of pxcMeasureContinuous in mcCallback)
} tMmfClbData;

bool FileTest = true;

// callback for pxcMeasureMultipleFramesWithCallbackTest, reused for other frame read+view
// (Tpx3 onlly, for other chips must use other pxcGetMeasuredFrame... and data formats)
void mmfCallback(intptr_t acqCount, intptr_t userData) { // ====================================
    int rc; // return codes
    tMmfClbData usrData = *((tMmfClbData*)userData);
    const unsigned cSize = 65536;
    unsigned short frameTotEvent[cSize];
    double frameToaITot[cSize];
    unsigned size = cSize;

    printf("mmfCallback acqCount=%lu, di=%d\n", (unsigned)acqCount, usrData.di);

    rc = pxcGetMeasuredFrameTpx3(usrData.di, (unsigned)acqCount - 1, frameToaITot, frameTotEvent, &size);
    printErrors("pxcGetMeasuredFrameTpx3", rc, ENTER_OFF); printf(", size=%d\n", size);

    if (FileTest) {
        rc = pxcSaveMeasuredFrame(usrData.di, (unsigned)acqCount - 1, "./output-files/frame-testtxt");
        printErrors("pxcSaveMeasuredFrame-txt", rc, ENTER_ON);

        rc = pxcSaveMeasuredFrame(usrData.di, (unsigned)acqCount - 1, "./output-files/frame-test.png");
        printErrors("pxcSaveMeasuredFrame-txt", rc, ENTER_ON);

        FileTest = false;
    } file extensions:
    #define PX_EXT_ASCII_FRAME          "txt"
    #define PX_EXT_BINARY_FRAME         "pbf"
    #define PX_EXT_MULTI_FRAME          "pmf"
    #define PX_EXT_BINARY_MULTI_FRAME   "bmf"
    #define PX_EXT_COMP_TPXSTREAM       "pcts"
    #define PX_EXT_TPX3_PIXELS          "t3p"
    #define PX_EXT_TPX3_PIXELS_ASCII    "t3pa"
    #define PX_EXT_TPX3_RAW_DATA        "t3r"
    #define PX_EXT_FRAME_DESC           "dsc"
    #define PX_EXT_INDEX                "idx"
    #define PX_EXT_CLUSTER_LOG          "clog"
    #define PX_EXT_PIXEL_LOG            "plog"
    #define PX_EXT_PNG                  "png"
    #define PX_EXT_PIXET_RAW_DATA       "prd"  

    if (rc==0) {
        printf("Measured frame index %lu, ", (unsigned)acqCount - 1);
        printf("count %d\n", pxcGetMeasuredFrameCount(usrData.di));

        showFrameDouble(frameTotEvent, frameToaITot, usrData.opm);
    }
}
*/

int main (int argc, char const* argv[]) { // ###################################################
    // Initialize Pixet
    int rc = pxcInitialize();
    if (rc) {
        printf("Could not initialize Pixet:\n");
        printErrors("pxcInitialize", rc, ENTER_ON);
        return -1;
    }

    int connectedDevicesCount = pxcGetDevicesCount();
    printf("Connected devices: %d\n", connectedDevicesCount);

    if (connectedDevicesCount == 0) return pxcExit();

    for (unsigned devIdx = 0; (signed)devIdx < connectedDevicesCount; devIdx++){
        char deviceName[256];
        memset(deviceName, 0, 256);
        pxcGetDeviceName(devIdx, deviceName, 256);

        char chipID[256];
        memset(chipID, 0, 256);
        pxcGetDeviceChipID(devIdx, 0, chipID, 256);
        printf("Device %d: Name %s, (first ChipID: %s)\n", devIdx, deviceName, chipID);
    }
    printf("=================================================================\n");

    int deviceIndex = 0;
    int err = 0;

    unsigned size = 65536;
    unsigned short frameData[size];
    
    const char fileConfig[] = "/home/pi/Downloads/H08-W0276.xml";
    printf(fileConfig);
    printf("\n");
    
    err = pxcLoadDeviceConfiguration(deviceIndex, fileConfig);
    printf("err %d \n", err);
    
    double bias = 80.0;
    double bias_check;
    
    err = pxcSetBias(deviceIndex, bias);
    err = pxcGetBias(deviceIndex, &bias_check);
    printf("bias: %lf \n", bias_check);
    
    err = pxcMeasureSingleFrame(deviceIndex, 10.0, frameData, &size);
    printf("err %d \n", err);
    
    showFrame(frameData, false);
    
    //for(unsigned i=0;i<size;i++)
        //printf("%d,", frameData[i]);
    
    
    return pxcExit(); // Exit Pixet
}
