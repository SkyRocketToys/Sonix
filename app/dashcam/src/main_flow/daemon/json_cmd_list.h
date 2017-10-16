#ifndef __JSON_CMD_LIST_H__
#define __JSON_CMD_LIST_H__

#include "json_cmd_list_ext.h"

#define CMD_LIST \
X(heartbeat, HeartBeat, HEARTBEAT, heartbeat) \
X(setchannel, SetChannel, SETCHANNEL, set_channel) \
X(setpwd, SetPwd, SETPWD, set_pwd) \
X(setssid, SetSsid, SETSSID, set_ssid) \
X(setwifiparameters, Setwifiparameters, SETWIFIPARAMETERS, set_wifiparameters) \
X(setwdr, SetWdr, SETWDR, set_wdr) \
X(setmirror, SetMirror, SETMIRROR, set_mirror) \
X(setflip, SetFlip, SETFLIP, set_flip) \
X(getvideostatus, GetVideoStatus, GETVIDEOSTATUS, get_videostatus) \
X(setvideoparameters, SetVideoParameters, SETVIDEOPARMETER, set_videoparameters) \
X(setvideobitrate, SetVideoBitrate, SETVIDEOBITRATE, set_videobitrate) \
X(setvideofps, SetVideoFPS, SETVIDEOFPS, set_videofps) \
X(setvideoresolution, SetVideoResolution, SETVIDEORESOLUTION, set_videoresolution) \
X(takepicture, TakePicture, TAKEPICTURE, takepicture) \
X(getsdspace, GetSDSpace, GETSDSPACE, get_sdspace) \
X(getsdformat, GetSDFormat, GETSDFORMAT, get_format_status) \
X(setsdformat, SetSDFormat, SETSDFORMAT, set_sdformat) \
X(getsdtest, GetSDTest, GETSDTEST, get_sdtest_status) \
X(setsdtest, SetSDTest, SETSDTEST, set_sdtest) \
X(setrecordstatus, SetRecordStatus, SETRECORDSTATUS, set_recordstatus) \
X(getrecordstatus, GetRecordStatus, GETRECORDSTATUS, get_recordstatus) \
X(setrecordaudiostatus, SetRecordAudioStatus, SETRECORDAUDIOSTATUS, set_recordaudiostatus) \
X(setrecordlength, SetRecordLength, SETRECORDLENGTH, set_recordlength) \
X(setrecordparameters, SetRecordParameters, SETRECORDPARAMETERS, set_recordparameters) \
X(setlooprecordstatus, SetLoopRecordStatus, SETLOOPRECORDSTATUS, set_looprecordstatus) \
X(getrecordcapability, GetRecordCapability, GETRECORDCAPABILITY, get_recordcapability) \
X(setprotectlength, SetProtectLength, SETPROTECTLENGTH, set_protectlength) \
X(synctime, SyncTime, SYNCTIME, synctime) \
X(gettime, GetTime, GETTIME, get_time) \
X(getiqversion, GetIQVersion, GETIQVERSION, get_iqversion) \
X(getbatterystatus, GetBatteryStatus, GETBATTERYSTATUS, get_batterystatus) \
X(getdeviceparameter, GetDeviceParameter, GETDEVICEPARAMETER, get_deviceparameter) \
X(setpowerfrequency, SetPowerFrequency, SETPOWERFREQUENCY, set_powerfrequency) \
X(setgsensorparamter, SetGSensorParamter, SETGSENSORPARAMTER, set_gsensorparamter) \
X(nvramresettodefault, NvramResetToDefault, NVRAMRESETTODEFAULT, nvramresettodefault) \
X(usbdclassmode, UsbDClassMode, USBDCLASSMODE, usbdclassmode) \
X(getvideolist, GetVideoList, GETVIDEOLIST, get_videolist) \
X(getindexfile, GetIndexFile, GETINDEXFILE, get_indexfile) \
X(downloadfilestart, DownloadFileStart, DOWNLOADFILESTART, downloadfilestart) \
X(downloadfile, DownloadFile, DOWNLOADFILE, downloadfile) \
X(downloadfilefinish, DownloadFileFinish, DOWNLOADFILEFINISH, downloadfilefinish) \
X(deletefile, DeleteFile, DELETEFILE, deletefile) \
X(streamvideo, StreamVideo, STREAMVIDEO, streamvideo) \
X(streamvideofinish, StreamVideoFinish, STREAMVIDEOFINISH, streamvideofinish) \
X(streamuvc, StreamUvc, STREAMUVC, streamuvc) \
X(sendfontfile, SendFontFile, SENDFONTFILE, sendfontfile) \
X(sendfwbin, SendFWBin, SENDFWBIN, sendfwbin) \
X(upgradefw, UpgradeFW, UPGRADEFW, upgradefw) \
X(gettimelapsevideolist, GetTimelapseVideoList, GETTIMELAPSEVIDEOLIST, get_timelapsevideolist) \
X(getosdstatus, GetOSDStatus, GETOSDSTATUS, get_osdstatus) \
X(setosdonoff, SetOSDOnOff, SETOSDONOFF, set_osdonoff) \
X(setplaybackseek, SetPlayBackSeek, SETPLAYBACKSEEK, set_playbackseek) \
X(notifyrtspstop, NotifyRTSPStop, NOTIFYRTSPSTOP, notifyrtspstop)

#endif // __JSON_CMD_LIST_H__
