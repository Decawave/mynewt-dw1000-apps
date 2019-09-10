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
#include <float.h>
#include "sysinit/sysinit.h"
#include "os/os.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_bsp.h"
#include "imgmgr/imgmgr.h"
#include <dw1000/dw1000_dev.h>
#include <dw1000/dw1000_hal.h>
#include <dw1000/dw1000_phy.h>
#include <dw1000/dw1000_mac.h>
#include <dw1000/dw1000_ftypes.h>
#include "uwbcfg/uwbcfg.h"
#include <config/config.h>
#include <tdma/tdma.h>
#include <app_encode.h>
#if MYNEWT_VAL(CCP_ENABLED)
#include <ccp/ccp.h>
#endif
#if MYNEWT_VAL(NRNG_ENABLED)
#include <nrng/nrng.h>
#endif
#include "dwm_hci.h"
#include "dwm_hci_app.h"

#if MYNEWT_VAL(TIMESCALE)
#include <timescale/timescale.h>
#endif
#if MYNEWT_VAL(WCS_ENABLED)
#include <wcs/wcs.h>
#endif
#if MYNEWT_VAL(SURVEY_ENABLED)
#include <survey/survey.h>
#endif
#if MYNEWT_VAL(NMGR_UWB_ENABLED)
#include <nmgr_uwb/nmgr_uwb.h>
#endif
#if MYNEWT_VAL(BLEPRPH_ENABLED)
#include "bleprph/bleprph.h"
#endif
#if MYNEWT_VAL(PAN_ENABLED)
#include <pan/pan.h>
#include <panmaster/panmaster.h>
#endif
#define DW_DATA_BUF_LEN 255

extern char dw_twr_rng_data_buf[DW_DATA_BUF_LEN];
extern char dw_twr_nrng_data_buf[DW_DATA_BUF_LEN];
static char dw_tdma_data_buf[DW_DATA_BUF_LEN] = "  TDMA-GET-DATA\n";
static char dw_twr_ss_data_buf[DW_DATA_BUF_LEN] = " twr_ss Reading the data from the App layer\0";
static char dw_twr_ds_data_buf[DW_DATA_BUF_LEN] = " twr_ds Reading the data from the App layer\0";
static char dw_nrng_ss_data_buf[DW_DATA_BUF_LEN] = " nrng_ss Reading the data from the App layer\0";
dw1000_rng_modes_t twr_rng_mode;

static bool dw1000_config_updated = false;
int uwb_config_updated(void)
{
    /* Workaround in case we're stuck waiting for ccp with the
     * wrong radio settings */
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    dw1000_ccp_instance_t *ccp = (dw1000_ccp_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_CCP);
    if (os_sem_get_count(&ccp->sem) == 0) {
        dw1000_phy_forcetrxoff(inst);
        dw1000_mac_config(inst, NULL);
        dw1000_phy_config_txrf(inst, &inst->config.txrf);
        dw1000_start_rx(inst);
        return 0;
    }

    dw1000_config_updated = true;
    return 0;
}
struct uwbcfg_cbs uwb_cb = {
    .uc_update = uwb_config_updated
};

