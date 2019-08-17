/**
 * Copyright (C) 2017-2018, Decawave Limited, All Rights Reserved
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_bsp.h"
#ifdef ARCH_sim
#include "mcu/mcu_sim.h"
#endif

#include <dw1000/dw1000_dev.h>
#include <dw1000/dw1000_hal.h>
#include <dw1000/dw1000_phy.h>
#include <dw1000/dw1000_mac.h>
#include <dw1000/dw1000_ftypes.h>
#include <rng/rng.h>

#include <tdma/tdma.h>
#include <ccp/ccp.h>
#if MYNEWT_VAL(DW1000_LWIP)
#include <lwip/lwip.h>
#endif

//#define DIAGMSG(s,u) printf(s,u)
#ifndef DIAGMSG
#define DIAGMSG(s,u)
#endif


static bool error_cb(dw1000_dev_instance_t * inst, dw1000_mac_interface_t * cbs);
static void slot_complete_cb(struct os_event * ev);

/*! 
 * @fn slot_cb(struct os_event * ev)
 *
 * @brief In this example slot_cb is used to initiate a range request. The slot_cb is scheduled 
 * MYNEWT_VAL(OS_LATENCY) in advance of the transmit epoch and a delayed start request is issued in advance of 
 * the required epoch. The transmission timing is controlled precisely by the DW1000 with the transmission time 
 * defined by the value of the dw_time variable. If the OS_LATENCY value is set too small the range request 
 * function will report a start_tx_error. In a synchronized network, the node device switches the transceiver 
 * to receiver mode for the same epoch; and will either receive the inbound frame or timeout after the frame 
 * duration as elapsed. This ensures that the transceiver is in receive mode for the minimum time required.   
 *
 * input parameters
 * @param inst - struct os_event *  
 *
 * output parameters
 *
 * returns none 
 */
static void 
slot_cb(struct os_event *ev){
    assert(ev);
    tdma_slot_t * slot = (tdma_slot_t *) ev->ev_arg;
    tdma_instance_t * tdma = slot->parent;
    dw1000_dev_instance_t * inst = tdma->dev_inst;
    uint16_t idx = slot->idx;
    dw1000_rng_instance_t *rng = (dw1000_rng_instance_t*)slot->arg;

    hal_gpio_toggle(LED_BLINK_PIN);  
    uint64_t dx_time = tdma_tx_slot_start(tdma, idx) & 0xFFFFFFFFFE00UL;
  
    /* Range with the clock master by default */
    dw1000_ccp_instance_t *ccp = (dw1000_ccp_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_CCP);
    assert(ccp);
    uint16_t node_address = ccp->frames[0]->short_address;

    /* Select single-sided or double sided twr every second slot */
    int mode = DWT_DS_TWR_EXT;
//    if (slot->idx%2==0) {
//        mode = DWT_DS_TWR;
 //   }
    dw1000_rng_request_delay_start(rng, node_address, dx_time, mode);
}


/*! 
 * @fn complete_cb(dw1000_dev_instance_t * inst, dw1000_mac_interface_t * cbs)
 *
 * @brief This callback is part of the  dw1000_mac_interface_t extension interface and invoked of the completion of a range request 
 * in the context of this example. The dw1000_mac_interface_t is in the interrupt context and is used to schedule events an event queue. 
 * Processing should be kept to a minimum giving the interrupt context. All algorithms activities should be deferred to a thread on an event queue. 
 * The callback should return true if and only if it can determine if it is the sole recipient of this event. 
 * 
 * NOTE: The MAC extension interface is a link-list of callbacks, subsequent callbacks on the list will be not be called in the 
 * event of returning true. 
 *
 * @param inst  - dw1000_dev_instance_t *
 * @param cbs   - dw1000_mac_interface_t *
 *
 * output parameters
 *
 * returns bool
 */
/* The timer callout */
static struct os_callout slot_callout;
static uint16_t g_idx_latest;

static bool
complete_cb(dw1000_dev_instance_t * inst, dw1000_mac_interface_t * cbs){
    if(inst->fctrl != FCNTL_IEEE_RANGE_16){
        return false;
    }
    dw1000_rng_instance_t* rng = (dw1000_rng_instance_t*)cbs->inst_ptr;
    g_idx_latest = (rng->idx)%rng->nframes; // Store valid frame pointer

    os_callout_init(&slot_callout, os_eventq_dflt_get(), slot_complete_cb, rng);
    os_eventq_put(os_eventq_dflt_get(), &slot_callout.c_ev);
    return true;
}

