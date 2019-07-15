#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "sysinit/sysinit.h"
#include "tdma_lib.h"

#if MYNEWT_VAL(TDMA_ENABLED)
#include <tdma/tdma.h>
#endif
#if MYNEWT_VAL(CCP_ENABLED)
#include <ccp/ccp.h>
#endif
#if MYNEWT_VAL(WCS_ENABLED)
#include <wcs/wcs.h>
#endif
#if MYNEWT_VAL(PAN_ENABLED)
#include <pan/pan.h>
#endif



void
pan_slot_timer_cb(struct os_event * ev)
{
    tdma_slot_t * slot = (tdma_slot_t *) ev->ev_arg;
    tdma_instance_t * tdma = slot->parent;
    dw1000_dev_instance_t * inst = tdma->parent;
    dw1000_ccp_instance_t * ccp = inst->ccp;
    uint16_t idx = slot->idx;

#if MYNEWT_VAL(WCS_ENABLED)
    wcs_instance_t * wcs = ccp->wcs;
    uint64_t dx_time = (ccp->epoch + (uint64_t) roundf((1.0l + wcs->skew) * (double)((idx * (uint64_t)tdma->period << 16)/tdma->nslots)));
#else
    uint64_t dx_time = (ccp->epoch + (uint64_t) ((idx * ((uint64_t)tdma->period << 16)/tdma->nslots)));
#endif

    if (inst->pan->status.valid) return;
     /*"Random" shift to hopefully avoid collisions */
    dx_time += (os_cputime_get32()&0x7)*(tdma->period<<16)/tdma->nslots/16;
    dw1000_pan_blink(inst, 2, DWT_BLOCKING, dx_time);
}