static void nrng_complete_cb(struct os_event *ev) {
    assert(ev != NULL);
    assert(ev->ev_arg != NULL);

    hal_gpio_toggle(LED_BLINK_PIN);
    dw1000_nrng_instance_t * nrng = (dw1000_nrng_instance_t *)ev->ev_arg;
    dw1000_dev_instance_t * inst = nrng->dev_inst;
    dw1000_rng_instance_t* rng = (dw1000_rng_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_RNG);
    assert(rng);
    nrng_frame_t * nrng_frame = nrng->frames[(nrng->idx)%nrng->nframes];
    rng->idx_current = (rng->idx)%rng->nframes;
    twr_frame_t * rng_frame = rng->frames[rng->idx_current];

#ifdef VERBOSE
    if (inst->status.start_rx_error)
        printf("{\"utime\": %lu,\"timer_ev_cb\": \"start_rx_error\"}\n",os_cputime_ticks_to_usecs(os_cputime_get32()));
    if (inst->status.start_tx_error)
        printf("{\"utime\": %lu,\"timer_ev_cb\":\"start_tx_error\"}\n",os_cputime_ticks_to_usecs(os_cputime_get32()));
    if (inst->status.rx_error)
        printf("{\"utime\": %lu,\"timer_ev_cb\":\"rx_error\"}\n",os_cputime_ticks_to_usecs(os_cputime_get32()));
    if (inst->status.rx_timeout_error)
        printf("{\"utime\": %lu,\"timer_ev_cb\":\"rx_timeout_error\"}\n",os_cputime_ticks_to_usecs(os_cputime_get32()));
#endif
    if (nrng_frame->code == DWT_DS_TWR_NRNG_FINAL || nrng_frame->code == DWT_DS_TWR_NRNG_EXT_FINAL){
        nrng_frame->code = DWT_DS_TWR_NRNG_END;
    }

    if (nrng_frame->code == DWT_SS_TWR_NRNG_FINAL || nrng_frame->code == DWT_SS_TWR_NRNG_EXT_FINAL){
#if MYNEWT_VAL(APP_RNG_VERBOSE)
        app_nrng_encode(nrng, nrng->seq_num, nrng->idx);
        /* first two bytes of dw_nrng_ss_data_buf are reserved for HCI header */
        memcpy(dw_nrng_ss_data_buf+2, dw_twr_nrng_data_buf , DW_DATA_BUF_LEN -2);
#endif
        nrng->slot_mask = 0;
        nrng_frame->code = DWT_SS_TWR_NRNG_END;
    }
    switch(rng_frame->code)
    {
        case DWT_SS_TWR_FINAL:
        case DWT_SS_TWR_EXT_FINAL:
            {
                app_rng_encode(rng);
                /* first two bytes of dw_twr_ss_data_buf are reserved for HCI header */
                memcpy(dw_twr_ss_data_buf+2, dw_twr_rng_data_buf , DW_DATA_BUF_LEN -2);
                rng_frame->code = DWT_SS_TWR_EXT_END;
                break;
            }
        case DWT_DS_TWR_FINAL:
        case DWT_DS_TWR_EXT_FINAL:
            {
                app_rng_encode(rng);
                /* first two bytes of dw_twr_ds_data_buf are reserved for HCI header */
                memcpy(dw_twr_ds_data_buf+2, dw_twr_rng_data_buf , DW_DATA_BUF_LEN -2);
                rng_frame->code = DWT_DS_TWR_EXT_END;
                break;
            }
        default: break;
    }
}
/*!
 * @fn complete_cb(dw1000_dev_instance_t * inst)
 *
 * @brief This callback is in the interrupt context and is uses to schedule an pdoa_complete event on the default event queue.
 * Processing should be kept to a minimum giving the context. All algorithms should be deferred to a thread on an event queue.
 * In this example all postprocessing is performed in the pdoa_ev_cb.
 * input parameters
 * @param inst - dw1000_dev_instance_t *
 *
 * output parameters
 *
 * returns none
 */
/* The timer callout */
static struct os_callout slot_callout;
static bool complete_cb(dw1000_dev_instance_t * inst, dw1000_mac_interface_t * cbs)
{
    if(inst->fctrl != FCNTL_IEEE_RANGE_16){
        return false;
    }
    dw1000_nrng_instance_t* nrng = (dw1000_nrng_instance_t*)cbs->inst_ptr;
    os_callout_init(&slot_callout, os_eventq_dflt_get(), nrng_complete_cb, nrng);
    os_eventq_put(os_eventq_dflt_get(), &slot_callout.c_ev);
    return true;
}

/*!
 * @fn slot_timer_cb(struct os_event * ev)
 *
 * @brief In this example this timer callback is used to start_rx.
 *
 * input parameters
 * @param inst - struct os_event *
 *
 * output parameters
 *
 * returns none
 */


