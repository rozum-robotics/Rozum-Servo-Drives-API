/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __LOADER_PROTO_H__
#define __LOADER_PROTO_H__

#define PARAM_STORE_PASSWORD   ((uint32_t)0x73617665) 
#define PARAM_RESTORE_PASSWORD ((uint32_t)0x6C6F6164) 
#define CO_OD_INDEX_STORE_PARAMETERS                        0x1010
#define CO_OD_SUBINDEX_1_STORE_PARAMETERS_SAVE_ALL_PARAMETERS 1

typedef enum
{
    CO_DEV_ERROR = 0,                   ///< Indicates error, input only, 1 byte of request field
    CO_DEV_FIELD_FIRST=1,
    CO_DEV_HW_ID_0 = 1,                 ///< First part of the device UID, 4 bytes MSB
    CO_DEV_HW_ID_1,                     ///< Second part of the device UID, 4 bytes MSB
    CO_DEV_HW_ID_2,                     ///< Third part of the device UID, 4 bytes MSB
    CO_DEV_BOOT_TYPE,                   ///< Bootloader type, 1 byte, #DeviceIdType_t
    CO_DEV_APP_TYPE,                    ///< Application type, 1 byte, #DeviceIdType_t
    CO_DEV_BOOT_VER,                    ///< Bootloader version, 4 byte MSB
    CO_DEV_APP_VER,                     ///< Application version, 4 byte MSB
    CO_DEV_DEV_MODE,                    ///< Developer mode is activated
    CO_DEV_FIELS_SIZE                    ///< last record, size
}CO_DEV_FIELD_t;

typedef enum {
    CO_DEV_CMD_ERASE_BOOT = 0, ///< Bootloader buffer erase command
    CO_DEV_CMD_ERASE_APP,      ///< Firmware buffer erase command
    CO_DEV_CMD_FLASH_BOOT,     ///< Flash bootloader command
    CO_DEV_CMD_FLASH_APP,      ///< Flash firmware command
    CO_DEV_CMD_EXEC,           ///< Execute main program without timeout
    CO_DEV_CMD_HALT,           ///< Halt bootloader (bootloader will not exec the main program without CO_DEV_CMD_EXEC cmd)
    CO_DEV_CMD_REQUEST_FIELD,  ///< Request field (info)
    CO_DEV_CMD_REQUEST_SDO_RX, ///< Request to any master to read the SDO record
    CO_DEV_CMD_WRITE_FIELD,     ///< Write service field
    CO_DEV_CMD_WRITE_ID_AS_SETTINGS ///< Write current ID as the unique config in the settings
} CO_DEV_CMD_t;

typedef enum {
    CO_BOOT_STATUS_OK = 0,            ///< OK
    CO_BOOT_STATUS_ERR_CRC,           ///< CRC Error
    CO_BOOT_STATUS_ERR_WRONG_APP_ID,  ///< The attempt of the writing wrong ID firmware
    CO_BOOT_STATUS_ERR_LENGTH,        ///< Wrong length
    CO_BOOT_STATUS_ERR_OFFSET,        ///< Wrong firmware offset
    CO_BOOT_STATUS_ERR_ERASE,         ///< Erase error
    CO_BOOT_STATUS_ERR_WRITE,         ///< Write error
    CO_BOOT_STATUS_ERR_WRONG_CMD_ARG, ///< Wrong command argument received
    CO_BOOT_STATUS_WARN_EMPTY,
    CO_BOOT_STATUS_WARN_WRONG_OPERATION
} CO_BOOT_STATUS_t;

enum {
    CO_CAN_ID_NMT_SERVICE = 0x000, /**< 0x000, Network management */
    CO_CAN_ID_SYNC = 0x080,        /**< 0x080, Synchronous message */
    CO_CAN_ID_EMERGENCY = 0x080,   /**< 0x080, Emergency messages (+nodeID) */
    CO_CAN_ID_TIMESTAMP = 0x100,   /**< 0x100, Time stamp message */
    CO_CAN_ID_TPDO_1 = 0x180,      /**< 0x180, Default TPDO1 (+nodeID) */
    CO_CAN_ID_RPDO_1 = 0x200,      /**< 0x200, Default RPDO1 (+nodeID) */
    CO_CAN_ID_TPDO_2 = 0x280,      /**< 0x280, Default TPDO2 (+nodeID) */
    CO_CAN_ID_RPDO_2 = 0x300,      /**< 0x300, Default RPDO2 (+nodeID) */
    CO_CAN_ID_TPDO_3 = 0x380,      /**< 0x380, Default TPDO3 (+nodeID) */
    CO_CAN_ID_RPDO_3 = 0x400,      /**< 0x400, Default RPDO3 (+nodeID) */
    CO_CAN_ID_DEV_READ = 0x400,
    CO_CAN_ID_TPDO_4 = 0x480,      /**< 0x480, Default TPDO4 (+nodeID) */
    CO_CAN_ID_DEV_CMD = 0x480,
    CO_CAN_ID_RPDO_4 = 0x500,      /**< 0x500, Default RPDO5 (+nodeID) */
    CO_CAN_ID_DEV_WRITE = 0x500,
    CO_CAN_ID_TSDO = 0x580,        /**< 0x580, SDO response from server (+nodeID) */
    CO_CAN_ID_RSDO = 0x600,        /**< 0x600, SDO request from client (+nodeID) */
    CO_CAN_ID_TERM = 0x680,
    CO_CAN_ID_HEARTBEAT = 0x700    /**< 0x700, Heartbeat message */
};

enum
{
    CO_SERVICE_FIELD_BOOT_HW_ID = 0,
    CO_SERVICE_FIELD_APP_HW_ID,
    CO_SERVICE_FIELD_APP_REV_ID
};

#endif 
