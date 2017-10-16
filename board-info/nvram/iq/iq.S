
#include "generated/snx_sdk_conf.h"
#include "sensor.h"

	.code 32
	.text

	.align 5
#ifdef CONFIG_MODULE_SENSOR_IMX322
	.ascii "imx322\0\0"
	.long __imx322_start
	.long __imx322_end
#endif
#ifdef CONFIG_MODULE_SENSOR_OV9715
	.ascii "ov9715\0\0"
	.long __ov9715_start
	.long __ov9715_end
#endif
#ifdef CONFIG_MODULE_SENSOR_OV2715
	.ascii "ov2715\0\0"
	.long __ov2715_start
	.long __ov2715_end
#endif
#ifdef CONFIG_MODULE_SENSOR_IMX238
	.ascii "imx238\0\0"
	.long __imx238_start
	.long __imx238_end
#endif
#ifdef CONFIG_MODULE_SENSOR_IMX225
	.ascii "imx225\0\0"
	.long __imx225_start
	.long __imx225_end
#endif
#ifdef CONFIG_MODULE_SENSOR_AR0330
	.ascii "ar0330\0\0"
	.long __ar0330_start
	.long __ar0330_end
#endif
#ifdef CONFIG_MODULE_SENSOR_TW9912
	.ascii "tw9912\0\0"
	.long __tw9912_start
	.long __tw9912_end
#endif
#ifdef CONFIG_MODULE_SENSOR_CS1211K
	.ascii "cs1211k\0"
	.long __cs1211k_start
	.long __cs1211k_end
#endif
#ifdef CONFIG_MODULE_SENSOR_H42
	.ascii "h42\0\0\0\0\0"
	.long __h42_start
	.long __h42_end
#endif
#ifdef CONFIG_MODULE_SENSOR_H62
	.ascii "h62\0\0\0\0\0"
	.long __h62_start
	.long __h62_end
#endif
#ifdef CONFIG_MODULE_SENSOR_GC1024
        .ascii "gc1024\0\0"
        .long __gc1024_start
        .long __gc1024_end
#endif
#ifdef CONFIG_MODULE_SENSOR_OV9732
        .ascii "ov9732\0\0"
        .long __ov9732_start
        .long __ov9732_end
#endif
#ifdef CONFIG_MODULE_SENSOR_SC2035
        .ascii "sc2035\0\0"
        .long __sc2035_start
        .long __sc2035_end
#endif
#ifdef CONFIG_MODULE_SENSOR_AR0237
        .ascii "ar0237\0\0"
        .long __ar0237_start
        .long __ar0237_end
#endif
#ifdef CONFIG_MODULE_SENSOR_HM2130
        .ascii "hm2130\0\0"
        .long __hm2130_start
        .long __hm2130_end
#endif
#ifdef CONFIG_MODULE_SENSOR_OV2740
        .ascii "ov2740\0\0"
        .long __ov2740_start
        .long __ov2740_end
#endif
#ifdef CONFIG_MODULE_SENSOR_OV7725
        .ascii "ov7725\0\0"
        .long __ov7725_start
        .long __ov7725_end
#endif
#ifdef CONFIG_MODULE_SENSOR_GC0308
        .ascii "gc0308\0\0"
        .long __gc0308_start
        .long __gc0308_end
#endif

	.ascii "\0\0\0\0\0\0\0\0"
	.long 0x0
	.long 0x0


#ifdef CONFIG_MODULE_SENSOR_IMX322
	.align 5
__imx322_start:
	.incbin "imx322/IQ.bin"
