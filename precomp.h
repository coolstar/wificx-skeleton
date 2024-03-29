#pragma once

#include <ntddk.h>
#include <wdf.h>
#include <ntintsafe.h>
#include <netadaptercx.h>
#include <netadapter.h>
#include <wificx.h>
#include <netiodef.h>

#include <net/checksum.h>
#include <net/logicaladdress.h>
#include <net/gso.h>
#include <net/virtualaddress.h>
#include <net/ieee8021q.h>
#include <net/packethash.h>

#include "forward.h"

// max number of physical fragments supported per TCB
#define MTK_MAX_PHYS_BUF_COUNT 16

#define MTK_MIN_RX_DESC 18
#define MTK_MAX_RX_DESC 1024

#define MTK_MIN_TCB 32
#define MTK_MAX_TCB 128

#define FRAME_CRC_SIZE 4
#define VLAN_HEADER_SIZE 4
#define RSVD_BUF_SIZE 8

// Ethernet Frame Sizes
#define ETHERNET_ADDRESS_LENGTH         6

// packet and header sizes
#define MTK_MAX_PACKET_SIZE (1514)
#define MTK_MAX_FRAME_SIZE  (MTK_MAX_PACKET_SIZE + VLAN_HEADER_SIZE + FRAME_CRC_SIZE)

// maximum link speed for send and recv in bps
#define MTK_MEDIA_MAX_SPEED 2'500'000'000