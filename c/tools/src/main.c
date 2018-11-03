#include <getopt.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "logging.h"
#include "cfg-update.h"
#include "fw-update.h"

char *expl_name = NULL;
static bool use_any_in_boot_mode = false;
static bool do_not_wait_halt_responce = true;
bool master_hb_inhibit = true;

usbcan_instance_t *inst;
usbcan_device_t *dev;

void safe_exit()
{
	fw_update_deinit();
	cfg_update_deinit();
	if(dev)
	{
		usbcan_device_deinit(dev);
	}
	
	if(inst)
	{
		usbcan_instace_deinit(inst);
	}
}

void usage(char **argv)
{
	fprintf(stdout,	"Usage: %s\n"
			"    port\n"
			"    id\n"
			"    [-F(--firmware-path) folder or file]\n"
			"    [-C(--config-path) folder or file]\n"
			"    [-X(--ignore-ident) yes]\n"
			"    [-B(--use-any-in-boot-mode) yes]\n"
			"    [-M(--master-hb)]\n"
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
		{"firmware-path",            required_argument, 0, 'F' },
		{"config-path",              required_argument, 0, 'C' },
		{"ignore-ident",             required_argument, 0, 'X' },
		{"master-hb",                optional_argument, 0, 'M' },
		{0, 0, 0, 0}
	};

	while (1) 
	{
		c = getopt_long(argc, argv, "-:B:F:C:X:M", long_options, &option_index);
		if (c == -1)
		{
			break;
		}

		switch (c) 
		{
			case ':':
				return false;
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
				if(strcmp(optarg, "yes") == 0)
				{
					ignore_ident = true;
					LOG_WARN(debug_log, "Identity validation disabled!!! Good luck you brave!!!");
				}
				else
				{
					LOG_ERROR(debug_log, "Type `yes` if you are brave.");
					return false;
				}
				break;
			case 'F':
				fw_path = optarg;
				break;
			case 'C':
				cfg_path = optarg;
				break;				
				break;
			case 'M':
				LOG_INFO(debug_log, "Enabling master hearbeat");
				master_hb_inhibit = false;
				break;
			default:
				break;
		}
	}

	if(!fw_path && !cfg_path)
	{
		return false;
	}

	return true;
}

int main(int argc, char **argv)
{
	DIR *dir;
	struct dirent *entry;
	debug_log = stdout;

	if(!parse_cmd_line(argc, argv))
	{
		usage(argv);
		exit(1);
	}

	atexit(safe_exit);

	inst = usbcan_instance_init(argv[1]);
	if(!inst)
	{
		LOG_ERROR(debug_log, "Can't create usbcan instance\n");
		exit(1);
	}
	dev = usbcan_device_init(inst, strtol(argv[2], 0, 0));
	if(!dev)
	{
		LOG_ERROR(debug_log, "Can't create device instance\n");
	}

	usbcan_inhibit_master_hb(inst, master_hb_inhibit);

	if(!(dir = opendir(dir_name)))
	{
		LOG_ERROR(debug_log, "Can't open firmware folder in '%s'", dir_name);
		exit(1);
	}

	while((entry = readdir(dir)) != NULL)
	{
		struct stat s;
		stat(entry->d_name, &s);
		if(!S_ISDIR(s.st_mode))
		{
			char *dot = strchr(entry->d_name, '.');
			if(dot && (strcmp(dot, ".bin") == 0))
			{
				char name[strlen(entry->d_name) + strlen(dir_name) + 2];
				sprintf(name, "%s/%s", dir_name, entry->d_name);
						
				download_result_t r = update(name, false);
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

	return 0;
}


