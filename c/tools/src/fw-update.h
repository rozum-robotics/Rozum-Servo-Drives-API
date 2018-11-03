#ifndef __FW_UPDATE_H__
#define __FW_UPDATE_H__



#define DOWNLOAD_TIMEOUT 15000
#define RESET_TIMEOUT 15000


typedef enum
{
	DL_IDLE,
	DL_DOWNLOADING,
	DL_SUCCESS,
	DL_ERROR
} download_result_t;




#endif