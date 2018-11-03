#include <getopt.h>
#include <dirent.h>
#include "common/servo_api.h"

#define FLASH_WRITE_TIMEOUT 2000

char *dir_name = NULL;

bool update(char *name)
{
	JSON_Value *root_value;
	JSON_Object *root_object;
	JSON_Array *settings;
	const char *cfg_fw;
	uint32_t cfg_hw, cfg_rev, dev_hw, dev_rev;
	int idx;

	LOG_INFO("Checking settings file '%s' compatibility", name);

	root_value = json_parse_file(name);
	root_object = json_value_get_object(root_value);

	if(!json_object_has_value(root_object, "ID") || !json_object_has_value(root_object, "Settings"))
	{
		LOG_WARN("Wrong file format");
		return false;
	}

	LOG_INFO("Reading config identity");

	cfg_fw = json_object_dotget_string(root_object, "ID.FW");
	cfg_hw = atoi(json_object_dotget_string(root_object, "ID.HW"));
	cfg_rev = atoi(json_object_dotget_string(root_object, "ID.REV"));

	LOG_INFO("Hardware: %"PRIu32", revision: %"PRIu32, cfg_hw, cfg_rev);

	LOG_INFO("Reading device identity");

	dev_hw = sdo_read_uint32_t(&def_cfg, 0x1018, 2);
	dev_rev = sdo_read_uint32_t(&def_cfg, 0x1018, 3);

	LOG_INFO("Hardware: %"PRIu32", revision: %"PRIu32, dev_hw, dev_rev);

	if((cfg_hw != dev_hw) || (cfg_rev != dev_rev))
	{
		LOG_INFO("Not compatible settings");
		return false;
	}

	settings = json_object_get_array(root_object, "Settings");

	for(idx = 0; idx < json_array_get_count(settings); idx++)
	{
		JSON_Object *entry = json_value_get_object(json_array_get_value(settings, idx));

		const char *name = json_object_get_string(entry, "Name");
		uint16_t sdo_idx = json_object_get_number(entry, "Index");
		uint8_t sdo_sidx = json_object_get_number(entry, "SubIndex");
		JSON_Array *jvalue = json_object_get_array(entry, "Value");
		int sz = json_array_get_count(jvalue);
		uint8_t value[sz];

		LOG_INFO("Writing %d byte(s) to '%s' 0x%"PRIx16" %"PRIu8"", sz, name, sdo_idx, sdo_sidx);

		for(int i = 0; i < sz; i++)
		{
			value[i] = json_array_get_number(jvalue, i);
		}

		sdo_write_array_uint8_t(&def_cfg, sdo_idx, sdo_sidx, value, sz, true);
	}

	LOG_INFO("Saving to flash");
	def_cfg.to = FLASH_WRITE_TIMEOUT;
	sdo_write_uint32_t(&def_cfg, CO_OD_INDEX_STORE_PARAMETERS,
                    CO_OD_SUBINDEX_1_STORE_PARAMETERS_SAVE_ALL_PARAMETERS,
					PARAM_STORE_PASSWORD);

	LOG_INFO("Resetting device");
	write_nmt(def_cfg.id, _CO_NMT_CMD_RESET_NODE); 
	LOG_INFO("Sleep 5s to let device to boot");
	msleep(5000);

	return true;
}

