/**
 * Copyright (C) 2021 ADVACAM
 * @author      Pavel Hudecek <pavel.hudecek@advacam.com>
 * 
 * Sensor maintenance examples
 */

/* Used API functions:

pxcGetLastError
pxcInitialize
pxcGetDevicesCount
pxcGetDeviceName
pxcGetDeviceChipID

pxcSetSensorRefresh
pxcDoSensorRefresh
pxcEnableSensorRefresh

pxcGetDeviceBadPixelMatrix
pxcGetPixelMaskMatrix
pxcSetPixelMaskMatrix


requires: pxcapi.h, pxcore.dll, minipix.dll, pixet.ini, link with pxcore.lib
*/

#include "pxcapi.h"
#include <cstring>

#define ERRMSG_BUFF_SIZE    512
#define ENTER_ON            true
#define ENTER_OFF           false

// use to show function name, return code, last error message and optional enter
void printErrors(const char* fName, int rc, bool enter) { // ===================================
    char errorMsg[ERRMSG_BUFF_SIZE];
    pxcGetLastError(errorMsg, ERRMSG_BUFF_SIZE);

    if (errorMsg[0]>0) {
        printf("%s %d, err: %s", fName, rc, errorMsg);
    } else {
        printf("%s %d, err: ---", fName, rc);
    }
    if (enter) printf("\n");
}

// count bytes with val in array
void countBytes(char* text, unsigned char array[], unsigned size, unsigned char val) { // ======
    int b=0;

    for (unsigned n=0; n<size; n++) {
        if (array[n]==val) b++;
    }
    printf("%s: %d\n", text, b);
}

// Sensor refresh - sequential changing of bias (suitable values depend on chip manufacturing technology details)
// pxcSetSensorRefresh: set sensor refresh sequence:
//      refresh steps with times [sec] and bias coefficients [1=100%] (physical val. limited to min/max)
//      (devIdx, "time1, coef1; time2, coef2; time3, coef3; ...")
// pxcDoSensorRefresh:
//      do refresh sequence now
// pxcEnableSensorRefresh:
//      enable/disable automatic refresh every defined time [sec] or before every measure (=0)
void SensorRefreshTest() { // ==================================================================
    int rc; // return codes

    rc = pxcSetSensorRefresh(0, "5, 2; 3, 1.5; 1, 1.2; 1, 1");
    printErrors("pxcSetSensorRefresh", rc, ENTER_ON);

    printf("Refreshing sensor...\n");
    rc = pxcDoSensorRefresh(0);
    printErrors("pxcDoSensorRefresh", rc, ENTER_ON);

    rc = pxcEnableSensorRefresh(0, true, 100);
    printErrors("pxcEnableSensorRefresh", rc, ENTER_ON);
}

// read bad pixels matrix on device 0
void pxcGetDeviceBadPixelMatrixTest() { // =====================================================
    int rc; // return codes
    const unsigned size = 65536;
    unsigned char badPixelMatrix[size]; // 1 bad / 0 good (warning: it's oposite to masked pixels)

    rc = pxcGetDeviceBadPixelMatrix(0, badPixelMatrix, size); // nerozpoznan ext. symbol
    printErrors("pxcGetDeviceBadPixelMatrix", rc, ENTER_ON);

    countBytes("Number of bad pixels registered in matrix config", badPixelMatrix, size, 1);
}

// read and write masked pixel matrix on device 0
void pxcGetPixelMaskMatrix_pxcSetPixelMaskMatrixTest() { // ====================================
    int rc; // return codes
    const unsigned size = 65536;
    unsigned char maskMatrix[size]; // 0 masked / 1 no (warning: it's oposite to bad pixels)

    // read mask
    rc= pxcGetPixelMaskMatrix(0, maskMatrix, size);
    printErrors("pxcGetPixelMaskMatrix", rc, ENTER_ON);

    countBytes("Number of masked pixels", maskMatrix, size, 0);

    // add chip edges to the mask
    for (int y=0; y<256; y++) {
        maskMatrix[y*256+0]=0;
        maskMatrix[y*256+255]=0;
    }
    for (int x=0; x<256; x++) maskMatrix[x]=0;
    for (int x=size-256; x<size; x++) maskMatrix[x]=0;

    countBytes("New number of masked pixels", maskMatrix, size, 0);

    // wrire mask back
    rc = pxcSetPixelMaskMatrix(0, maskMatrix, size);
    printErrors("pxcSetPixelMaskMatrix", rc, ENTER_ON);
}

int main (int argc, char const* argv[]) { // ###################################################
    int deviceIndex=0;

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
    printf("----------------------------------------\n");
 
    // examples:

    SensorRefreshTest();
    printf("----------------------------------------\n");
    pxcGetDeviceBadPixelMatrixTest();
    printf("----------------------------------------\n");
    pxcGetPixelMaskMatrix_pxcSetPixelMaskMatrixTest();
    printf("----------------------------------------\n");

    return pxcExit(); // Exit Pixet
}
