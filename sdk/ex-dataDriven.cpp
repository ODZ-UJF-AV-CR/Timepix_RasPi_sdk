/**
 * Copyright (C) 2021 ADVACAM
 * @authors    Jan Ingerle <jan.ingerle@advacam.com>
 *             Pavel Hudecek <pavel.hudecek@advacam.com>
 * 
 * Data driven measuring examples
 */

/* Used API functions:

pxcGetLastError
pxcInitialize
pxcGetDevicesCount
pxcGetDeviceName
pxcGetDeviceChipID

pxcSetDeviceParameter
pxcSetTimepix3Mode

pxcGetMeasuredTpx3Pixels
pxcGetMeasuredTpx3PixelsCount
pxcGetMetaDataValue
pxcMeasureTpx3DataDrivenMode

requires: pxcapi.h, common.h, pxcore.dll, minipix.dll, pixet.ini, link with pxcore.lib
*/

#include "pxcapi.h"
//#include "common.h"
//#include <cstring>
//#include <algorithm>
//#include <vector>
//#include <thread>

#define PAR_DD_BUFF_SIZE    "DDBuffSize"    // Data Driven Buffer Size [MB], default 100
#define PAR_DD_BLOCK_SIZE   "DDBlockSize"   // Data Driven Block Size [B], default 66000
#define PAR_TRG_STG         "TrgStg"        // 0=logical 0, 1 = logical 1, 2 = rising edge, 3 = falling edge
#define PAR_DCC_LEVEL       "DataConsystencyCheckLevel"
// sensitivity on data timestamp discontinuity in % (temporary experimental function, not on all devices)

// use to show fn name, return code, and last error message
void printErrors(const char* fName, int rc) { // ===============================================
    const int ERRMSG_BUFF_SIZE = 512;
    char errorMsg[ERRMSG_BUFF_SIZE];

    pxcGetLastError(errorMsg, ERRMSG_BUFF_SIZE);
    if (errorMsg[0]>0) {
        printf("%s %d err: %s\n", fName, rc, errorMsg);
    } else {
        printf("%s %d err: ---\n", fName, rc);
    }
}

// callback function for data processing, used by pxcMeasureTpx3DataDrivenMode
void onTpx3Data(intptr_t eventData, intptr_t userData) { // ====================================
    int deviceIndex = *((unsigned*)userData);
    unsigned pixelCount = 0;
    unsigned pixelBuffMax = 10000000;
    static unsigned long long pixelSum = 0;
    static int calls=0;
    int rc; // return code
    unsigned n;

    rc = pxcGetMeasuredTpx3PixelsCount(deviceIndex, &pixelCount);
    
    calls++;
    pixelSum += pixelCount;
    if (eventData!=NULL) {
        printf("(rc= %d, eventData=(pointer: %lu, view method not defined)) PixelCount: %u PixelSum: %llu\n", rc, (unsigned long)eventData, pixelCount, pixelSum);
    } else {
        printf("(rc= %d, eventData=NULL) calls: %d pxCnt: %u pxSum: %llu\n", rc, calls, pixelCount, pixelSum);
    }
    
    static Tpx3Pixel pxData[1000000];
    rc = pxcGetMeasuredTpx3Pixels(deviceIndex, pxData, 1000000);
    printErrors("pxcGetMeasuredTpx3Pixels", rc);

    if (calls % 10 == 0 || calls==1) for (n=0; n<pixelCount; n++) {
        printf( "n: %d Pixel: xy: %lu,%lu, ToA=%1.2e, ToT=%g\n", n, pxData[n].index % 256, pxData[n].index / 256, pxData[n].toa, pxData[n].tot);
        if (n>19) {printf("and more pixels...\n"); break;}
    }
}

