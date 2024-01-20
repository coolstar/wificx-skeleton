#include "precomp.h"

#include "trace.h"
#include "power.h"
#include "device.h"
#include "adapter.h"

_Use_decl_annotations_
NTSTATUS
EvtDeviceD0Entry(
    _In_ WDFDEVICE wdfDevice,
    WDF_POWER_DEVICE_STATE previousState)
{
    MTK_ADAPTER* adapter = MtkGetDeviceContext(wdfDevice)->Adapter;

    TraceEntry(TraceLoggingUInt32(previousState, "PreviousState"));

    if (previousState != WdfPowerDeviceD3Final)
    {
        // We're coming back from low power, undo what
        // we did in EvtDeviceD0Exit
    }

    TraceExitResult(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}

_Use_decl_annotations_
NTSTATUS
EvtDeviceD0Exit(
    _In_ WDFDEVICE Device,
    _In_ WDF_POWER_DEVICE_STATE TargetState
)
{
    MTK_ADAPTER* adapter = MtkGetDeviceContext(Device)->Adapter;

    TraceEntry();

    if (TargetState != WdfPowerDeviceD3Final)
    {
        NET_ADAPTER_LINK_STATE linkState;
        NET_ADAPTER_LINK_STATE_INIT(
            &linkState,
            NDIS_LINK_SPEED_UNKNOWN,
            MediaConnectStateUnknown,
            MediaDuplexStateUnknown,
            NetAdapterPauseFunctionTypeUnknown,
            NetAdapterAutoNegotiationFlagNone);

        NetAdapterSetLinkState(adapter->NetAdapter, &linkState);
    }

    TraceExitResult(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}