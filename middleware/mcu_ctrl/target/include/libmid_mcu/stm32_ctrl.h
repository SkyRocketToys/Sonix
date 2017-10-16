// ----------------------------------------------------------------------------
// Support for the hex file format.
// ----------------------------------------------------------------------------
#ifndef STM32_CTRL_H
#define STM32_CTRL_H

#define MAX_BLOCK_SIZE 256
#define UART_CLOCK_RATE 10000000
#define UART_BAUD_MAX 115200

typedef struct HexFileData_t
{
	uint32_t address;
	uint16_t bytes;
	uint8_t data[MAX_BLOCK_SIZE]; // Maximum block size of 256 bytes.
	struct HexFileData_t *next; // Next block.
} HexFileData;

typedef struct HexFile_t
{
	HexFileData *data;
	uint32_t length; // Number of blocks of data.
	uint32_t bytes;
	int end_of_file; // Reached the end of file record.
	uint32_t start_linear_address; // Presumably this is the code entry address.
	uint32_t end_address; // Last address found.
	int valid; // Checksums are okay.
} HexFile;

HexFile *read_hex_file(uint8_t *src);

void free_hex_file(HexFile *hex);

// ----------------------------------------------------------------------------
// Support for reading from sd card.
// ----------------------------------------------------------------------------

uint8_t *load_sd_file(char *name);

void free_sd_file(uint8_t *data);

// ----------------------------------------------------------------------------
// Support for flashing the flight board.
// ----------------------------------------------------------------------------

typedef enum ConnectionMode
{
	CleanFlight,
	SoftwareBootloader,
	HardwareBootloader,
} ConnectionMode_t;

void flash_init(
HexFile *hex,
ConnectionMode_t mode,
int bWantLoad);
#endif