// test of pxcMeasureTpx3DataDrivenMode - using callback
void timepix3DataDrivenGetPixelsTest(unsigned deviceIndex) { // ================================
    int rc; // return codes
    int sleep_time = 0;
    double measTime = 20;
    int devIdx = deviceIndex; // transmitted over pointer for use in the callback function 

    // working with TOA, TOATOT, TOT_NOTOA, not working with EVENT_ITOT
    // with TOT_NOTOA be carefully for threshold value
    rc = pxcSetTimepix3Mode(deviceIndex, PXC_TPX3_OPM_TOATOT);
    printErrors("pxcSetTimepix3Mode", rc);

    rc = pxcSetDeviceParameter(deviceIndex, PAR_DD_BLOCK_SIZE, 6000);
    printf("pxcSetDeviceParameter %d", rc);
    rc = pxcSetDeviceParameter(deviceIndex, PAR_DD_BUFF_SIZE, 100);
    //printf(", %d", rc);
    //rc = pxcSetDeviceParameter(deviceIndex, PAR_DCC_LEVEL, 80); 
    printErrors(",", rc);

    // First do the sensor refresh: Clean the chip for free charges.
    // In data-driven/callbacks mode, some chips can sometimes stop producing data
    //    in first measurement, if not refreshed before.
    // Alternatively can be used dummy measurement.
    printf("Refreshing sensor...\n");
    rc = pxcDoSensorRefresh(deviceIndex);
    printErrors("pxcDoSensorRefresh", rc);

    rc = pxcMeasureTpx3DataDrivenMode(deviceIndex, measTime, "", PXC_TRG_NO, onTpx3Data, (intptr_t)&devIdx);
    printErrors("pxcMeasureTpx3DataDrivenMode", rc);
}

// test of pxcMeasureTpx3DataDrivenMode - data to file
void timepix3DataDrivenToFileTest(unsigned deviceIndex) { // ===================================
    int rc; // return codes

    // working with TOA, TOATOT, TOT_NOTOA, not working with EVENT_ITOT
    // with TOT_NOTOA be carefully for threshold value
    rc = pxcSetTimepix3Mode(deviceIndex, PXC_TPX3_OPM_TOA);
    printErrors("pxcSetTimepix3Mode", rc);

    // set the block and buffer size
    rc = pxcSetDeviceParameter(deviceIndex, PAR_DD_BLOCK_SIZE, 6000); // in B
    printf("pxcSetDeviceParameter %d", rc);
    rc = pxcSetDeviceParameter(deviceIndex, PAR_DD_BUFF_SIZE, 500); // in MB
    printf(", %d", rc);
    rc = pxcSetDeviceParameter(deviceIndex, PAR_TRG_STG, 3);
    // (0=logical 0, 1 = logical 1, 2 = rising edge, 3 = falling edge)
    printErrors(",", rc);

    //rc = pxcMeasureTpx3DataDrivenMode(deviceIndex, 5, "./output-files/dataDri-test.t3r", PXC_TRG_NO, 0, 0);
    //rc = pxcMeasureTpx3DataDrivenMode(deviceIndex, 5, "./output-files/dataDri-test.t3pa", PXC_TRG_HWSTART, 0, 0); // measurement with trigger
    rc = pxcMeasureTpx3DataDrivenMode(deviceIndex, 5, "./output-files/dataDri-test.t3pa", PXC_TRG_NO, 0, 0);
    printErrors("pxcMeasureTpx3DataDrivenMode", rc);
}

int main (int argc, char const* argv[]) { // ###################################################
    int deviceIndex=0;

    int rc = pxcInitialize();
    if (rc) {
        printf("Could not initialize Pixet:\n");
        printErrors("pxcInitialize", rc);
        return -1;
    }

    int connectedDevicesCount = pxcGetDevicesCount();
    printf("Connected devices: %d\n", connectedDevicesCount);

    if (connectedDevicesCount == 0) return pxcExit();

    for (unsigned devIdx = 0; (signed)devIdx < connectedDevicesCount; devIdx++){
        char deviceName[256];
        for (int n=0; n<256; n++) deviceName[n]=0;
        pxcGetDeviceName(devIdx, deviceName, 256);

        char chipID[256];
        for (int n=0; n<256; n++) chipID[n]=0;
        pxcGetDeviceChipID(devIdx, 0, chipID, 256);
        printf("Device %d: Name %s, (first ChipID: %s)\n", devIdx, deviceName, chipID);
    }
    printf("----------------------------------------\n");
 
    // Different Example Measurements:

    timepix3DataDrivenGetPixelsTest(deviceIndex);
    printf("----------------------------------------\n");
    timepix3DataDrivenToFileTest(deviceIndex);

    return pxcExit(); // Exit Pixet
}
