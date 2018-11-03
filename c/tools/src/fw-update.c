#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "logging.h"
#include "usbcan_proto.h"
#include "usbcan_types.h"
#include "usbcan_util.h"
#include "loader_proto.h"
#include "crc32.h"

download_result_t download_result = DL_IDLE;

uint32_t dev_hw = -1;
uint32_t alive = 0;

void download_start();

void _nmt_state_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	static bool boot_captured = false;

	if(use_any_in_boot_mode)
	{
		if(!boot_captured && (state == 2))
		{
			boot_captured = true;
			dev->id = id;
		}
	}

	if((dev_hw == -1) && (id == dev->id))
	{
		LOG_INFO(debug_log, "Reading device identity");
		can_msg_t m_read = 
		{
			.id = CO_CAN_ID_DEV_CMD, 
			.dlc = 3, 
			.data = 
			{
				dev->id, 
				CO_DEV_CMD_REQUEST_FIELD,
				CO_DEV_APP_TYPE
			}
		};
		write_com_frame(inst, &m_read);
	}
}

void erase()
{
	LOG_INFO(debug_log, "Device halted");
	can_msg_t m_erase = 
	{
		.id = CO_CAN_ID_DEV_CMD, 
		.dlc = 6, 
		.data = 
		{
			dev->id, 
			CO_DEV_CMD_ERASE_APP, 
			fw[8] /*?????*/, 
			(fw_len >> 16) & 0xff, 
			(fw_len >> 8) & 0xff, 
			fw_len & 0xff
		}
	};
	write_com_frame(inst, &m_erase);
	LOG_INFO(debug_log, "Erasing flash");
}

void download_start()
{
	download_result = DL_DOWNLOADING;
	LOG_INFO(debug_log, "Starting download");
	LOG_INFO(debug_log, "Halting");
	can_msg_t m_halt = 
	{
		.id = CO_CAN_ID_DEV_CMD, 
		.dlc = 2, 
		.data = 
		{
			dev->id, 
			CO_DEV_CMD_HALT
		}
	};
	write_com_frame(inst, &m_halt);
	if(do_not_wait_halt_responce)
	{
		msleep(200);
		erase();
	}
}

void download(uint8_t *data, ssize_t *off, ssize_t len)
{
		can_msg_t m_download = 
		{
			.id = CO_CAN_ID_DEV_WRITE, 
			.dlc = len + 4, 
			.data = 
			{
				dev->id, 
				(*off >> 16) & 0xff, 
				(*off >> 8) & 0xff, 
				*off & 0xff
			}
		};

		memcpy(m_download.data + 4, data + *off, len);
		*off += len;

		write_com_frame(inst, &m_download);
}

void _com_frame_cb(usbcan_instance_t *inst, can_msg_t *m)
{
	if(!do_not_wait_halt_responce)
	{
		if((m->id == CO_CAN_ID_DEV_CMD) && (m->data[0] == dev->id) &&
				(m->data[1] == CO_DEV_CMD_HALT))
		{
			erase();
		}
	}

	if((m->id == CO_CAN_ID_DEV_CMD) &&
			                    ((m->data[1] == CO_DEV_CMD_ERASE_BOOT) || 
								 (m->data[1] == CO_DEV_CMD_ERASE_APP)))
	{
	
		alive = 0;

		LOG_INFO(debug_log, "Firmware flash region erased");
		ptr = 0;

		download(fw, &ptr, 4);
		return;
	}

	if((m->id == CO_CAN_ID_DEV_CMD) && (m->data[0] == dev->id) &&
			                    (m->data[1] == CO_DEV_CMD_REQUEST_FIELD) && 
								 (m->data[2] == CO_DEV_APP_TYPE))
	{
		
		alive = 0;
		dev_hw = m->data[3];
	
		LOG_INFO(debug_log, "Device identity: %d", dev_hw);
		return;

	}

		
	if((m->id == CO_CAN_ID_DEV_WRITE) && (m->data[0] == dev->id))
	{
		alive = 0;

		if(ptr >= fw_len)
		{
			can_msg_t m_flash = 
			{
				.id = CO_CAN_ID_DEV_CMD, 
				.dlc = 2, 
				.data = 
				{
					dev->id, 
					CO_DEV_CMD_FLASH_APP, 
				}
			};
			write_com_frame(inst, &m_flash);

			return;
		}
		
		if((ptr % 1024) == 0)
		{
			LOG_INFO(debug_log, "Downloading %d/%d", ptr, fw_len);
		}
		download(fw, &ptr, 4);
	}


	if((m->id == CO_CAN_ID_DEV_CMD) && (m->data[0] == dev->id) &&
			              ((m->data[1] == CO_DEV_CMD_FLASH_BOOT) || 
						   (m->data[1] == CO_DEV_CMD_FLASH_APP)))
	{
		alive = 0;

		if(m->data[2] == CO_BOOT_STATUS_OK)
		{
			LOG_INFO(debug_log, "FLASH OK");
			can_msg_t m_exec = 
			{
				.id = CO_CAN_ID_DEV_CMD, 
				.dlc = 2, 
				.data = 
				{
					dev->id, 
					CO_DEV_CMD_EXEC, 
				}
			};
			write_com_frame(inst, &m_exec);
			download_result = DL_SUCCESS;
		
		}
		else
		{
			LOG_INFO(debug_log, "FLASH FAILED");
			download_result = DL_ERROR;
		}
	}
}

download_result_t update(char *name, bool id_ignore)
{
	uint32_t fw_hw = 0;
	uint32_t crc = 0;
	int orig_len;
	
	download_result = DL_IDLE;

	if(!id_ignore)
	{
		LOG_INFO(debug_log, "Checking firmware file '%s' compatibility", name);
		LOG_INFO(debug_log, "Reading firmware identity");
	}

	f = fopen(name, "rb");
	if(!f)
	{
		LOG_ERROR(debug_log, "Can't open file");
		return DL_ERROR;
	}
	map_len = flen(f);
	orig_len = map_len;

	if((int)map_len <= 0)
	{
		LOG_ERROR(debug_log, "Empty file");
		return DL_ERROR;
	}

	if(map_len % 4)
	{
		ssize_t new = 4 * (map_len / 4 + 1);
		LOG_INFO(debug_log, "Firmware length (%d) not multiple 4 bytes, extending to (%d)", map_len, new);
		map_len = new;
	}

	map = malloc(map_len);
		
	if(fread(map, 1, orig_len, f) != orig_len)
	{
		LOG_ERROR(debug_log, "fread error");
		exit(1);
	}

	fw_hw = map[8];

	if(!id_ignore)
	{
		LOG_INFO(debug_log, "Firmware identity %d", fw_hw);
		if(fw_hw != dev_hw)
		{
			LOG_INFO(debug_log, "Not compatible firmware");
			return download_result;
		}
	}

	fw = malloc(map_len + sizeof(crc));
	memcpy(fw, map, map_len);
	fw_len = map_len;
	memcpy(fw + 4, &fw_len, sizeof(fw_len));
	crc = crc32(fw + 4, fw_len - 4);
	memcpy(fw + fw_len, &crc, sizeof(crc));
	fw_len += 4;
	LOG_INFO(debug_log, "Firmware CRC: 0x%"PRIx32, crc);

	download_start();

	while((download_result == DL_DOWNLOADING) && (alive < DOWNLOAD_TIMEOUT))
	{
		msleep(100);
		alive += 100;
	}

	if(alive >= DOWNLOAD_TIMEOUT)
	{
		LOG_ERROR(debug_log, "Timeout while downloading");
		download_result = DL_ERROR;
	}

	return download_result;
}



