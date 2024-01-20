#pragma once

typedef struct _MTK_ADAPTER
{
    // WDF handles associated with this context
    NETADAPTER NetAdapter;
    WDFDEVICE WdfDevice;

    // spin locks
    WDFSPINLOCK Lock;

    WDFDMAENABLER DmaEnabler;

    NET_ADAPTER_LINK_LAYER_ADDRESS PermanentAddress;
    NET_ADAPTER_LINK_LAYER_ADDRESS CurrentAddress;
} MTK_ADAPTER, * PMTK_ADAPTER;

WDF_DECLARE_CONTEXT_TYPE_WITH_NAME(MTK_ADAPTER, MtkGetAdapterContext);

EVT_NET_ADAPTER_CREATE_TXQUEUE   EvtAdapterCreateTxQueue;
EVT_NET_ADAPTER_CREATE_RXQUEUE   EvtAdapterCreateRxQueue;

NTSTATUS
MtkInitializeAdapterContext(
    _In_ MTK_ADAPTER* adapter,
    _In_ WDFDEVICE device,
    _In_ NETADAPTER netAdapter);

NTSTATUS
MtkAdapterStart(
    _In_ MTK_ADAPTER* adapter);