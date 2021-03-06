#include "logging.h"
#include "usbcan_proto.h"
#include "usbcan_types.h"
#include "usbcan_util.h"
#include "loader_proto.h"
#include "crc32.h"
#include "util.h"

#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#define DIR_SEPARATOR "\\"
#else
#define DIR_SEPARATOR "/"
#endif


#define DOWNLOAD_TIMEOUT 15000
#define RESET_TIMEOUT 15000

typedef enum
{
	DL_IDLE,
	DL_DOWNLOADING,
	DL_SUCCESS,
	DL_WRONG_IDENT,
	DL_ERROR
} download_result_t;


download_result_t download_result = DL_IDLE;

char *dir_name = NULL;

FILE *f = NULL;
uint8_t *fw = NULL;
ssize_t fw_len = 0;
ssize_t ptr = 0;
bool use_any_in_boot_mode = false;
bool do_not_wait_halt_responce = true;
bool update_all = false;
bool id_ignore = false;

uint32_t dev_hw_type = -1;
uint32_t alive = 0;
bool master_hb_inhibit = true;
bool read_dev_ident_req = false;
bool do_not_reset = false;

usbcan_instance_t *inst;
usbcan_device_t *dev;

void download_start();
void erase();
void download_start();
void write_block(uint8_t *data, ssize_t *off, ssize_t len);

uint16_t fw_hw_type = 0;
uint16_t fw_hw_rev = 0;

void safe_exit()
{
	if(f)
	{
		fclose(f);
		f = NULL;
	}
	if(fw)
	{
		free(fw);
		fw = NULL;
	}
	usbcan_instance_deinit(&inst);
}

