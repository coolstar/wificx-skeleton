#pragma once

typedef struct _MTK_DEVICE
{
    MTK_ADAPTER* Adapter;
    WDFDEVICE FxDevice;
} MTK_DEVICE, *PMTK_DEVICE;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(MTK_DEVICE, MtkGetDeviceContext);

EVT_WDF_DEVICE_PREPARE_HARDWARE     EvtDevicePrepareHardware;
EVT_WDF_DEVICE_RELEASE_HARDWARE     EvtDeviceReleaseHardware;