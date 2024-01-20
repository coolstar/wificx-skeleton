#include "precomp.h"

#include "trace.h"
#include "device.h"
#include "adapter.h"
#include "wifidevice.h"
#include "power.h"

// {3ABD5B77-B0E2-46CC-8C30-7BFBB83B18F3}
TRACELOGGING_DEFINE_PROVIDER(
    WiFiCxSampleTraceProvider,
    "WiFiCxSample.Trace.Provider",
    (0x3abd5b77, 0xb0e2, 0x46cc, 0x8c, 0x30, 0x7b, 0xfb, 0xb8, 0x3b, 0x18, 0xf3));

EXTERN_C __declspec(code_seg("INIT")) DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_UNLOAD EvtDriverUnload;
EVT_WDF_DRIVER_DEVICE_ADD EvtDriverDeviceAdd;

_Use_decl_annotations_
__declspec(code_seg("INIT"))
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT driverObject,
    _In_ PUNICODE_STRING registryPath)
{
    bool traceLoggingRegistered = false;
    NTSTATUS status = STATUS_SUCCESS;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        TraceLoggingRegister(WiFiCxSampleTraceProvider));

    traceLoggingRegistered = true;

    TraceEntry(TraceLoggingUnicodeString(registryPath));

    WDF_DRIVER_CONFIG driverConfig;
    WDF_DRIVER_CONFIG_INIT(&driverConfig, EvtDriverDeviceAdd);

    driverConfig.DriverPoolTag = 'RTEK';

    driverConfig.EvtDriverUnload = EvtDriverUnload;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfDriverCreate(driverObject, registryPath, WDF_NO_OBJECT_ATTRIBUTES, &driverConfig, NULL));

Exit:

    if (traceLoggingRegistered)
    {
        TraceExitResult(status);

        if (!NT_SUCCESS(status))
        {
            TraceLoggingUnregister(WiFiCxSampleTraceProvider);
        }
    }

    return status;
}

_Use_decl_annotations_
NTSTATUS
#pragma prefast(suppress: __WARNING_EXCESSIVESTACKUSAGE, "TVS:12813961 call stack depth well-defined")
EvtDriverDeviceAdd(
    _In_ WDFDRIVER driver,
    _Inout_ PWDFDEVICE_INIT deviceInit)
{
    UNREFERENCED_PARAMETER((driver));

    TraceEntry();

    NTSTATUS status = STATUS_SUCCESS;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetDeviceInitConfig(deviceInit));

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiDeviceInitConfig(deviceInit));

    WDF_OBJECT_ATTRIBUTES deviceAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&deviceAttributes, MTK_DEVICE);

    {
        WDF_PNPPOWER_EVENT_CALLBACKS pnpPowerCallbacks;
        WDF_PNPPOWER_EVENT_CALLBACKS_INIT(&pnpPowerCallbacks);
        pnpPowerCallbacks.EvtDevicePrepareHardware = EvtDevicePrepareHardware;
        pnpPowerCallbacks.EvtDeviceReleaseHardware = EvtDeviceReleaseHardware;
        pnpPowerCallbacks.EvtDeviceD0Entry = EvtDeviceD0Entry;
        pnpPowerCallbacks.EvtDeviceD0Exit = EvtDeviceD0Exit;

        WdfDeviceInitSetPnpPowerEventCallbacks(deviceInit, &pnpPowerCallbacks);
    }

    /* {
        WDF_POWER_POLICY_EVENT_CALLBACKS powerPolicyCallbacks;
        WDF_POWER_POLICY_EVENT_CALLBACKS_INIT(&powerPolicyCallbacks);

        powerPolicyCallbacks.EvtDeviceArmWakeFromSx = EvtDeviceArmWakeFromSx;
        powerPolicyCallbacks.EvtDeviceDisarmWakeFromSx = EvtDeviceDisarmWakeFromSx;

        WdfDeviceInitSetPowerPolicyEventCallbacks(deviceInit, &powerPolicyCallbacks);
    }*/

    WDFDEVICE wdfDevice;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfDeviceCreate(&deviceInit, &deviceAttributes, &wdfDevice));

    MTK_DEVICE* devContext = MtkGetDeviceContext(wdfDevice);
    devContext->FxDevice = wdfDevice;

    WdfDeviceSetAlignmentRequirement(wdfDevice, FILE_256_BYTE_ALIGNMENT);

    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS idleSettings;
    WDF_DEVICE_POWER_POLICY_IDLE_SETTINGS_INIT(&idleSettings, IdleCannotWakeFromS0);
    idleSettings.UserControlOfIdleSettings = IdleAllowUserControl;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfDeviceAssignS0IdleSettings(wdfDevice, &idleSettings));

    /*WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS wakeSettings;
    WDF_DEVICE_POWER_POLICY_WAKE_SETTINGS_INIT(&wakeSettings);
    wakeSettings.UserControlOfWakeSettings = WakeAllowUserControl;

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WdfDeviceAssignSxWakeSettings(wdfDevice, &wakeSettings));*/

    WIFI_DEVICE_CONFIG wifiDeviceConfig;
    WIFI_DEVICE_CONFIG_INIT(&wifiDeviceConfig,
        WDI_VERSION_LATEST,
        EvtWiFiDeviceSendCommand,
        EvtWiFiDeviceCreateAdapter,
        EvtWifiDeviceCreateWiFiDirectDevice);

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        WifiDeviceInitialize(wdfDevice, &wifiDeviceConfig));

Exit:
    TraceExitResult(status);

    return status;
}

_Use_decl_annotations_
VOID
EvtDriverUnload(
    _In_ WDFDRIVER driver)
{
    UNREFERENCED_PARAMETER((driver));

    TraceEntry();

    TraceLoggingUnregister(WiFiCxSampleTraceProvider);
}