static void
nrng_ss_slot_cb(struct os_event * ev){
    assert(ev);

    tdma_slot_t * slot = (tdma_slot_t *) ev->ev_arg;
    tdma_instance_t * tdma = slot->parent;
    dw1000_ccp_instance_t *ccp = tdma->ccp;
    dw1000_dev_instance_t * inst = tdma->dev_inst;
    uint16_t idx = slot->idx;
    dw1000_nrng_instance_t *nrng = (dw1000_nrng_instance_t*)slot->arg;
    /* Avoid colliding with the ccp in case we've got out of sync */
    if (os_sem_get_count(&ccp->sem) == 0) {
        return;
    }
    if (ccp->local_epoch==0 || inst->slot_id == 0xffff) return;

    /* Process any newtmgr packages queued up */
    if (idx > 6 && idx < (tdma->nslots-6) && (idx%4)==0) {
        nmgr_uwb_instance_t *nmgruwb = (nmgr_uwb_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_NMGR_UWB);
        assert(nmgruwb);
        if (uwb_nmgr_process_tx_queue(nmgruwb, tdma_tx_slot_start(tdma, idx))) {
            return;
        }
    }

    if (inst->role&DW1000_ROLE_ANCHOR) {
        /* Listen for a ranging tag */
        dw1000_set_delay_start(inst, tdma_rx_slot_start(tdma, idx));
        uint16_t timeout = dw1000_phy_frame_duration(
                &inst->attrib, sizeof(nrng_request_frame_t))
            + nrng->config.rx_timeout_delay;

        /* Padded timeout to allow us to receive any nmgr packets too */
        dw1000_set_rx_timeout(inst, timeout + 0x1000);
        dw1000_nrng_listen(nrng, DWT_BLOCKING);
    } else {
        /* Range with the anchors */
        if (idx%MYNEWT_VAL(NRNG_NTAGS) != inst->slot_id) {
            return;
        }

        /* Range with the anchors */
        uint64_t dx_time = tdma_tx_slot_start(tdma, idx) & 0xFFFFFFFFFE00UL;
        uint32_t slot_mask = 0;
        for (uint16_t i = MYNEWT_VAL(NODE_START_SLOT_ID);
                i <= MYNEWT_VAL(NODE_END_SLOT_ID); i++) {
            slot_mask |= 1UL << i;
        }

        if(dw1000_nrng_request_delay_start(
                    nrng, BROADCAST_ADDRESS, dx_time,
                    DWT_SS_TWR_NRNG, slot_mask, 0).start_tx_error) {
            uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
            printf("{\"utime\": %lu,\"msg\": \"slot_timer_cb_%d:start_tx_error\"}\n",
                    utime,idx);
        }
    }
}

    static void
pan_complete_cb(struct os_event * ev)
{
    assert(ev != NULL);
    assert(ev->ev_arg != NULL);
    dw1000_pan_instance_t *pan = (dw1000_pan_instance_t *)ev->ev_arg;

    if (pan->dev_inst->slot_id != 0xffff) {
        uint32_t utime = os_cputime_ticks_to_usecs(os_cputime_get32());
        printf("{\"utime\": %lu,\"msg\": \"slot_id = %d\"}\n", utime, pan->dev_inst->slot_id);
        printf("{\"utime\": %lu,\"msg\": \"euid16 = 0x%X\"}\n", utime, pan->dev_inst->my_short_address);
    }
}

/* This function allows the ccp to compensate for the time of flight
 * from the master anchor to the current anchor.
 * Ideally this should use a map generated and make use of the euid in case
 * the ccp packet is relayed through another node.
 */
    static uint32_t
tof_comp_cb(uint16_t short_addr)
{
    float x = MYNEWT_VAL(CCP_TOF_COMP_LOCATION_X);
    float y = MYNEWT_VAL(CCP_TOF_COMP_LOCATION_Y);
    float z = MYNEWT_VAL(CCP_TOF_COMP_LOCATION_Z);
    float dist_in_meters = sqrtf(x*x+y*y+z*z);
#ifdef VERBOSE
    printf("d=%dm, %ld dwunits\n", (int)dist_in_meters,
            (uint32_t)(dist_in_meters/dw1000_rng_tof_to_meters(1.0)));
#endif
    return dist_in_meters/dw1000_rng_tof_to_meters(1.0);
}
    static void
