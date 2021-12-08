/**
 * Copyright (C) 2021 ADVACAM
 * @author    Pavel Hudecek <pavel.hudecek@advacam.com>
 * 
 * Other minipix examples
 */

/* Used API functions:

pxcGetLastError
pxcInitialize
pxcGetDevicesCount
pxcGetDeviceName
pxcGetDeviceChipID
pxcGetDeviceChipCount
pxcGetBias
pxcGetBiasRange
pxcGetThreshold
pxcGetThresholdRange
pxcGetDAC
pxcGetTimepixClock

pxcGetTimepixMode
pxcGetMetaDataValue
pxcSetDeviceParameter
pxcGetDeviceParameter
pxcSetTimepix3Mode
pxcIsTimepixCalibrationEnabled

requires: pxcapi.h, common.h, pxcore.dll, minipix.dll, pixet.ini, link with pxcore.lib
*/

#include "pxcapi.h"
#include "common.h"
#include <cstring>

#define PAR_DD_BUFF_SIZE    "DDBuffSize"
#define PAR_DD_BLOCK_SIZE   "DDBlockSize"
#define PAR_TRG_STG         "TrgStg"
#define PAR_DCC_LEVEL       "DataConsystencyCheckLevel"
// (tpx3dev.cpp)

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

// set/read device parameters on device with index 0
// (example parameters related do data-driven measurenments)
void pxcSetDeviceParameterTest() { // ==========================================================
    int rc; // return codes

    // Data Driven Block Size [B], default 66000
    rc = pxcSetDeviceParameter(0, PAR_DD_BLOCK_SIZE, 66000);
    printErrors("pxcSetDeviceParameter", rc, ENTER_ON);

    // Data Driven Buffer Size [MB], default 100
    rc = pxcSetDeviceParameter(0, PAR_DD_BUFF_SIZE, 100);
    printErrors("pxcSetDeviceParameter", rc, ENTER_ON);

    // 0=logical 0, 1 = logical 1, 2 = rising edge, 3 = falling edge
    rc = pxcSetDeviceParameter(0, PAR_TRG_STG, 3);
    printErrors("pxcSetDeviceParameter", rc, ENTER_ON);

    rc = pxcGetDeviceParameter(0, PAR_DD_BUFF_SIZE);
    printErrors("\npxcGetDeviceParameter", rc, ENTER_ON);

    /* Related functions:

    pxcSetDeviceParameterDouble / pxcGetDeviceParameterDouble
    pxcSetDeviceParameterString / pxcGetDeviceParameterString
    */
}

// get device parameters with text names
void pxcGetDeviceParameterTest() { // ==========================================================
    int rc; // return codes
    double dv1, dv2; // double values
    char chr[50];
    
    rc = pxcGetDeviceParameterDouble(0, "TemperatureChip", &dv1);
    printErrors("pxcGetDeviceParameterDouble-chip", rc, ENTER_ON);
    rc = pxcGetDeviceParameterDouble(0, "TemperatureCpu", &dv2);
    printErrors("pxcGetDeviceParameterDouble-cpu", rc, ENTER_ON);
    
    printf("Chip temp: %f\nCPU temp: %f\n", dv1, dv2);
    
    rc = pxcGetDeviceParameterString(0, "HwLibVer", chr, 50);
    printErrors("pxcGetDeviceParameterString", rc, ENTER_ON);
    printf("HWlib ver: %s\n", chr);
}
// set operation mode on device with index 0
// strongly recommended to use before all measurements
// (some devices sometimes measure something without it, some do not or measure something else)
void pxcSetTimepix3ModeTest() { // =============================================================
    int rc; // return codes

    rc = pxcSetTimepix3Mode(0, PXC_TPX3_OPM_TOA);
    printErrors("pxcSetTimepix3Mode", rc, ENTER_ON);

    /* Other TPX3 modes:

    PXC_TPX3_OPM_TOATOT
    PXC_TPX3_OPM_TOA
    PXC_TPX3_OPM_EVENT_ITOT
    PXC_TPX3_OPM_TOT_NOTOA

    related Medipix3 funcions:
    
    pxcSetMedipix3OperationMode
    pxcSetMedipix3GainMode
    */
}

