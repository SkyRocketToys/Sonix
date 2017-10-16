#ifndef __USBD_EXT_H__
#define __USBD_EXT_H__


#if 0
#define USBD_EXT_PRINT(fmt, args...) print_msg("[usbd-ext] "fmt, ##args)
#else
#define USBD_EXT_PRINT(fmt, args...)
#endif



void usbd_ext_init(void);
void usbd_ext_uninit(void);
void usbd_ext_class_switch(int32_t cs_mode, uint32_t options);


#endif
