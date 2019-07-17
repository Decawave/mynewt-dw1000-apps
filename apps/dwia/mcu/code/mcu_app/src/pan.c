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
    uint16_t idx = slot->idx;

    if (inst->pan->status.valid &&
        dw1000_pan_lease_remaining(inst)>MYNEWT_VAL(PAN_LEASE_EXP_MARGIN)) {

        /* Listen for possible pan resets from master */
        uint16_t timeout = dw1000_phy_frame_duration(&inst->attrib, sizeof(sizeof(struct _pan_frame_t)))
            + MYNEWT_VAL(XTALT_GUARD);
        dw1000_set_rx_timeout(inst, timeout);
        dw1000_set_delay_start(inst, tdma_rx_slot_start(inst, idx));
        dw1000_set_on_error_continue(inst, true);
        dw1000_pan_listen(inst, DWT_BLOCKING);
    } else {
        /* Subslot 0 is for master reset, subslot 1 is for sending requests */
        uint64_t dx_time = tdma_tx_slot_start(inst, idx + 1.0f/16);
        dw1000_pan_blink(inst, 2, DWT_BLOCKING, dx_time);
    }
}
