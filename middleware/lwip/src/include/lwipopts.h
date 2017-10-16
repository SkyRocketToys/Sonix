/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Simon Goldschmidt
 *
 */
#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

/* Minimal changes to opt.h required */
#define MEM_SIZE                        128*1024 
#define TCPIP_THREAD_STACKSIZE          512
#define TCPIP_MBOX_SIZE                 512
#define DEFAULT_TCP_RECVMBOX_SIZE       128
#define DEFAULT_UDP_RECVMBOX_SIZE       128
#define DEFAULT_RAW_RECVMBOX_SIZE 		128
#define DEFAULT_ACCEPTMBOX_SIZE         128
#define MEMP_NUM_NETBUF 				128	

#define MEMP_NUM_TCPIP_MSG_INPKT        128

#define SYS_LIGHTWEIGHT_PROT			1

#define LWIP_DHCP                       1
#define LWIP_DNS						1 
#define LWIP_TIMEVAL_PRIVATE			0
#define LWIP_NETIF_TX_SINGLE_PBUF	 	1

#define LWIP_NOASSERT	                1
#define LWIP_PROVIDE_ERRNO				1

#define TCP_MSS						1460
#define MEMP_NUM_TCP_SEG			192
#define TCP_SND_BUF					65535//(32 * TCP_MSS)
#define TCP_WND						65535//(32 * TCP_MSS)
#define PBUF_POOL_SIZE 		        64 


#define MEMP_NUM_NETCONN			32
#define MEMP_NUM_UDP_PCB			12
#define MEMP_NUM_TCP_PCB			20


#define LWIP_SO_RCVTIMEO			1
#define LWIP_SO_SNDTIMEO            1
/* ------------------------ Memory options -------------------------------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
		lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
		byte alignment -> define MEM_ALIGNMENT to 2. 
*/
#define MEM_ALIGNMENT			4

#endif /* __LWIPOPTS_H__ */