twr_ss_slot_cb(struct os_event * ev)
{
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
    dw1000_rng_modes_t mode = twr_rng_mode;
    dw1000_rng_request_delay_start(rng, node_address, dx_time, mode);
}

int main(int argc, char **argv){
    int rc;

    sysinit();
    uwbcfg_register(&uwb_cb);
    conf_load();

    hal_gpio_init_out(LED_BLINK_PIN, 1);
    hal_gpio_init_out(LED_1, 1);
    hal_gpio_init_out(LED_3, 1);

    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    inst->config.rxauto_enable = false;
    inst->config.dblbuffon_enabled = false;
    dw1000_set_dblrxbuff(inst, inst->config.dblbuffon_enabled);

    dw1000_nrng_instance_t* nrng = (dw1000_nrng_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_NRNG);
    assert(nrng);
    dw1000_rng_instance_t* rng = (dw1000_rng_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_RNG);
    assert(rng);

    dw1000_mac_interface_t cbs = (dw1000_mac_interface_t){
        .id =  DW1000_APP0,
            .inst_ptr = nrng,
            .complete_cb = complete_cb
    };
    dw1000_mac_interface_t *cbs_p = dw1000_mac_get_interface(inst, DW1000_NRNG);
    dw1000_mac_interface_t nrng_cbs;
    memcpy(&nrng_cbs,cbs_p,sizeof(dw1000_mac_interface_t));
    dw1000_mac_remove_interface(inst, DW1000_NRNG);
    nrng_cbs.complete_cb = complete_cb;

    dw1000_mac_append_interface(inst, &cbs);
    dw1000_mac_append_interface(inst, &nrng_cbs);

    inst->slot_id = 0xffff;
    inst->my_long_address = ((uint64_t) inst->lotID << 32) + inst->partID;
#if MYNEWT_VAL(BLEPRPH_ENABLED)
    ble_init(inst->my_long_address);
#endif
    dw1000_ccp_instance_t *ccp = (dw1000_ccp_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_CCP);
    assert(ccp);
    dw1000_pan_instance_t *pan = (dw1000_pan_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_PAN);
    assert(pan);

    if (inst->role&DW1000_ROLE_CCP_MASTER) {
        /* Start as clock-master */
        dw1000_ccp_start(ccp, CCP_ROLE_MASTER);
    } else {
        dw1000_ccp_start(ccp, CCP_ROLE_SLAVE);
        dw1000_ccp_set_tof_comp_cb(ccp, tof_comp_cb);
    }

    if (inst->role&DW1000_ROLE_PAN_MASTER) {
        /* As pan-master, first lookup our address and slot_id */
        struct image_version fw_ver;
        struct panmaster_node *node;
        panmaster_find_node(inst->my_long_address, NETWORK_ROLE_ANCHOR, &node);
        assert(node);
        /* Update my fw-version in the panmaster db */
        imgr_my_version(&fw_ver);
        panmaster_add_version(inst->my_long_address, &fw_ver);
        /* Set short address and slot id */
        inst->my_short_address = node->addr;
        inst->slot_id = node->slot_id;
        dw1000_pan_start(pan, PAN_ROLE_MASTER, NETWORK_ROLE_ANCHOR);
    } else {
        dw1000_pan_set_postprocess(pan, pan_complete_cb);
        network_role_t role = (inst->role&DW1000_ROLE_ANCHOR)?
            NETWORK_ROLE_ANCHOR : NETWORK_ROLE_TAG;
        dw1000_pan_start(pan, PAN_ROLE_RELAY, role);
    }

#if MYNEWT_VAL(APP_RNG_VERBOSE) > 1
    inst->config.rxdiag_enable = 1;
#endif
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

    /* Pan is slots 1&2 */
    tdma_instance_t * tdma = (tdma_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_TDMA);
    assert(tdma);
    tdma_assign_slot(tdma, dw1000_pan_slot_timer_cb, 1, (void*)pan);
    tdma_assign_slot(tdma, dw1000_pan_slot_timer_cb, 2, (void*)pan);
    dwm_hci_init();
    printf("DWM-HCI-SPI-SLAVE\n");
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    assert(0);
    return rc;
}
enum status
{
    STOPPED = 0,
    STARTED = 1,
};
enum status tdma_flag = STOPPED;
enum status twr_ss_flag = STOPPED;
enum status twr_ds_flag = STOPPED;
enum status nrng_ss_flag = STOPPED;
char * dw_tdma_start(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    tdma_instance_t * tdma = (tdma_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_TDMA);
    assert(tdma);
    memset(dw_tdma_data_buf , 0, DW_DATA_BUF_LEN);
    if(tdma_flag == STARTED)
    {
        memcpy(dw_tdma_data_buf+2,"tdma already STARTED\n\0",23);
        return dw_tdma_data_buf;
    }

    tdma_flag = STARTED;
    memcpy(dw_tdma_data_buf+2,"tdma_STARTED\n\0",14);
    return dw_tdma_data_buf;
}