__imx322_end:
#if (RES_IQ_SIZE > imx322_SIZE)
	.space (RES_IQ_SIZE - imx322_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_OV9715
	.align 5
__ov9715_start:
	.incbin "ov9715/IQ.bin"
__ov9715_end:
#if (RES_IQ_SIZE > ov9715_SIZE)
	.space (RES_IQ_SIZE - ov9715_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_OV2715
	.align 5
__ov2715_start:
	.incbin "ov2715/IQ.bin"
__ov2715_end:
#if (RES_IQ_SIZE > ov2715_SIZE)
	.space (RES_IQ_SIZE - ov2715_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_IMX238
	.align 5
__imx238_start:
	.incbin "imx238/IQ.bin"
__imx238_end:
#if (RES_IQ_SIZE > imx238_SIZE)
	.space (RES_IQ_SIZE - imx238_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_IMX225
	.align 5
__imx225_start:
	.incbin "imx225/IQ.bin"
__imx225_end:
#if (RES_IQ_SIZE > imx225_SIZE)
	.space (RES_IQ_SIZE - imx225_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_AR0330
	.align 5
__ar0330_start:
	.incbin "ar0330/IQ.bin"
__ar0330_end:
#if (RES_IQ_SIZE > ar0330_SIZE)
	.space (RES_IQ_SIZE - ar0330_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_TW9912
	.align 5
__tw9912_start:
	.incbin "tw9912/IQ.bin"
__tw9912_end:
#if (RES_IQ_SIZE > tw9912_SIZE)
	.space (RES_IQ_SIZE - tw9912_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_CS1211K
	.align 5
__cs1211k_start:
	.incbin "cs1211k/IQ.bin"
__cs1211k_end:
#if (RES_IQ_SIZE > cs1211k_SIZE)
	.space (RES_IQ_SIZE - cs1211k_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_H42
	.align 5
__h42_start:
	.incbin "h42/IQ.bin"
__h42_end:
#if (RES_IQ_SIZE > h42_SIZE)
	.space (RES_IQ_SIZE - h42_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_H62
	.align 5
__h62_start:
	.incbin "h62/IQ.bin"
__h62_end:
#if (RES_IQ_SIZE > h62_SIZE)
	.space (RES_IQ_SIZE - h62_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_GC1024
        .align 5
__gc1024_start:
        .incbin "gc1024/IQ.bin"
__gc1024_end:
#if (RES_IQ_SIZE > gc1024_SIZE)
        .space (RES_IQ_SIZE - gc1024_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_OV9732
        .align 5
__ov9732_start:
        .incbin "ov9732/IQ.bin"
__ov9732_end:
#if (RES_IQ_SIZE > ov9732_SIZE)
        .space (RES_IQ_SIZE - ov9732_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_SC2035
        .align 5
__sc2035_start:
        .incbin "sc2035/IQ.bin"
__sc2035_end:
#if (RES_IQ_SIZE > sc2035_SIZE)
        .space (RES_IQ_SIZE - sc2035_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_AR0237
        .align 5
__ar0237_start:
        .incbin "ar0237/IQ.bin"
__ar0237_end:
#if (RES_IQ_SIZE > ar0237_SIZE)
        .space (RES_IQ_SIZE - ar0237_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_HM2130
        .align 5
__hm2130_start:
        .incbin "hm2130/IQ.bin"
__hm2130_end:
#if (RES_IQ_SIZE > hm2130_SIZE)
        .space (RES_IQ_SIZE - hm2130_SIZE), 0xff
#endif
#endif

#ifdef CONFIG_MODULE_SENSOR_OV2740
        .align 5
__ov2740_start:
        .incbin "ov2740/IQ.bin"
__ov2740_end:
#if (RES_IQ_SIZE > ov2740_SIZE)
        .space (RES_IQ_SIZE - ov2740_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_OV7725
        .align 5
__ov7725_start:
        .incbin "ov7725/IQ.bin"
__ov7725_end:
#if (RES_IQ_SIZE > ov7725_SIZE)
        .space (RES_IQ_SIZE - ov7725_SIZE), 0xff
#endif
#endif
#ifdef CONFIG_MODULE_SENSOR_GC0308
        .align 5
__gc0308_start:
        .incbin "gc0308/IQ.bin"
__gc0308_end:
#if (RES_IQ_SIZE > gc0308_SIZE)
        .space (RES_IQ_SIZE - gc0308_SIZE), 0xff
#endif
#endif

	.end
