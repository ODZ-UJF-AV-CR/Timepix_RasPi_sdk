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

// test of pxcMeasureSingleFrameTpx3 - number of pixels hit and all events
void pxcMeasureSingleFrameTpx3Test2(unsigned deviceIndex) { // =================================
    int rc; // return codes
    const unsigned cSize = 65536;
    unsigned short frameTotEvent[cSize];
    double frameToaITot[cSize];
    double frameTime=1;
    unsigned size = cSize;
    unsigned n, events, pixels;

    //int mode = PXC_TPX3_OPM_TOA;
    int mode = PXC_TPX3_OPM_EVENT_ITOT;
    rc = pxcSetTimepix3Mode(deviceIndex, mode);
    printErrors("pxcSetTimepix3Mode", rc, ENTER_ON);
    // Warning: Some devices sometimes measure something without the mode set,
    // but some other devices measure something else in this case.

    rc = pxcMeasureSingleFrameTpx3(deviceIndex, frameTime, frameToaITot, frameTotEvent, &size, PXC_TRG_NO);
    printErrors("pxcMeasureSingleFrameTpx3", rc, ENTER_OFF);
    
    if (!rc) {
        for (n=0, events=0, pixels=0; n<cSize; n++) {
            if (frameTotEvent[n]>0) pixels++;
            events += frameTotEvent[n];
        }
        printf(", pixels: %d, events: %d\n", pixels, events);
    }
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

// show: 1. frame with events count, 2. frame with ToA or iToT depends on opm
// (this view of ToA is nonsensical, for illustration only)
void showFrameDouble(unsigned short *frameTotEvent, double *frameToaITot, int opm) { // ========
    printf("events per pixels cluster: --------------------------\n");
    showFrame(frameTotEvent, false);
    if (opm==PXC_TPX3_OPM_TOA)
        printf("Toa per pixels cluster: -------------------------\n");
    else
        printf("iToT per pixels cluster: ------------------------\n");
    showFrame((unsigned short *)frameToaITot, true);
}

// test of pxcMeasureSingleFrameTpx3 - simple view of frame
void pxcMeasureSingleFrameTpx3Test(unsigned deviceIndex) { // ==================================
    int rc;                               // return codes
    const unsigned  cSize = 65536;        // chip pixels count
    unsigned short  frameTotEvent[cSize]; // frame data - event count 
    double          frameToaITot[cSize];  // frame data - integrated time over threshold
    double          frameTime=1;          // frame acquisition time
    unsigned        size = cSize;         // buffer size and measured data size

    //int mode = PXC_TPX3_OPM_TOA;
    int mode = PXC_TPX3_OPM_EVENT_ITOT;
    rc = pxcSetTimepix3Mode(deviceIndex, mode);
    printErrors("pxcSetTimepix3Mode", rc, ENTER_ON);
    // Warning: Some devices sometimes measure something without the mode set,
    // but some other devices measure something else in this case.

    rc = pxcMeasureSingleFrameTpx3(deviceIndex, frameTime, frameToaITot, frameTotEvent, &size, PXC_TRG_NO);
    
    if (rc) {
        printErrors("pxcMeasureSingleFrameTpx3", rc, ENTER_ON);
    } else {
        showFrameDouble(frameTotEvent, frameToaITot, mode);
    }
}

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
    } /* file extensions:
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
    #define PX_EXT_PIXET_RAW_DATA       "prd"  */

    if (rc==0) {
        printf("Measured frame index %lu, ", (unsigned)acqCount - 1);
        printf("count %d\n", pxcGetMeasuredFrameCount(usrData.di));

        showFrameDouble(frameTotEvent, frameToaITot, usrData.opm);
    }
}

// test of pxcMeasureMultipleFramesWithCallback - starts the measure with callback and wait for all frames arrive
void pxcMeasureMultipleFramesWithCallbackTest(unsigned deviceIndex) { // =======================
    int rc; // return codes
    double frameTime = 1;
    unsigned frameCount = 5;
    tMmfClbData usrData;

    usrData.di = deviceIndex;
    //usrData.opm = PXC_TPX3_OPM_TOA;
    usrData.opm = PXC_TPX3_OPM_EVENT_ITOT;
    rc = pxcSetTimepix3Mode(deviceIndex, usrData.opm);
    printErrors("pxcSetTimepix3Mode", rc, ENTER_ON);
    // Warning: Some devices sometimes measure something without the mode set,
    // but some other devices measure something else in this case.

    // Measure multiple frames and receive callback after each measured frame
    rc = pxcMeasureMultipleFramesWithCallback(deviceIndex, frameCount, frameTime, PXC_TRG_NO, mmfCallback, (intptr_t)&usrData);
    printErrors("pxcMeasureMultipleFramesWithCallback", rc, ENTER_ON);
}

