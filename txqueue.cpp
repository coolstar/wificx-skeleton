#include "precomp.h"

#include "txqueue.h"
#include "device.h"
#include "trace.h"
#include "adapter.h"

_Use_decl_annotations_
NTSTATUS
EvtAdapterCreateTxQueue(
    _In_ NETADAPTER netAdapter,
    _Inout_ NETTXQUEUE_INIT* txQueueInit
)
{
    NTSTATUS status = STATUS_SUCCESS;

    TraceEntryNetAdapter(netAdapter);

    MTK_ADAPTER* adapter = MtkGetAdapterContext(netAdapter);

    WDF_OBJECT_ATTRIBUTES txAttributes;
    WDF_OBJECT_ATTRIBUTES_INIT_CONTEXT_TYPE(&txAttributes, MTK_TXQUEUE);

    txAttributes.EvtDestroyCallback = EvtTxQueueDestroy;

    NET_PACKET_QUEUE_CONFIG txConfig;
    NET_PACKET_QUEUE_CONFIG_INIT(
        &txConfig,
        EvtTxQueueAdvance,
        EvtTxQueueSetNotificationEnabled,
        EvtTxQueueCancel);
    //txConfig.EvtStart = EvtTxQueueStart;
    //txConfig.EvtStop = EvtTxQueueStop;

    const ULONG queueId = NetTxQueueInitGetQueueId(txQueueInit);

    NETPACKETQUEUE txQueue;
    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        NetTxQueueCreate(
            txQueueInit,
            &txAttributes,
            &txConfig,
            &txQueue));

    MTK_TXQUEUE* tx = MtkGetTxQueueContext(txQueue);
    tx->QueueId = queueId;

    NET_EXTENSION_QUERY extension;
    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_CHECKSUM_NAME,
        NET_PACKET_EXTENSION_CHECKSUM_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->ChecksumExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_GSO_NAME,
        NET_PACKET_EXTENSION_GSO_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->GsoExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_PACKET_EXTENSION_IEEE8021Q_NAME,
        NET_PACKET_EXTENSION_IEEE8021Q_VERSION_1,
        NetExtensionTypePacket);

    NetTxQueueGetExtension(txQueue, &extension, &tx->Ieee8021qExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_VIRTUAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetTxQueueGetExtension(txQueue, &extension, &tx->VirtualAddressExtension);

    NET_EXTENSION_QUERY_INIT(
        &extension,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_NAME,
        NET_FRAGMENT_EXTENSION_LOGICAL_ADDRESS_VERSION_1,
        NetExtensionTypeFragment);

    NetTxQueueGetExtension(txQueue, &extension, &tx->LogicalAddressExtension);

    GOTO_IF_NOT_NT_SUCCESS(Exit, status,
        MtkTxQueueInitialize(txQueue, adapter));

Exit:
    TraceExitResult(status);

    return status;
}

NTSTATUS
MtkTxQueueInitialize(
    _In_ NETPACKETQUEUE txQueue,
    _In_ MTK_ADAPTER* adapter
)
{
    NTSTATUS status = STATUS_SUCCESS;

    MTK_TXQUEUE* tx = MtkGetTxQueueContext(txQueue);

    tx->Adapter = adapter;
    tx->Rings = NetTxQueueGetRingCollection(txQueue);

Exit:
    return status;
}

_Use_decl_annotations_
void
EvtTxQueueAdvance(
    _In_ NETPACKETQUEUE txQueue
)
{
    TraceEntry(TraceLoggingPointer(txQueue, "TxQueue"));

    MTK_TXQUEUE* tx = MtkGetTxQueueContext(txQueue);

    TraceExit();
}

_Use_decl_annotations_
void
EvtTxQueueDestroy(
    _In_ WDFOBJECT txQueue
)
{
    MTK_TXQUEUE* tx = MtkGetTxQueueContext(txQueue);

    if (tx->TxdArray)
        WdfObjectDelete(tx->TxdArray);
    tx->TxdArray = NULL;
}

_Use_decl_annotations_
VOID
EvtTxQueueSetNotificationEnabled(
    _In_ NETPACKETQUEUE txQueue,
    _In_ BOOLEAN notificationEnabled
)
{
    TraceEntry(TraceLoggingPointer(txQueue), TraceLoggingBoolean(notificationEnabled));

    MTK_TXQUEUE* tx = MtkGetTxQueueContext(txQueue);

    TraceExit();
}


_Use_decl_annotations_
void
EvtTxQueueCancel(
    _In_ NETPACKETQUEUE txQueue
)
{
    TraceEntry(TraceLoggingPointer(txQueue, "TxQueue"));

    MTK_TXQUEUE* tx = MtkGetTxQueueContext(txQueue);
    MTK_ADAPTER* adapter = tx->Adapter;

    //
    // If the chipset is able to cancel outstanding IOs, then it should do so
    // here. However, the RTL8168D does not seem to support such a feature, so
    // the queue will continue to be drained like normal.
    //

    NET_RING* pr = NetRingCollectionGetPacketRing(tx->Rings);

    while (pr->BeginIndex != pr->EndIndex)
    {
        NET_PACKET* packet = NetRingGetPacketAtIndex(pr, pr->BeginIndex);
        packet->Ignore = 1;

        pr->BeginIndex = NetRingIncrementIndex(pr, pr->BeginIndex);
    }

    NET_RING* fr = NetRingCollectionGetFragmentRing(tx->Rings);
    fr->BeginIndex = fr->EndIndex;

    TraceExit();
}