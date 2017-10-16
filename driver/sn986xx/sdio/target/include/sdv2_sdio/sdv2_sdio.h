/**
 * @file
 * this is sdio driver header file, include this file before use
 * @author CJ
 */

#ifndef __SDV2_SDIO_H__
#define __SDV2_SDIO_H__

// IO_CURRENT_STATE R5 response
#define STATE_DIS				0x0
#define STATE_CMD				0x1
#define STATE_TRN				0x2
#define STATE_RFU				0x3

#define CISTPL_VERS_1			0x15
#define CISTPL_ALTSTR			0x16
#define CISTPL_MANFID			0x20
#define CISTPL_FUNCID			0x21
#define CISTPL_FUNCE			0x22

/**
 * @brief sdio command structure
 */
struct sdio_command
{
	uint8_t		function;		/**< function number */
	uint8_t		block;			/**< block mode */
	uint8_t		write;			/**< read/write mode */
	uint32_t	addr;			/**< register address */
	uint32_t	count;			/**< Byte/Block count */
	uint32_t	value;			/**< Data Value */
};

/**
 * @brief sdio card tuple table structure
 */
typedef struct tuple_tbl
{
	uint8_t code;				/**< tuple code */
	uint8_t body_size;			/**< tuple body size */
	uint8_t body[64];			/**< tuple body */
}tuple_t;

//=========================================================================
// Return message
//=========================================================================
#define	SDIO_RTN_ERR_FN						-1		/**< define return message*/
#define	SDIO_RTN_ERR_STATE					-2		/**< define return message*/
#define	SDIO_RTN_ERR_FLAG					-3		/**< define return message*/
#define	SDIO_RTN_ERR_CIS_POINTER			-4		/**< define return message*/
#define	SDIO_RTN_ERR_CARD_DETECT			-5		/**< define return message*/
#define	SDIO_RTN_ERR_RSP_CMD7				-7		/**< define return message*/

#define	SDIO_RTN_PASS						1		/**< define return message*/

// =========================================================================
// SDIO Function
// =========================================================================
void sd_sdio_enable(uint32_t isEnable);
void sd_sdio_reset(void);
int sd_sdio_fn_select(uint32_t fun_sel);
int sd_sdio_r5_check(uint8_t state, struct sd_m2_command *cmd_info);
int sd_sdio_cccr_read(void);
int sd_sdio_cis_read(void);
int sd_sdio_sd_identify(void);
void sd_sdio_cmd52(struct sdio_command *sdio_cmd_info, struct sd_m2_command *cmd_info);
void sdio_cmd53(struct sdio_command *sdio_cmd_info, struct sd_m2_command *cmd_info);

#endif