void _nmt_state_cb(usbcan_instance_t *inst, int id, usbcan_nmt_state_t state)
{
	static bool boot_captured = false;
	
	if(use_any_in_boot_mode)
	{
		if(!boot_captured && (state == CO_NMT_BOOT))
		{
			boot_captured = true;
			LOG_INFO(debug_log, "Captured device with ID %d", id);
			dev->id = id;
		}
	}

	if(read_dev_ident_req && (id == dev->id))
	{
		read_dev_ident_req = false;
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

		if(m->data[2] == 0)
		{
			LOG_INFO(debug_log, "Firmware flash region erased");
			ptr = 0;
			write_block(fw, &ptr, 4);
		}
		else
		{
			LOG_ERROR(debug_log, "Error while erasing (%d)", m->data[2]);
			download_result = DL_ERROR;
		}
		return;
	}

	if((m->id == CO_CAN_ID_DEV_CMD) && (m->data[0] == dev->id) &&
			                    (m->data[1] == CO_DEV_CMD_REQUEST_FIELD) && 
								 (m->data[2] == CO_DEV_APP_TYPE))
	{
		
		alive = 0;
		dev_hw_type = m->data[3];
	
		can_msg_t m_read = 
		{
			.id = CO_CAN_ID_DEV_CMD, 
			.dlc = 3, 
			.data = 
			{
				dev->id, 
				CO_DEV_CMD_REQUEST_FIELD,
				CO_DEV_APP_VER
			}
		};
		write_com_frame(inst, &m_read);
		return;

	}


	if((m->id == CO_CAN_ID_DEV_WRITE) && (m->data[0] == dev->id))
	{
		alive = 0;

		if(m->data[1] != 0)
		{
			LOG_ERROR(debug_log, "Error while downloading (%d)", m->data[1]);		
			download_result = DL_ERROR;
			return;
		}

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
			LOG_INFO(debug_log, "Downloading %d/%d", (int)ptr, (int)fw_len);
		}
		write_block(fw, &ptr, 4);
	}


	if((m->id == CO_CAN_ID_DEV_CMD) && (m->data[0] == dev->id) &&
			              ((m->data[1] == CO_DEV_CMD_FLASH_BOOT) || 
						   (m->data[1] == CO_DEV_CMD_FLASH_APP)))
	{
		alive = 0;

		if(m->data[2] == CO_BOOT_STATUS_OK)
		{
			LOG_INFO(debug_log, "FLASH OK\n");
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
			LOG_ERROR(debug_log, "FLASH FAILED");
			download_result = DL_ERROR;
		}
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
			fw_hw_type, 
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

void write_block(uint8_t *data, ssize_t *off, ssize_t len)
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

bool read_ident()
{
	int to;

	dev_hw_type = -1;

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

	for(to = RESET_TIMEOUT; to > 0; to -= 100)
	{
		if(dev_hw_type != -1) 
		{
			break;
		}
		msleep(100);
	}
	if(dev_hw_type == -1) 
	{
		LOG_ERROR(debug_log, "Can't read device HW type");
		download_result = DL_ERROR;
		return false;
	}
	return true;
}

bool reset()
{
	int to;

	read_dev_ident_req = true;

	dev_hw_type = -1;

	LOG_INFO(debug_log, "Resetting device %d", dev->id);
	write_nmt(inst, dev->id, CO_NMT_CMD_RESET_NODE);

	for(to = RESET_TIMEOUT; to > 0; to -= 100)
	{
		if(dev_hw_type != -1)
		{
			break;
		}
		msleep(100);
	}
	if(dev_hw_type == -1)
	{
		LOG_ERROR(debug_log, "Can't read device HW type");
		download_result = DL_ERROR;
		return false;
	}
	return true;
}

download_result_t update(char *name, bool ignore_identity)
{
	uint32_t crc = 0;
	uint32_t file_len, data_len;
	
	download_result = DL_IDLE;
	
	do
	{
		if(usbcan_get_device_state(inst, dev->id) == CO_NMT_BOOT)
		{
			LOG_INFO(debug_log, "Already in bootloader state");
			read_dev_ident_req = true;
			do_not_reset = true;
			_nmt_state_cb(inst, dev->id, CO_NMT_BOOT);	
		}

		if(!ignore_identity)
		{
			LOG_INFO(debug_log, "Checking firmware file '%s' compatibility", name);
			LOG_INFO(debug_log, "Reading firmware identity");
		}

		f = fopen(name, "rb");
		if(!f)
		{
			LOG_ERROR(debug_log, "Can't open file");
			download_result = DL_ERROR;
			break;
		}
		file_len = flen(f);

		if((int)file_len <= 0)
		{
			LOG_ERROR(debug_log, "Empty file");
			download_result = DL_ERROR;
			break;
		}

		data_len = (file_len % 4) ? 4 * (file_len / 4 + 1) : file_len;
		if(data_len != file_len)
		{
			LOG_INFO(debug_log, "Firmware length (%d) not multiple 4 bytes, extending to (%d)", file_len, data_len);
		}
		
		fw_len = data_len + sizeof(crc);
		fw = calloc(fw_len, 1);
			
		if(fread(fw, 1, file_len, f) != file_len)
		{
			LOG_ERROR(debug_log, "fread error");
			download_result = DL_ERROR;
			break;
		}

		if(!do_not_reset)
		{
			if(!reset())
			{
				break;
			}
		}
		else
		{
			if(!read_ident())
			{
				break;
			}
		}

		LOG_INFO(debug_log, "Device HW type %d", dev_hw_type);

		fw_hw_type = fw[8];

		if(!ignore_identity)
		{
			LOG_INFO(debug_log, "Firmware HW type %d", fw_hw_type);
			if(fw_hw_type != dev_hw_type)
			{
				LOG_INFO(debug_log, "Not compatible firmware");
				download_result = DL_WRONG_IDENT;
				break;
			}
		}
		else
		{
			LOG_WARN(debug_log, "Firmware identity ignored!!!");
		}
		
		memcpy(fw + 4, &data_len, sizeof(data_len));
		crc = crc32(fw + 4, data_len - 4);
		memcpy(fw + data_len, &crc, sizeof(crc));

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
	}
	while(0);
	
	if(f)
	{
		fclose(f);
		f = NULL;
	}
	if(fw)
	{
		free(fw);
		fw = NULL;
	}

	return download_result;
}

void usage(char **argv)
{
	fprintf(stdout,	"Usage: %s\n"
			"    [-X(--ignore-ident)\n"
			"    [-M(--master-hb)]\n"
			"    [-B(--use-any-in-boot-mode) yes]\n"
			"    [-v(--version)]\n"
            		"    [-R(--do-not-reset)]\n"
			"    port\n"
			"    id or 'all'\n"
			"    firmware_folder or firmware_file\n"
			"\n",
			argv[0]);
}

bool parse_cmd_line(int argc, char **argv)
{
	int c;
	int option_index = 0;
	static struct option long_options[] = 
	{
		{"use-any-in-boot-mode",     required_argument, 0, 'B' },
		{"ignore-ident",   no_argument, 0, 'X' },
		{"version",   no_argument, 0, 'v' },
		{"master-hb",     no_argument, 0, 'M' },
		{"do-not-reset",     no_argument, 0, 'R' },
		{0,         0,                 0,  0 }
	};


	while (1) 
	{
		c = getopt_long(argc, argv, "B:XMvR", long_options, &option_index);
		if (c == -1)
		{
			break;
		}

		switch (c) 
		{
			case '?':
				break;

			case 'R':
				do_not_reset = true;
				break;

			case 'B':
				if(strcmp(optarg, "yes") == 0)
				{
					use_any_in_boot_mode = true;
					LOG_WARN(debug_log, "First device captured in BOOT mode will be used for flashing!!!");
				}
				else
				{
					LOG_ERROR(debug_log, "Type `yes` if you are brave.");
					return false;
				}
				break;
			case 'X':
				id_ignore = true;
				break;
	
			case 'M':
				LOG_INFO(debug_log, "Enabling master hearbeat");
				master_hb_inhibit = false;
				break;
			case 'v':
				fprintf(stdout, "Build date: %s\nBuild time: %s\n", __DATE__, __TIME__);
				exit(0);
			default:
				break;
		}
	}

	if((argc - optind) < 3)
	{
		return false;
	}

	return true;
}

void batch_update(int id)
{
	DIR *dir;
	char *name = 0;
	struct stat s;
	struct dirent *entry;
	
	dev = usbcan_device_init(inst, id);
	if(!dev)
	{
		LOG_ERROR(debug_log, "Can't create device instance\n");
	}

	if(!wait_device(inst, dev->id, 2000))
	{
		if(!update_all)
		{
			exit(1);
		}
		else
		{
			usbcan_device_deinit(&dev);
			return;
		}
	}


	stat(dir_name, &s);

	if(!S_ISDIR(s.st_mode))
        {
		LOG_INFO(debug_log, "Using explicit file");
		download_result_t r = update(dir_name, id_ignore);
		if(r == DL_ERROR)
		{
			exit(1);
		}
	}
	else
	{
		if(!(dir = opendir(dir_name)))
		{
			LOG_ERROR(debug_log, "Can't open firmware folder in '%s'", dir_name);
			exit(1);
		}



		while((entry = readdir(dir)) != NULL)
		{
			name = realloc(name, strlen(entry->d_name) + strlen(dir_name) + 2);
			sprintf(name, "%s"DIR_SEPARATOR"%s", dir_name, entry->d_name);
			stat(name, &s);
			if(!S_ISDIR(s.st_mode))
			{
				char *dot = strchr(entry->d_name, '.');
				if(dot && (strcmp(dot, ".bin") == 0))
				{
					download_result_t r = update(name, id_ignore);
					if(r == DL_ERROR)
					{
						exit(1);
					}
					if(r == DL_SUCCESS)
					{
						break;
					}
				}
				else
				{
					LOG_WARN(debug_log, "Not a firmware file '%s'", entry->d_name);
				}
			}
		}

		usbcan_device_deinit(&dev);

		if(name)
		{
			free(name);
		}
	}
}

int main(int argc, char **argv)
{
	int id = 0;

	debug_log = stdout;

	if(!parse_cmd_line(argc, argv))
	{
		usage(argv);
		exit(1);
	}

	atexit(safe_exit);

	inst = usbcan_instance_init(argv[optind]);
	if(!inst)
	{
		LOG_ERROR(debug_log, "Can't create usbcan instance\n");
		exit(1);
	}

	if(strcmp(argv[optind + 1], "all") != 0)
	{
		update_all = false;
		id = strtol(argv[optind + 1], 0, 0);
	}
	else
	{
		update_all = true;
	}

	dir_name = argv[optind + 2];

	usbcan_inhibit_master_hb(inst, master_hb_inhibit);

	LOG_INFO(debug_log, "Updating firmware");
	
	if(update_all)
	{		
		LOG_INFO(debug_log, "Discovering devices...");
		msleep(5000);
	}

	usbcan_setup_nmt_state_cb(inst, _nmt_state_cb);
	usbcan_setup_com_frame_cb(inst, _com_frame_cb);
	
	if(update_all)
	{
		for(int i = 0; i < USB_CAN_MAX_DEV; i++)
		{
			if(inst->dev_alive[i] >= 0)
			{
				LOG_INFO(debug_log, "Updating device %d", i);
				batch_update(i);
			}
		}
	}
	else
	{
		batch_update(id);
	}

	return 0;
}