void dw_twr_start(void);
void dw_twr_start(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    tdma_instance_t * tdma = (tdma_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_TDMA);
    assert(tdma);
    dw1000_rng_instance_t* rng = (dw1000_rng_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_RNG);
    assert(rng);
    for (uint16_t i = 6; i < MYNEWT_VAL(TDMA_NSLOTS); i=i+2)
    {
        if((i+1) < MYNEWT_VAL(TDMA_NSLOTS))
            tdma_assign_slot(tdma, twr_ss_slot_cb,  i+1, (void*)(rng));
    }
}

char * dw_twr_ss_start(void)
{

    memset(dw_twr_ss_data_buf,0,DW_DATA_BUF_LEN);
    if(nrng_ss_flag == STARTED || twr_ds_flag == STARTED)
        dw_tdma_stop();
    if(twr_ss_flag == STARTED)
    {
        memcpy(dw_twr_ss_data_buf+2,"twr_ss already STARTED\n\0",25);
        return dw_twr_ss_data_buf;
    }
    //twr_rng_mode = DWT_SS_TWR;
    twr_rng_mode = DWT_SS_TWR_EXT;
    dw_twr_start();
    tdma_flag = STARTED;
    twr_ss_flag = STARTED;
    memcpy(dw_tdma_data_buf+2,"tdma_STARTED\n\0",14);
    memcpy(dw_twr_ss_data_buf+2,"twr_ss_STARTED\n\0",16);
    return dw_twr_ss_data_buf;
}

char * dw_twr_ds_start(void)
{

    memset(dw_twr_ds_data_buf, 0, DW_DATA_BUF_LEN);
    if(nrng_ss_flag == STARTED || twr_ss_flag == STARTED)
        dw_tdma_stop();
    if(twr_ds_flag == STARTED)
    {
        memcpy(dw_twr_ds_data_buf+2,"twr_ds already STARTED\n\0",25);
        return dw_twr_ds_data_buf;
    }
    //twr_rng_mode = DWT_DS_TWR;
    twr_rng_mode = DWT_DS_TWR_EXT;
    dw_twr_start();
    tdma_flag = STARTED;
    twr_ds_flag = STARTED;
    memcpy(dw_tdma_data_buf+2,"tdma_STARTED\n\0",14);
    memcpy(dw_twr_ds_data_buf+2,"twr_ds_STARTED\n\0",16);
    return dw_twr_ds_data_buf;
}


