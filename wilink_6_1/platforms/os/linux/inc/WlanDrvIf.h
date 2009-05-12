/*
 * WlanDrvIf.h
 *
 * Copyright(c) 1998 - 2009 Texas Instruments. All rights reserved.      
 * All rights reserved.                                                  
 *                                                                       
 * Redistribution and use in source and binary forms, with or without    
 * modification, are permitted provided that the following conditions    
 * are met:                                                              
 *                                                                       
 *  * Redistributions of source code must retain the above copyright     
 *    notice, this list of conditions and the following disclaimer.      
 *  * Redistributions in binary form must reproduce the above copyright  
 *    notice, this list of conditions and the following disclaimer in    
 *    the documentation and/or other materials provided with the         
 *    distribution.                                                      
 *  * Neither the name Texas Instruments nor the names of its            
 *    contributors may be used to endorse or promote products derived    
 *    from this software without specific prior written permission.      
 *                                                                       
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS   
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT     
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR 
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT      
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT   
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


/*
 * src/WlanDrvIf.h
 *
 */

#ifndef WLAN_DRV_IF_H
#define WLAN_DRV_IF_H

#include <linux/completion.h>
#include <linux/netdevice.h>
#include <linux/workqueue.h>
#include <mach/gpio.h>

#include "tidef.h"
#include "WlanDrvCommon.h"
#include "paramOut.h"
#include "DrvMain.h"
#include "windows_types.h"

#define TIWLAN_DRV_NAME    "tiwlan"
#define TIWLAN_DRV_IF_NAME TIWLAN_DRV_NAME"%d"


#ifdef TI_DBG
#define ti_dprintf(log, fmt, args...) do { \
   if (log != TIWLAN_LOG_OTHER) {   \
      printk(KERN_INFO fmt, ## args); \
   } \
} while (0)
#else
#define ti_dprintf(log, fmt, args...)
#endif


#define ti_nodprintf(log, fmt, args...)

typedef enum 
{
   TIWLAN_LOG_ERROR,
   TIWLAN_LOG_INFO,
   TIWLAN_LOG_OTHER,
   TIWLAN_LOG_DUMMY
} EWlanDrvIfLog;

/*
 * TCmdRespUnion is defined for each OS:
 * For Linx and WM that defined is empty.
 * For OSE the new typedef includes all "Done" typedefs in union from EMP code (H files).
 */
typedef struct
{
} TCmdRespUnion;


/* Driver object */
typedef struct 
{
    TWlanDrvIfCommon         tCommon;   /* The driver object common part */

    int                      irq;       /* The OS IRQ handle */
    struct work_struct       tWork;     /* The OS work handle. */
    spinlock_t               lock;      /* The OS spinlock handle. */
    unsigned long            flags;     /* For saving the cpu flags during spinlock */
    TI_HANDLE                hPollTimer;/* Polling timer for working without interrupts (debug) */
    struct net_device_stats  stats;     /* The driver's statistics for OS reports. */
    struct sock             *wl_sock;   /* The OS socket used for sending it the driver events */
    struct net_device       *netdev;    /* The OS handle for the driver interface. */

    NDIS_HANDLE		         ConfigHandle;/* Temp - For Windows compatibility */

} TWlanDrvIfObj, *TWlanDrvIfObjPtr;


#define NETDEV(drv) (((TWlanDrvIfObj*)(drv))->netdev)

#endif /* WLAN_DRV_IF_H*/