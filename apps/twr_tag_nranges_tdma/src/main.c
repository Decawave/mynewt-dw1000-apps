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
#include <dw1000/dw1000_rng.h>
#include <dw1000/dw1000_ftypes.h>
#include <tdma/dw1000_tdma.h>
#include <ccp/dw1000_ccp.h>

#if MYNEWT_VAL(DW1000_LWIP)
#include <dw1000/dw1000_lwip.h>
#endif
#if MYNEWT_VAL(DW1000_PAN)
#include <dw1000/dw1000_pan.h>
#endif
#if MYNEWT_VAL(N_RANGES_NPLUS_TWO_MSGS)
#include <nranges/dw1000_nranges.h>
dw1000_nranges_instance_t *nranges_instance = NULL;
#endif

//#define DIAGMSG(s,u) printf(s,u)
#ifndef DIAGMSG
#define DIAGMSG(s,u)
#endif


static dw1000_rng_config_t rng_config = {
    .tx_holdoff_delay = 0x0300,      // Send Time delay in usec.
    .rx_timeout_period = 0x1,        // Receive response timeout in usec
    .tx_guard_delay = 0x0200
};

#if MYNEWT_VAL(DW1000_PAN)
static dw1000_pan_config_t pan_config = {
    .tx_holdoff_delay = 0x0C00,         // Send Time delay in usec.
    .rx_timeout_period = 0x8000         // Receive response timeout in usec.
};
#endif

#define N_FRAMES MYNEWT_VAL(N_NODES)*2

static nrng_frame_t twr[N_FRAMES] = {
    [0] = {
        .fctrl = FCNTL_IEEE_N_RANGES_16, // frame control (0x8841 to indicate a data frame using 16-bit addressing).
        .PANID = 0xDECA,                 // PAN ID (0xDECA)
        .code = DWT_TWR_INVALID
    }
};

static void set_default_rng_params(nrng_frame_t *frame , uint16_t nframes)
{
    uint16_t i ;
    for(i = 1 ; i<nframes ; i++)
    {
        (frame+i)->fctrl = frame->fctrl;
        (frame+i)->PANID = frame->PANID;
        (frame+i)->code  = frame->code;
    }
}

#define NSLOTS MYNEWT_VAL(TDMA_NSLOTS)

static uint16_t g_slot[NSLOTS] = {0};

/*! 
 * @fn slot_timer_cb(struct os_event * ev)
 *
 * @brief This function each 
 *
 * input parameters
 * @param inst - struct os_event *  
 *
 * output parameters
 *
 * returns none 
 */
static void 
slot_timer_cb(struct os_event *ev){
    assert(ev);

    tdma_slot_t * slot = (tdma_slot_t *) ev->ev_arg;
    tdma_instance_t * tdma = slot->parent;
    dw1000_dev_instance_t * inst = tdma->parent;
    dw1000_nranges_instance_t * nranges = nranges_instance;

    dw1000_ccp_instance_t * ccp = inst->ccp;
    uint16_t idx = slot->idx;

    hal_gpio_toggle(LED_BLINK_PIN);

#if MYNEWT_VAL(CLOCK_CALIBRATION_ENABLED)
    clkcal_instance_t * clk = ccp->clkcal;
    uint64_t dx_time = (ccp->epoch + (uint64_t) roundf(clk->skew * (double)((idx * (uint64_t)tdma->period << 16)/tdma->nslots)));
#else
    uint64_t dx_time = (ccp->epoch + (uint64_t) (idx * ((uint64_t)tdma->period << 16)/tdma->nslots));
#endif
    dx_time = dx_time & 0xFFFFFFFE00UL;
    
    uint32_t tic = os_cputime_ticks_to_usecs(os_cputime_get32());
    if(dw1000_nranges_request_delay_start(inst, 0xffff, dx_time, DWT_DS_TWR_NRNG, MYNEWT_VAL(NODE_START_SLOT_ID), MYNEWT_VAL(NODE_END_SLOT_ID)).start_tx_error){
        uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
        printf("{\"utime\": %lu,\"msg\": \"slot_timer_cb_%d:start_tx_error\"}\n",utime,idx);
    }
    uint32_t toc = os_cputime_ticks_to_usecs(os_cputime_get32());
    printf("{\"utime\": %lu,\"slot_timer_cb_tic_toc\": %lu}\n",toc,toc-tic);
    
    for(int i=0; i<nranges->nnodes; i++){
        nrng_frame_t *prev_frame = nranges->frames[i][FIRST_FRAME_IDX];
        nrng_frame_t *frame = nranges->frames[i][SECOND_FRAME_IDX];

        if ((frame->code == DWT_DS_TWR_NRNG_FINAL && prev_frame->code == DWT_DS_TWR_NRNG_T2)\
             || (prev_frame->code == DWT_DS_TWR_NRNG_EXT_T2 && frame->code == DWT_DS_TWR_NRNG_EXT_FINAL)) {
            float range = dw1000_rng_tof_to_meters(dw1000_nranges_twr_to_tof_frames(frame, prev_frame));
            printf("  src_addr= 0x%X  dst_addr= 0x%X  range= %lu\n",prev_frame->src_address,prev_frame->dst_address, (uint32_t)(range*1000));
            frame->code = DWT_DS_TWR_NRNG_END;
            prev_frame->code = DWT_DS_TWR_NRNG_END;
        }
    }
}
/*! 
 * @fn slot0_timer_cb(struct os_event * ev)
 * @brief This function is a place holder
 *
 * input parameters
 * @param inst - struct os_event *  
 * output parameters
 * returns none 
static void 
slot0_timer_cb(struct os_event *ev){
    //printf("{\"utime\": %lu,\"msg\": \"%s:[%d]:slot0_timer_cb\"}\n",os_cputime_ticks_to_usecs(os_cputime_get32()),__FILE__, __LINE__); 
}
 */