char * dw_nrng_ss_start(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    tdma_instance_t * tdma = (tdma_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_TDMA);
    assert(tdma);
    dw1000_nrng_instance_t* nrng = (dw1000_nrng_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_NRNG);
    assert(nrng);

    memset(dw_nrng_ss_data_buf,0,DW_DATA_BUF_LEN);
    if(twr_ss_flag == STARTED)
        dw_tdma_stop();
    if(nrng_ss_flag == STARTED)
    {
        memcpy(dw_nrng_ss_data_buf+2,"nrng_ss already STARTED\n\0",25);
        return dw_nrng_ss_data_buf;
    }
#if MYNEWT_VAL(SURVEY_ENABLED)
    survey_instance_t *survey = (survey_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_SURVEY);

    tdma_assign_slot(tdma, survey_slot_range_cb, MYNEWT_VAL(SURVEY_RANGE_SLOT), (void*)survey);
    tdma_assign_slot(tdma, survey_slot_broadcast_cb, MYNEWT_VAL(SURVEY_BROADCAST_SLOT), (void*)survey);
    for (uint16_t i = 6; i < MYNEWT_VAL(TDMA_NSLOTS); i=i+2)
#else
        for (uint16_t i = 3; i < MYNEWT_VAL(TDMA_NSLOTS); i=i+2)
#endif
        {
            tdma_assign_slot(tdma, nrng_ss_slot_cb, i, (void*)nrng);
        }
    tdma_flag = STARTED;
    nrng_ss_flag = STARTED;
    memcpy(dw_tdma_data_buf+2,"tdma_STARTED\n\0",14);
    memcpy(dw_nrng_ss_data_buf+2,"nrng_ss_STARTED\n\0",16);
    return dw_nrng_ss_data_buf;
}

char * dw_tdma_stop(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    tdma_instance_t * tdma = (tdma_instance_t*)dw1000_mac_find_cb_inst_ptr(inst, DW1000_TDMA);
    memset(dw_tdma_data_buf,0,DW_DATA_BUF_LEN);
    if(tdma_flag == STOPPED)
    {
        memcpy(dw_tdma_data_buf+2,"tdma already STOPPED\n\0",22);
        return dw_tdma_data_buf;
    }
    tdma_stop(tdma);
    tdma_flag = STOPPED;
    twr_ss_flag = STOPPED;
    twr_ds_flag = STOPPED;
    nrng_ss_flag = STOPPED;
    memcpy(dw_tdma_data_buf+2,"tdma_STOPPED\n\0",14);
    memcpy(dw_twr_ss_data_buf+2,"twr_ss_STOPPED\n\0",16);
    memcpy(dw_nrng_ss_data_buf+2,"twr_ds_STOPPED\n\0",16);
    memcpy(dw_nrng_ss_data_buf+2,"nrng_ss_STOPPED\n\0",16);
    return dw_tdma_data_buf;
}

char * dw_twr_ss_stop(void)
{
    memset(dw_twr_ss_data_buf, 0, DW_DATA_BUF_LEN);
    if(twr_ss_flag == STOPPED)
    {
        memcpy(dw_twr_ss_data_buf+2,"twr_ss already STOPPED\n\0",24);
        return dw_twr_ss_data_buf;
    }
    dw_tdma_stop();
    return dw_twr_ss_data_buf;
};

char * dw_twr_ds_stop(void)
{
    memset(dw_twr_ds_data_buf, 0, DW_DATA_BUF_LEN);
    if(twr_ds_flag == STOPPED)
    {
        memcpy(dw_twr_ds_data_buf+2,"twr_ds already STOPPED\n\0",24);
        return dw_twr_ds_data_buf;
    }
    dw_tdma_stop();
    return dw_twr_ds_data_buf;
};

char * dw_nrng_ss_stop(void)
{
    memset(dw_nrng_ss_data_buf, 0, DW_DATA_BUF_LEN);
    if(nrng_ss_flag == STOPPED)
    {
        memcpy(dw_nrng_ss_data_buf+2,"nrng_ss already STOPPED\n\0",24);
        return dw_nrng_ss_data_buf;
    }
    dw_tdma_stop();
    return dw_nrng_ss_data_buf;
};

char * dw_tdma_get_data(void)
{
    return dw_tdma_data_buf;
};

char * dw_twr_ss_get_data(void)
{
    return dw_twr_ss_data_buf;
};

char * dw_twr_ds_get_data(void)
{
    return dw_twr_ds_data_buf;
};

char * dw_nrng_ss_get_data(void)
{
    return dw_nrng_ss_data_buf;
};
