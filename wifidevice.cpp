#include "precomp.h"
#include "trace.h"
#include "device.h"
#include "wifidevice.h"
#include "adapter.h"

NTSTATUS
MtkRegisterScatterGatherDma(
    _In_ MTK_ADAPTER* adapter)
{
    TraceEntryMtkAdapter(adapter);

    WDF_DMA_ENABLER_CONFIG dmaEnablerConfig;
    WDF_DMA_ENABLER_CONFIG_INIT(&dmaEnablerConfig, WdfDmaProfileScatterGather64, MTK_MAX_PACKET_SIZE);
    dmaEnablerConfig.Flags |= WDF_DMA_ENABLER_CONFIG_REQUIRE_SINGLE_TRANSFER;
    dmaEnablerConfig.WdmDmaVersionOverride = 3;

    NTSTATUS status = STATUS_SUCCESS;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfDmaEnablerCreate(
            adapter->WdfDevice,
            &dmaEnablerConfig,
            WDF_NO_OBJECT_ATTRIBUTES,
            &adapter->DmaEnabler),
        TraceLoggingMtkAdapter(adapter));

Exit:

    TraceExitResult(status);
    return status;
}

_Use_decl_annotations_
NTSTATUS
EvtWiFiDeviceCreateAdapter(
	_In_ WDFDEVICE WdfDevice,
	_Inout_ NETADAPTER_INIT* AdapterInit
)
{
    TraceEntry();

    NTSTATUS status = STATUS_SUCCESS;

    GOTO_WITH_INSUFFICIENT_RESOURCES_IF_NULL(Exit, status, AdapterInit);

    NET_ADAPTER_DATAPATH_CALLBACKS datapathCallbacks;
    NET_ADAPTER_DATAPATH_CALLBACKS_INIT(
        &datapathCallbacks,
        EvtAdapterCreateTxQueue,
        EvtAdapterCreateRxQueue);

    NetAdapterInitSetDatapathCallbacks(
        AdapterInit,
        &datapathCallbacks);

    WDF_OBJECT_ATTRIBUTES adapterAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&adapterAttributes, MTK_ADAPTER);

    NETADAPTER netAdapter;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetAdapterCreate(AdapterInit, &adapterAttributes, &netAdapter));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiAdapterInitialize(netAdapter));

    MTK_ADAPTER* adapter = MtkGetAdapterContext(netAdapter);
    MTK_DEVICE* device = MtkGetDeviceContext(WdfDevice);

    device->Adapter = adapter;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        MtkInitializeAdapterContext(adapter, WdfDevice, netAdapter));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        MtkRegisterScatterGatherDma(adapter),
        TraceLoggingMtkAdapter(adapter));

    {
        //Temp for barebones init

        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            NDIS_LINK_SPEED_UNKNOWN,
            MediaConnectStateDisconnected,
            MediaDuplexStateUnknown,
            NetAdapterPauseFunctionTypeUnknown,
            NetAdapterAutoNegotiationFlagNone);
        NetAdapterSetLinkState(adapter->NetAdapter, &linkState);
    }

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        MtkAdapterStart(adapter));

Exit:
    TraceExitResult(status);

    return status;
}

_Use_decl_annotations_
void
EvtWiFiDeviceSendCommand(
    _In_ WDFDEVICE WdfDevice,
    _Inout_ WIFIREQUEST Request
)
{
    TraceEntry();

    UNREFERENCED_PARAMETER(WdfDevice);
    
    UINT16 MessageID = WifiRequestGetMessageId(Request);
    UINT InputBufferLen, OutputBufferLen;
    WifiRequestGetInOutBuffer(Request, &InputBufferLen, &OutputBufferLen);

    TraceLoggingWrite(WiFiCxSampleTraceProvider,
        "WiFiCommand",
        TraceLoggingHexInt16(MessageID),
        TraceLoggingUInt32(InputBufferLen),
        TraceLoggingUInt32(OutputBufferLen));

    DbgPrint("Command: 0x%x, Input Len: %d, Output Len: %d\n", MessageID, InputBufferLen, OutputBufferLen);

    WifiRequestComplete(Request, STATUS_NOT_IMPLEMENTED, 0);

Exit:
    TraceExit();
}

_Use_decl_annotations_
NTSTATUS
EvtWifiDeviceCreateWiFiDirectDevice(
    _In_ WDFDEVICE Device,
    _In_ WIFIDIRECT_DEVICE_INIT* WfdDeviceInit
)
{
    TraceEntry();

    NTSTATUS status;

    WDF_OBJECT_ATTRIBUTES wfdDeviceAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&wfdDeviceAttributes, MTK_WIFIDIRECTDEVICE);

    WIFIDIRECTDEVICE wfdDevice;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiDirectDeviceCreate(WfdDeviceInit, &wfdDeviceAttributes, &wfdDevice));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiDirectDeviceInitialize(wfdDevice));

    PMTK_WIFIDIRECTDEVICE wfdContext = MtkGetWifiDirectContext(wfdDevice);
    wfdContext->WdfDevice = Device;
    wfdContext->WifiDirectDevice = wfdDevice;

Exit:
    TraceExitResult(status);

    return status;
}