// refresh devices and reconnect device 0 if present
void pxcRefreshDevices_pxcReconnectDevice_Test() { // ==========================================
    int rc; // return codes
    int devCnt;

    rc = pxcRefreshDevices();
    printErrors("pxcRefreshDevices", rc, ENTER_ON);

    devCnt = pxcGetDevicesCount();
    printf("Connected devices: %d\n", devCnt);

    if (devCnt>0) {
        rc = pxcReconnectDevice(0);
        printErrors("pxcReconnectDevice", rc, ENTER_ON);
    }
}

// test if the calibration of Timepix ToT counts to energy in keV is enabled (Medipix2 only)
void pxcIsTimepixCalibrationEnabledTest() { // =================================================
    int rc; // return codes

    rc = pxcIsTimepixCalibrationEnabled(0);
    printErrors("pxcIsTimepixCalibrationEnabled", rc, ENTER_ON);

    // can be changed by pxcSetTimepixCalibrationEnabled
}

// read metadata values from device with index 0 
// (it only works properly while measuring, otherwise it can also cause a problem
// "Frame index exeeds the number of measured frames" in later called functions)
void pxcGetMetaDataValueTest() { // ============================================================
    int rc; // return codes
    char acqTime[100];
    char startTime[100];
    char shutterTime[100];
    memset(acqTime, 0, 100);
    memset(startTime, 0, 100);
    memset(shutterTime, 0, 100);
    u32 dsize = 100;

    rc = pxcGetMetaDataValue(0, 0, "Acq time", acqTime, &dsize);
    printf("pxcGetMetaDataValue %d", rc);
    rc = pxcGetMetaDataValue(0, 0, "Start time", startTime, &dsize);
    printf(", %d", rc);
    rc = pxcGetMetaDataValue(0, 0, "Shutter open time", shutterTime, &dsize);
    printErrors(",", rc, ENTER_ON);

    printf("measure time settings:\n");
    printf( "- TIME Acq time:     %s\n", acqTime );
    printf( "- TIME Start time:   %s\n", startTime );
    printf( "- TIME Shutter time: %s\n", shutterTime );
}

// list all connected devices with parameters
void deviceInfo(int connectedDevicesCount) { // ================================================
    for (unsigned devIdx = 0; (signed)devIdx < connectedDevicesCount; devIdx++){
        char deviceName[256];
        memset(deviceName, 0, 256);
        pxcGetDeviceName(devIdx, deviceName, 256);

        char chipID[256];
        memset(chipID, 0, 256);
        pxcGetDeviceChipID(devIdx, 0, chipID, 256);

        printf("Device %d: Name %s, (first ChipID: %s), chips count: %d\n",
            devIdx, deviceName, chipID, pxcGetDeviceChipCount(devIdx)
        );

        double bias1, bias2;
        pxcGetBias(devIdx, &bias1);
        printf("Bias: %f, ", bias1);
        pxcGetBiasRange(devIdx, &bias1, &bias2);
        printf("min: %f, max: %f\n", bias1, bias2);
        // bias can be changed by pxcSetBias

        double threshold1, threshold2;
        pxcGetThreshold(devIdx, 0, &threshold1);
        printf("Threshold: %f, ", threshold1);
        pxcGetThresholdRange(devIdx, 0, &threshold1, &threshold2);
        printf("min: %f, max: %f\n", threshold1, threshold2);
        // Threshold can be changed by pxcSetThreshold

        unsigned short dacVal;
        pxcGetDAC(devIdx, 0, 0, &dacVal);
        printf("DAC value (chip 0, DAC 0): %d\n", dacVal);
        // DACs can be set by pxcSetDAC

        if ((int)devIdx < connectedDevicesCount-1) printf ("---------------\n");
    }
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

    printf("----------------------------------------\n");
    deviceInfo(connectedDevicesCount);
    printf("----------------------------------------\n");
 
    // Different examples:

    pxcGetDeviceParameterTest();
    printf("----------------------------------------\n");
    pxcSetTimepix3ModeTest();
    printf("----------------------------------------\n");
    pxcSetDeviceParameterTest();
    printf("----------------------------------------\n");
    pxcRefreshDevices_pxcReconnectDevice_Test();
    
    return pxcExit(); // Exit Pixet
}
