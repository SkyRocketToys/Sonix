/** \file snx_mid_nvram.h
 * SONiX NVRAM middleware header file, All system configuration management center.
 * \n
 * \author chkuo
 * \date   2015-05-04
 */

#ifndef __SNX_MID_NVRAM_H__
#define __SNX_MID_NVRAM_H__


#ifdef __cplusplus
extern "C"{
#endif

/** \defgroup mid_nvram NVRAM middleware modules
 * \n
 * @{
 */

/** @} */


/** \defgroup debug_macro Debug macros
 * \ingroup mid_nvram
 * \n
 * @{
 */
#define DEBUG_NVRAM 0		/**< NVRAM DEBUG Enable/Disable, 0: Disable, 1: Enable */

#define nvram_msg(fmt, args...) print_msg((fmt), ##args)				/**< NVRAM print message macro */
#define nvram_msg_queue(fmt, args...) print_msg_queue((fmt), ##args)	/**< NVRAM print message queue macro */

/** @} */

/** \defgroup errorcode_macro Error code define
 * \ingroup mid_nvram
 * \n
 * @{
 */
#define NVRAM_SUCCESS               0	/**< Success, no error */
#define NVRAM_E_MEMALLOCFAIL        1	/**< memory allocate fail */
#define NVRAM_E_PKGNOEXIST          2	/**< Pack no exist */
#define NVRAM_E_PKGCRCF             3	/**< Pack crc16 fail */
#define NVRAM_E_PKGNSEMPTY          4	/**< Pack name source is empty (or NULL) */
#define NVRAM_E_SPCENENG            5	/**< NVRAM remain space not enough */
#define NVRAM_E_PKGNEWF             6	/**< Create pack fail */
#define NVRAM_E_IDMAPPINGF          7	/**< Pack_id and config_id no match */
#define NVRAM_E_CFGNOEXIST          8	/**< Config no exist */
#define NVRAM_E_CFGNEWF             9	/**< Create config fail */
#define NVRAM_E_PKGIDNOTFOUND      10	/**< Package id not found */
#define NVRAM_E_DATATYPE           11	/**< data type error */
#define NVRAM_E_DTPTRNULL          12	/**< data point is NULL */
#define NVRAM_E_CFGNSEMPTY         13	/**< Config name source is empty (or NULL) */
#define NVRAM_E_VERNOTFOUND        14	/**< NVRAM Version config data not found */
#define NVRAM_E_NVRAMNYRDY         15	/**< NVRAM Not yet ready */
#define NVRAM_E_FCOMPRESS          16	/**< data compress failed */
#define NVRAM_E_FUNCOMPRESS        17	/**< data uncompress failed */
#define NVRAM_E_CMDNSUPPORT        18	/**< Command not support */

/** @} */

/** \defgroup enum_typedef mp mode enable or not define
 * \ingroup mid_nvram
 * \n
 * @{
 */

typedef enum{
	NVRAM_MP_MODE_DISABLE = 0,
	NVRAM_MP_MODE_ENABLE,
} nvram_mp_mode_enable_t;

/** @} */

/** \defgroup mpmode pkg name and enable name define
 * \ingroup mid_nvram
 * \n
 * @{
 */
#define NVRAM_MP_MODE_PKG_NAME			"MP_MODE"
#define NVRAM_MP_MODE_CFG_ENABLE_NAME	"enable"


/** @} */

/** \defgroup enum_typedef Config data type define
 * \ingroup mid_nvram
 * \n
 * @{
 */
typedef enum{
	NVRAM_DT_STRING = 0,	//!< String (char)
	NVRAM_DT_BIN_RAW,		//!< Binary (file)
	NVRAM_DT_INT,			//!< integer (4 Bytes)
	NVRAM_DT_FLOAT,			//!< float (4 Bytes)
	NVRAM_DT_UINT,			//!< unsigned integer (4 Bytes)
	NVRAM_DT_UCHAR,			//!< unsigned char
} nvram_datatype_t;

/** @} */

/** @} */

/** \defgroup enum_typedef reset to default mode define
 * \ingroup mid_nvram
 * \n
 * @{
 */
typedef enum{
	NVRAM_RTD_ALL = 0,		//!< Reset all
	NVRAM_RTD_PKG,			//!< Reset Pack and all config of this pack.
	NVRAM_RTD_CFG,			//!< Reset config
} nvram_reset_mode_t;

/** @} */

/** \defgroup struct_typedef Access NVRAM data information struct
 * \ingroup mid_nvram
 * \n
 * @{
 */

//!  Access NVRAM data information struct
/*!
*/
typedef struct nvram_data_info_s {
	union {
		//!  len struct of nvram_data_info_s
		/*!
		*/
		unsigned int len;	//!< bitmap type, data length(Bit[31:28]) and data type(Bit[27:0])
		struct {
			unsigned int data_len  : 28;	//!< len[31:28]: data type mode (Please reference nvram_datatype_t), little-endian
			unsigned int data_type : 4;		//!< len[27:0]: data length, little-endian
		};
	};
	void *data;	//!< data of memory point
} nvram_data_info_t;

/** @} */

/** \defgroup struct_typedef NVRAM pack name information struct
 * \ingroup mid_nvram
 * \n
 * @{
 */

//!  NVRAM pack name information struct
/*!
*/
typedef struct nvram_pkg_name_info_s {
	char *name;
	unsigned int id;
} nvram_pkg_name_info_t;

/** @} */

/** \defgroup struct_typedef NVRAM config name and data information struct
 * \ingroup mid_nvram
 * \n
 * @{
 */

//!  NVRAM config name and data information struct
/*!
*/
typedef struct nvram_cfg_name_data_info_s {
	char *name;
	nvram_data_info_t data_info;
} nvram_cfg_name_data_info_t;

/** @} */

/** \defgroup struct_typedef NVRAM binary file information struct
 * \ingroup mid_nvram
 * \n
 * @{
 */

//!  NVRAM binary file information struct
/*!
*/
typedef struct nvram_file_s {
	unsigned int start_addr;	//!< file start address in flash position
	unsigned int offset;		//!< data offet
	unsigned int bsize;			//!< data size
	void *buffer;				//!< data buffer pointer
	void *cfg;					//!< config info pointer
} nvram_file_t;

/** @} */


/** \defgroup func_nvram NVRAM functions
 * \ingroup mid_nvram
 * \n
 * @{
 */

/** \fn int snx_nvram_init(void)
 * \description NVRMA initial function, Only need to execute once during system initialization
 * \n
 * \param : None
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_init(void);

/** \fn int snx_nvram_uninit(void)
 * \description NVRMA uninitial function, Only need to execute once during system un-initialization
 * \n
 * \param : None
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_uninit(void);

/** \fn int snx_nvram_get_immediately(char *pkg_name, char *cfg_name, nvram_data_info_t *data)
 * \description  Get an immediately data from NVRAM
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : data information struct. (include data length, data type...)
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_get_immediately(char *pkg_name, char *cfg_name, nvram_data_info_t *data);

/** \fn int snx_nvram_set_immediately(char *pkg_name, char *cfg_name, nvram_data_info_t *data)
 * \description  Set an immediately data to NVRAM
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : data information struct. (include data length, data type...)
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_set_immediately(char *pkg_name, char *cfg_name, nvram_data_info_t *data);

/** \fn int snx_nvram_open(char *pkg_name)
 * \description  Open an NVRAM pack, Read this pack all data from flash to DDR memory.
 * \n
 * \param pkg_name : pack name string
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_open(char *pkg_name);

/** \fn int snx_nvram_close(char *pkg_name);
 * \description  Close an NVRAM pack
 * \n
 * \param pkg_name : pack name string
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_close(char *pkg_name);

/** \fn int snx_nvram_get(char *pkg_name, char *cfg_name, nvram_data_info_t *data)
 * \description  Get an data from DDR memory
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data: data information struct. (include data length, data type...)
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int nvram_get(char *pkg_name, char *cfg_name, nvram_data_info_t *data);

/** \fn int snx_nvram_set(char *pkg_name, char *cfg_name, nvram_data_info_t *data)
 * \description  Set an data to DDR memory
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data: data information struct. (include data length, data type...)
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int nvram_set(char *pkg_name, char *cfg_name, nvram_data_info_t *data);

/** \fn int snx_nvram_reset(char *pkg_name)
 * \description  Reset NVRAM pack data from flash to DDR memory
 * \n
 * \param pkg_name : pack name string
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_reset(char *pkg_name);

/** \fn int snx_nvram_commit(char *pkg_name)
 * \description  NVRAM pack data save to flash
 * \n
 * \param pkg_name : pack name string
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_commit(char *pkg_name);

/** \fn int snx_nvram_get_data_len(char *pkg_name, char *cfg_name, unsigned int *len)
 * \description  Get an data length from NVRAM, Provide application layer get data
 * \n length first, Help to application allocate memory size.
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param len : data length memory point
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_get_data_len(char *pkg_name, char *cfg_name, unsigned int *len);

/** \fn int snx_nvram_string_set(char *pkg_name, char *cfg_name, char *data)
 * \description  Set an data of string to NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : string data
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_string_set(char *pkg_name, char *cfg_name, char *data);

/** \fn int snx_nvram_string_get(char *pkg_name, char *cfg_name, char *data)
 * \description  Get an data of string from NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : string data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_string_get(char *pkg_name, char *cfg_name, char *data);

/** \fn int snx_nvram_binary_set(char *pkg_name, char *cfg_name, unsigned char *data, unsigned int len)
 * \description  Set an data of binary to NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : binary data
 * \param len : length of binary data
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_binary_set(char *pkg_name, char *cfg_name, unsigned char *data, unsigned int len);

/** \fn int snx_nvram_binary_get(char *pkg_name, char *cfg_name, unsigned char *data)
 * \description  Get an data of binary from NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : binary data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_binary_get(char *pkg_name, char *cfg_name, unsigned char *data);

/** \fn int snx_nvram_integer_set(char *pkg_name, char *cfg_name, int data)
 * \description  Set an data of integer to NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : integer data
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_integer_set(char *pkg_name, char *cfg_name, int data);

/** \fn int snx_nvram_integer_get(char *pkg_name, char *cfg_name, int *data)
 * \description  Get an data of integer from NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : integer data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_integer_get(char *pkg_name, char *cfg_name, int *data);

/** \fn int snx_nvram_float_set(char *pkg_name, char *cfg_name, float data)
 * \description  Set an data of float to NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : float data
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_float_set(char *pkg_name, char *cfg_name, float data);

/** \fn int snx_nvram_float_get(char *pkg_name, char *cfg_name, float *data)
 * \description  Get an data of float from NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : float data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_float_get(char *pkg_name, char *cfg_name, float *data);

/** \fn int snx_nvram_unsign_integer_set(char *pkg_name, char *cfg_name, unsigned int data)
 * \description  Set an data of unsigned integer to NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : unsigned integer data
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_unsign_integer_set(char *pkg_name, char *cfg_name, unsigned int data);

/** \fn int snx_nvram_unsign_integer_get(char *pkg_name, char *cfg_name, unsigned int *data)
 * \description  Get an data of unsigned integer from NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : unsigned integer data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_unsign_integer_get(char *pkg_name, char *cfg_name, unsigned int *data);

/** \fn int snx_nvram_uchar_hex_set(char *pkg_name, char *cfg_name, unsigned char *data, unsigned int len)
 * \description  Set an data of hexadecimal(uchar) to NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : hex data
 * \param len : length of binary data
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_uchar_hex_set(char *pkg_name, char *cfg_name, unsigned char *data, unsigned int len);

/** \fn int snx_nvram_uchar_hex_get(char *pkg_name, char *cfg_name, unsigned char *data)
 * \description  Get an data of hexadecimal(uchar) from NVRAM
 *
 * \n
 * \param pkg_name : pack name string
 * \param cfg_name : config name string
 * \param data : hex data, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_uchar_hex_get(char *pkg_name, char *cfg_name, unsigned char *data);

/** \fn int snx_nvram_get_all_pkg_count(void)
 * \description  Get all pack count
 *
 * \n
 * \param : None
 * \n
 * \return NVRAM all pack count.
 */
inline int snx_nvram_get_all_pkg_count(void);

/** \fn int snx_nvram_get_all_cfg_count(char *pkg_name)
 * \description  Get all config count
 *
 * \n
 * \param pkg_name : pack name string
 * \n
 * \return NVRAM all of config count of pack name.
 */
int snx_nvram_get_all_cfg_count(char *pkg_name);

/** \fn int snx_nvram_get_pkg_name_list(nvram_pkg_name_info_t *pkg_name_info)
 * \description  Get all pack information
 *
 * \n
 * \param pkg_name_info : pack name information struct
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode.
 */
int snx_nvram_get_pkg_name_list(nvram_pkg_name_info_t *pkg_name_info);

/** \fn int snx_nvram_get_cfg_name_list(char *pkg_name, nvram_cfg_name_data_info_t *cfg_name_info)
 * \description  Get all config information
 *
 * \n
 * \param pkg_name : Pack name string
 * \param cfg_name_info : Config name information struct
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode.
 */
int snx_nvram_get_cfg_name_list(char *pkg_name, nvram_cfg_name_data_info_t *cfg_name_info);

/** \fn nvram_file_t* snx_nvram_binary_open(char *pkg_name, char *cfg_name)
 * \description  Open the binary file
 *
 * \n
 * \param pkg_name: pack name string
 * \param cfg_name : config name string
 * \param data : binary data point, this memory must be to allocate first by application.
 * \n
 * \return binary file struct point. if failed return NULL.
 */
nvram_file_t* snx_nvram_binary_open(char *pkg_name, char *cfg_name);

/** \fn int snx_nvram_binary_close(nvram_file_t *filep)
 * \description  Close the binary file
 *
 * \n
 * \param filep: file strunct point
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_binary_close(nvram_file_t *filep);

/** \fn int snx_nvram_binary_seek(nvram_file_t *filep, unsigned int offset)
 * \description  Sets the position the data to a new position.
 *
 * \n
 * \param filep: file strunct point
 * \param data : binary data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_binary_seek(nvram_file_t *filep, unsigned int offset);

/** \fn int snx_nvram_binary_read(nvram_file_t *filep, unsigned int rsize, unsigned char *data)
 * \description  Read block of data from binary file.
 *
 * \n
 * \param filep: file strunct point
 * \param rsize : Read Size (in bytes)
 * \param data : binary data point, this memory must be to allocate first by application.
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
inline int snx_nvram_binary_read(nvram_file_t *filep, int rsize, unsigned char *data);

/** \fn int snx_nvram_reset_to_default(nvram_reset_mode_t, char *pkg_name, char *cfg_name)
 * \description  Reset config data to default
 *
 * \n
 * \param mode: reset to default mode
 * \param pkg_name: pack name string
 * \param cfg_name : config name string
 * \n
 * \return NVRAM_SUCCESS or NVRAM_ErrorCode
 */
int snx_nvram_reset_to_default(nvram_reset_mode_t mode, char *pkg_name, char *cfg_name);

/** @} */

void set_nvram_addr (unsigned int addr, int i);

#ifdef __cplusplus
}
#endif


#endif /*__SNX_MID_NVRAM_H__*/
