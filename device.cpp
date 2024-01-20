#include "precomp.h"

#include "trace.h"
#include "device.h"
#include "adapter.h"

NTSTATUS
MtkInitializeHardware(
    _In_ MTK_DEVICE* pDevice,
    _In_ WDFCMRESLIST resourcesRaw,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    TraceEntry();
    //
    // Read the registry parameters
    //
    NTSTATUS status = STATUS_SUCCESS;

    WIFI_DEVICE_CAPABILITIES deviceCapabilities = { 0 };
    deviceCapabilities.Size = sizeof(deviceCapabilities);
    deviceCapabilities.HardwareRadioState = 1;
    deviceCapabilities.SoftwareRadioState = 1;
    deviceCapabilities.ActionFramesSupported = 0;
    deviceCapabilities.NumRxStreams = 1;
    deviceCapabilities.NumTxStreams = 1;
    deviceCapabilities.Support_eCSA = 0;
    deviceCapabilities.MACAddressRandomization = 0;
    deviceCapabilities.BluetoothCoexistenceSupport = WDI_BLUETOOTH_COEXISTENCE_PERFORMANCE_MAINTAINED;
    deviceCapabilities.SupportsNonWdiOidRequests = 0;
    deviceCapabilities.FastTransitionSupported = 0;
    deviceCapabilities.MU_MIMOSupported = 0;
    deviceCapabilities.BSSTransitionSupported = 0;
    deviceCapabilities.SAEAuthenticationSupported = 0;
    deviceCapabilities.MBOSupported = 0;
    deviceCapabilities.BeaconReportsImplemented = 0;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status, 
        WifiDeviceSetDeviceCapabilities(pDevice->FxDevice, &deviceCapabilities));

    DOT11_AUTH_CIPHER_PAIR cipherPairs;
    cipherPairs.AuthAlgoId = DOT11_AUTH_ALGO_80211_OPEN;
    cipherPairs.CipherAlgoId = DOT11_CIPHER_ALGO_NONE;

    WIFI_STATION_CAPABILITIES stationCapabilities = { 0 };
    stationCapabilities.Size = sizeof(stationCapabilities);
    stationCapabilities.ScanSSIDListSize = 32;
    stationCapabilities.DesiredSSIDListSize = 32;
    stationCapabilities.PrivacyExemptionListSize = 16;
    stationCapabilities.KeyMappingTableSize = 16;
    stationCapabilities.DefaultKeyTableSize = 16;
    stationCapabilities.WEPKeyValueMaxLength = 16;
    stationCapabilities.MaxNumPerSTA = 16;
    stationCapabilities.NumSupportedUnicastAlgorithms = 1;
    stationCapabilities.UnicastAlgorithmsList = &cipherPairs;
    stationCapabilities.NumSupportedMulticastDataAlgorithms = 1;
    stationCapabilities.MulticastDataAlgorithmsList = &cipherPairs;
    stationCapabilities.NumSupportedMulticastMgmtAlgorithms = 1;
    stationCapabilities.MulticastMgmtAlgorithmsList = &cipherPairs;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status, 
        WifiDeviceSetStationCapabilities(pDevice->FxDevice, &stationCapabilities));

    WIFI_BAND_INFO bandInfo[2];
    RtlZeroMemory(&bandInfo, sizeof(bandInfo));
    bandInfo[0].BandID = WDI_BAND_ID_2400;
    bandInfo[0].BandState = TRUE;
    bandInfo[0].NumValidPhyTypes = 0;
    bandInfo[0].NumValidChannelTypes = 0;
    bandInfo[0].NumChannelWidths = 0;

    bandInfo[1].BandID = WDI_BAND_ID_5000;
    bandInfo[1].BandState = TRUE;
    bandInfo[1].NumValidPhyTypes = 0;
    bandInfo[1].NumValidChannelTypes = 0;
    bandInfo[1].NumChannelWidths = 0;

    WIFI_BAND_CAPABILITIES bandCapabilities = { 0 };
    bandCapabilities.Size = sizeof(bandCapabilities);
    bandCapabilities.NumBands = 2;
    bandCapabilities.BandInfoList = bandInfo;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status, 
        WifiDeviceSetBandCapabilities(pDevice->FxDevice, &bandCapabilities));

    WIFI_PHY_CAPABILITIES phyCapabilities = { 0 };
    phyCapabilities.Size = sizeof(phyCapabilities);
    phyCapabilities.NumPhyTypes = 0;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiDeviceSetPhyCapabilities(pDevice->FxDevice, &phyCapabilities));

    WIFI_WIFIDIRECT_CAPABILITIES wifiDirectCapabilities = { 0 };
    wifiDirectCapabilities.Size = sizeof(wifiDirectCapabilities);
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiDeviceSetWiFiDirectCapabilities(pDevice->FxDevice, &wifiDirectCapabilities));

Exit:
    TraceExitResult(status);
    return status;
}

_Use_decl_annotations_
NTSTATUS
EvtDevicePrepareHardware(
    _In_ WDFDEVICE device,
    _In_ WDFCMRESLIST resourcesRaw,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    MTK_DEVICE* devContext = MtkGetDeviceContext(device);

    TraceEntry();

    NTSTATUS status = STATUS_SUCCESS;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status, MtkInitializeHardware(devContext, resourcesRaw, resourcesTranslated));

Exit:
    TraceExitResult(status);
    return status;
}

_Use_decl_annotations_
NTSTATUS
EvtDeviceReleaseHardware(
    _In_ WDFDEVICE device,
    _In_ WDFCMRESLIST resourcesTranslated)
{
    UNREFERENCED_PARAMETER(resourcesTranslated);
    MTK_DEVICE* devContext = MtkGetDeviceContext(device);

    TraceEntry();

    NTSTATUS status = STATUS_SUCCESS;
    TraceExitResult(status);
    return status;
}