#if MYNEWT_VAL(N_RANGES_NPLUS_TWO_MSGS)
void dw1000_nranges_pkg_init(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    uint16_t nnodes = MYNEWT_VAL(N_NODES);
    set_default_rng_params(twr, sizeof(twr)/sizeof(nrng_frame_t));
    nranges_instance = dw1000_nranges_init(inst, DWT_NRNG_INITIATOR, sizeof(twr)/sizeof(nrng_frame_t), nnodes);
    dw1000_nrng_set_frames(inst, twr, sizeof(twr)/sizeof(nrng_frame_t));
}
#endif

#define SLOT MYNEWT_VAL(SLOT_ID)

int main(int argc, char **argv){
    int rc;

    sysinit();
    hal_gpio_init_out(LED_BLINK_PIN, 1);
    hal_gpio_init_out(LED_1, 1);
    hal_gpio_init_out(LED_3, 1);

    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);

    inst->PANID = 0xDECA;
    inst->my_short_address = MYNEWT_VAL(DEVICE_ID);
    inst->my_long_address = ((uint64_t) inst->device_id << 32) + inst->partID;

    dw1000_set_panid(inst,inst->PANID);
    dw1000_mac_init(inst, &inst->config);
    dw1000_rng_init(inst, &rng_config, 0);
    //dw1000_rng_set_frames(inst, twr, sizeof(twr)/sizeof(nrng_frame_t));

#if MYNEWT_VAL(DW1000_CCP_ENABLED)
    dw1000_ccp_init(inst, 2, MYNEWT_VAL(UUID_CCP_MASTER));
    dw1000_ccp_start(inst, CCP_ROLE_SLAVE);
#endif
#if MYNEWT_VAL(DW1000_PAN)
    dw1000_pan_init(inst, &pan_config);
    dw1000_pan_start(inst, DWT_NONBLOCKING);
#endif
#if MYNEWT_VAL(N_RANGES_NPLUS_TWO_MSGS)
    printf("number of nodes  ===== %u \n",nranges_instance->nnodes);
#endif
    printf("device_id = 0x%lX\n",inst->device_id);
    printf("PANID = 0x%X\n",inst->PANID);
    printf("DeviceID = 0x%X\n",inst->my_short_address);
    printf("partID = 0x%lX\n",inst->partID);
    printf("lotID = 0x%lX\n",inst->lotID);
    printf("xtal_trim = 0x%X\n",inst->xtal_trim);
    printf("no of frames == %u \n",sizeof(twr)/sizeof(nrng_frame_t));
    printf("frame_duration = %d usec\n",dw1000_phy_frame_duration(&inst->attrib, sizeof(twr_frame_final_t))); 
    printf("SHR_duration = %d usec\n",dw1000_phy_SHR_duration(&inst->attrib)); 
    printf("holdoff = %d usec\n",(uint16_t)ceilf(dw1000_dwt_usecs_to_usecs(rng_config.tx_holdoff_delay))); 

   for (uint16_t i = 0; i < sizeof(g_slot)/sizeof(uint16_t); i++)
        g_slot[i] = i;
    tdma_init(inst, MYNEWT_VAL(TDMA_PERIOD), NSLOTS);
    tdma_assign_slot(inst->tdma, slot_timer_cb, g_slot[SLOT], &g_slot[SLOT]);
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }
    assert(0);
    return rc;
}