/*! 
 * @fn slot_complete_cb(struct os_event * ev)
 *
 * @brief In the example this function represents the event context processing of the received range request. 
 * In this case, a JSON string is constructed and written to stdio. See the ./apps/matlab or ./apps/python folders for examples on 
 * how to parse and render these results. 
 * 
 * input parameters
 * @param inst - struct os_event *  
 * output parameters
 * returns none 
 */
static void 
slot_complete_cb(struct os_event * ev){
    assert(ev != NULL);
    assert(ev->ev_arg != NULL);
  
    hal_gpio_toggle(LED_BLINK_PIN);
}

/*! 
 * @fn error_cb(struct os_event *ev)
 *
 * @brief This callback is in the interrupt context and is called on error event.
 * In this example just log event. 
 * Note: interrupt context so overlapping IO is possible
 * input parameters
 * @param inst - dw1000_dev_instance_t * inst
 *
 * output parameters
 *
 * returns none 
 */
static bool
error_cb(dw1000_dev_instance_t * inst, dw1000_mac_interface_t * cbs)
{ 
    if(inst->fctrl != FCNTL_IEEE_RANGE_16){
        return false;
    }   

    uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    if (inst->status.start_rx_error)
        printf("{\"utime\": %lu,\"msg\": \"start_rx_error,%s:%d\"}\n",utime, __FILE__, __LINE__);
    if (inst->status.start_tx_error)
        printf("{\"utime\": %lu,\"msg\": \"start_tx_error,%s:%d\"}\n",utime, __FILE__, __LINE__);
    if (inst->status.rx_error)
        printf("{\"utime\": %lu,\"msg\": \"rx_error,%s:%d\"}\n",utime, __FILE__, __LINE__);

    return true;
}


int main(int argc, char **argv){
    int rc;

    sysinit();

    hal_gpio_init_out(LED_BLINK_PIN, 1);
    hal_gpio_init_out(LED_1, 1);
    hal_gpio_init_out(LED_3, 1);
    
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    dw1000_rng_instance_t* rng = (dw1000_rng_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_RNG);
    assert(rng);

    dw1000_mac_interface_t cbs = {
        .id = DW1000_APP0,
        .inst_ptr = rng,
        .tx_error_cb = error_cb,
        .rx_error_cb = error_cb,
        .complete_cb = complete_cb
    };
    dw1000_mac_append_interface(inst, &cbs);
    
    dw1000_ccp_instance_t *ccp = (dw1000_ccp_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_CCP);
    assert(ccp);
    dw1000_ccp_start(ccp, CCP_ROLE_SLAVE);

    uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
    printf("{\"utime\": %lu,\"exec\": \"%s\"}\n",utime,__FILE__); 
    printf("{\"utime\": %lu,\"msg\": \"device_id = 0x%lX\"}\n",utime,inst->device_id);
    printf("{\"utime\": %lu,\"msg\": \"PANID = 0x%X\"}\n",utime,inst->PANID);
    printf("{\"utime\": %lu,\"msg\": \"DeviceID = 0x%X\"}\n",utime,inst->my_short_address);
    printf("{\"utime\": %lu,\"msg\": \"partID = 0x%lX\"}\n",utime,inst->partID);
    printf("{\"utime\": %lu,\"msg\": \"lotID = 0x%lX\"}\n",utime,inst->lotID);
    printf("{\"utime\": %lu,\"msg\": \"xtal_trim = 0x%X\"}\n",utime,inst->xtal_trim);  
    printf("{\"utime\": %lu,\"msg\": \"frame_duration = %d usec\"}\n",utime,dw1000_phy_frame_duration(&inst->attrib, sizeof(twr_frame_final_t))); 
    printf("{\"utime\": %lu,\"msg\": \"SHR_duration = %d usec\"}\n",utime,dw1000_phy_SHR_duration(&inst->attrib)); 
    printf("{\"utime\": %lu,\"msg\": \"holdoff = %d usec\"}\n",utime,(uint16_t)ceilf(dw1000_dwt_usecs_to_usecs(rng->config.tx_holdoff_delay))); 

    tdma_instance_t * tdma = (tdma_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_TDMA);
    assert(tdma);
    /* Slot 0:ccp, 1+ twr */
    for (uint16_t i = 1; i < MYNEWT_VAL(TDMA_NSLOTS); i++)
        tdma_assign_slot(tdma, slot_cb,  i, (void*)rng);

#if MYNEWT_VAL(RNG_VERBOSE) > 1
    inst->config.rxdiag_enable = 1;
#endif

    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}

