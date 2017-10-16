_start:
	cmp	r1, #0xFFFFFFFF
	ldreq sp, =0xEFFFFC
	b main

.globl _TEXT_BASE
_TEXT_BASE:
	.word CONFIG_START
.globl _bss_start
_bss_start:
	.word __bss_start
.globl _bss_end
_bss_end:
	.word __bss_end

.globl SONIXFW_TAG1  
SONIXFW_TAG1:
	.word 0x696e6f73
.globl SONIXFW_TAG2
SONIXFW_TAG2:
	.word 0x00776678 
		

.globl _FLASH_INFO_START
_FLASH_INFO_START:
	.word 0x00000000
.globl FLASH_INFO_LEN
FLASH_INFO_LEN:
	.word 0x00000000
.globl _HW_SETTING_START
_HW_SETTING_START:
	.word 0x00000000
.globl HW_SETTING_LEN
HW_SETTING_LEN:
	.word 0x00000000
.globl _U_BOOT_START
_U_BOOT_START:
	.word 0x00000000
.globl U_BOOT_LEN
U_BOOT_LEN:
	.word 0x00000000
.globl _U_ENV_START
_U_ENV_START:
	.word 0x00000000
.globl U_ENV_LEN
U_ENV_LEN:
	.word 0x00000000	
.globl _FLASH_LAYOUT_START
_FLASH_LAYOUT_START:
	.word 0x00000000
.globl FLASH_LAYOUT_LEN
FLASH_LAYOUT_LEN:
	.word 0x00000000
.globl _FACTORY_START
_FACTORY_START:
	.word 0x00000000
.globl FACTORY_LEN
FACTORY_LEN:
	.word 0x00000000
.globl _U_LOGO_START
_U_LOGO_START:
	.word 0x00000000
.globl U_LOGO_LEN
U_LOGO_LEN:
	.word 0x00000000		
.globl _RESCUE_START
_RESCUE_START:
	.word 0x00000000
.globl RESCUE_LEN
RESCUE_LEN:
	.word 0x00000000

.globl jump2
jump2:
	MOV		pc, r0
	
