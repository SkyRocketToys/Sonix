#ifndef __JSON_CMD_LIST_EXT_H__
#define __JSON_CMD_LIST_EXT_H__
#include <generated/snx_sdk_conf.h>

#define CMD_LIST_EXT \
X(setfactorymode, SetFactoryMode, SETFACTORYMODE, set_factory_mode) \
X(setflightresponse, SetFlightResponse, SETFLIGHTRESPONSE, set_flight_response) \
X(getflightdata, GetFlightData, GETFLIGHTDATA, get_flight_data) \
X(getwifistatus, GetWifiStatus, GETWIFISTATUS, get_wifistatus) \
X(disablesdsave, DisableSDSave, DISABLESDSAVE, disable_sd_save) \
X(getflightfwversion, GetFlightFWVersion, GETFLIGHTFWVERSION, get_flight_fwversion)

#endif // __JSON_CMD_LIST_EXT_H__
