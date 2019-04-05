#ifndef __USBCAN_CONFIG_DOT_H__
#define __USBCAN_CONFIG_DOT_H__


/*----------- protocol specific parameters ---------------*/
#define USB_CAN_STX				2 //packet delimiter
#define USB_CAN_HEAD_SZ				3
#define USB_CAN_CRC_SZ				2
#define USB_CAN_FRAME_TYPE_SZ			1
#define USB_CAN_OHEAD				(USB_CAN_HEAD_SZ + USB_CAN_CRC_SZ)
#define USB_CAN_CAN_MSG_SZ(m)			(USB_CAN_FRAME_TYPE_SZ + (m).dlc + ((m).id & EID_FLAG ? 4 : 2))
#define USB_CAN_COM_FRAME_SZ(m)			(USB_CAN_CAN_MSG_SZ(m) + USB_CAN_OHEAD)
#define USB_CAN_COM_FRAME_MAX_SZ		(4 + 1 + 8 + USB_CAN_OHEAD + USB_CAN_FRAME_TYPE_SZ)
#define USB_CAN_EID_FLAG			(0x40000000u)
#define USB_CAN_MAX_DEV				128


/*---------------- optionlal parameters ------------------*/
#define USB_CAN_MASTER_HB_IVAL_MS		1000
#define USB_CAN_TRAJ_SYNC_IVAL_MS		50
#define USB_CAN_HB_ALIVE_THRESHOLD_MS		3000
#define USB_CAN_MASTER_HB_COM_FRAME_ID		0x700
#define USB_CAN_TRAJ_SYNC_COM_FRAME_ID		0x27F
#define USB_CAN_MAX_PAYLOAD			4096

#define USB_CAN_POLL_GRANULARITY_MS		10
#define USB_CAN_FLUSH_TOUT_MS			1000
#define USB_CAN_REOPEN_DELAY_MS			100

#define USB_CAN_MAX_SDO_PAYLOAD			4096

#define USB_CAN_OUTGOING_UDP_PORT		17701
#define USB_CAN_INGOING_UDP_PORT		17700

#endif