// test of pxcMeasureMultipleFrames - measuring N frames with view the data after end
void pxcMeasureMultipleFramesTest(unsigned deviceIndex) { // ===================================
    int rc; // return codes
    double frameTime = 1;
    unsigned frameCount = 5;
    tMmfClbData usrData;

    usrData.di = deviceIndex;
    //usrData.opm = PXC_TPX3_OPM_TOA;
    usrData.opm = PXC_TPX3_OPM_EVENT_ITOT;
    rc = pxcSetTimepix3Mode(deviceIndex, usrData.opm);
    printErrors("pxcSetTimepix3Mode", rc, ENTER_ON);
    // Warning: Some devices sometimes measure something without the mode set,
    // but some other devices measure something else in this case.

    printf("measuring %d frames...\n", frameCount);

    pxcMeasureMultipleFrames(deviceIndex, frameCount, frameTime, PXC_TRG_NO);
    printErrors("pxcMeasureMultipleFrames", rc, ENTER_ON);

    for (unsigned n=0; n<frameCount; n++) {
        mmfCallback(n+1, (intptr_t)&usrData);
    }
}

// callback for pxcMeasureContinuousTest
void mcCallback(intptr_t eventData, intptr_t userData) { // ====================================
    int rc; // return codes
    static unsigned cnt=0;
    tMmfClbData usrData = *((tMmfClbData*)userData);

    mmfCallback(eventData, userData); // show frame
    printf("mcCallback evd=%d, cnt=%d, endFrame=%d - Press enter to exit\n", (unsigned)eventData, cnt, usrData.cnt);

    cnt++;
    if (usrData.cnt>0 && cnt>=usrData.cnt) {
        // stop continuous measuring on user defined number of frames
        printf("pxcAbortMeasurement...");
        rc = pxcAbortMeasurement(usrData.di);
        printErrors("pxcAbortMeasurement", rc, ENTER_ON);
        printf("Press enter to exit program.\n");
    }
}

// test of pxcMeasureContinuous - starts the measure with callback until pxcAbortMeasurement used
void pxcMeasureContinuousTest(unsigned deviceIndex) { // =======================================
    int rc; // return codes
    double frameTime = 2;
    unsigned buffrerFrames = 3;
    tMmfClbData usrData;

    usrData.di = deviceIndex;
    usrData.cnt = 0; // num of frames to stop in callback (0 endless)
    //usrData.opm = PXC_TPX3_OPM_TOA;
    usrData.opm = PXC_TPX3_OPM_EVENT_ITOT;
    rc = pxcSetTimepix3Mode(deviceIndex, usrData.opm);
    printErrors("pxcSetTimepix3Mode", rc, ENTER_ON);
    // Warning: Some devices sometimes measure something without the mode set,
    // but some other devices measure something else in this case.

    // method 1 (single step):
    //rc = pxcMeasureContinuous(deviceIndex, buffrerFrames, frameTime, PXC_TRG_NO, mcCallback, (intptr_t)&usrData);
    //printErrors("pxcMeasureContinuous", rc, ENTER_ON);

    // method 2 (2 steps):
    // register event
    pxcRegisterAcqEvent(0, PXC_ACQEVENT_ACQ_FINISHED, mcCallback, (intptr_t)&usrData);
    printErrors("pxcRegisterAcqEvent", rc, ENTER_ON);
    // (later you can use pxcUnregisterAcqEvent with the same parameters to unregister it)

    if (rc!=0) return;
    // start continuous measuring (method 2, step 2)
    rc = pxcMeasureContinuous(deviceIndex, buffrerFrames, frameTime);
    printErrors("pxcMeasureContinuous", rc, ENTER_ON);
    
    if (rc!=0) return;
    printf("waiting for callbacks...\n");
    getchar(); // waiting so that callbacks can come
    printf("pxcMeasureContinuousTest: user end\n");
}
/* Method 1:
    - if not waiting afther start, immediately occurs callback, with error "Pixet has not been initialized yet." in all functions
    - pxcAbortMeasurement more seconds waiting, generate rc=-6 and no last error,
        callback occurs again and generate errors "Frame index exceeds the number of measured frames."

   Method 2:
    - if not waiting afther start, nothing happend
    - pxcAbortMeasurement more seconds waiting, generate rc=-6 and no last error
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

    // Different Example Measurements:
    
    pxcMeasureSingleFrameTpx3Test(deviceIndex);
    printf("=================================================================\n");
    for (int i=0; i<10; i++) pxcMeasureSingleFrameTpx3Test2(deviceIndex);
    printf("=================================================================\n");

    FileTest = true; // save first frame

    pxcMeasureMultipleFramesWithCallbackTest(deviceIndex);
    printf("=================================================================\n");
    
    pxcMeasureMultipleFramesTest(deviceIndex);
    printf("=================================================================\n");

    pxcMeasureContinuousTest(deviceIndex);

    return pxcExit(); // Exit Pixet
}
