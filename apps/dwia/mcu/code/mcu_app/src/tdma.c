#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <config/config.h>
#include <dw1000/dw1000_dev.h>
#include <dw1000/dw1000_hal.h>
#if MYNEWT_VAL(PAN_ENABLED)
#include <pan/pan.h>
#endif

#include "json_util.h"
#include "tdma_lib.h"
#include "uwbcfg/uwbcfg.h"

uint16_t g_slot[MYNEWT_VAL(TDMA_NSLOTS)] = {0};
extern struct uwb dat;

int slot_assign(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);

    for (uint16_t i = 0; i < sizeof(g_slot)/sizeof(uint16_t); i++)
        g_slot[i] = i;
    tdma_assign_slot(inst->tdma, slot0_cb, g_slot[0], &g_slot[0]);
    tdma_assign_slot(inst->tdma, pan_slot_timer_cb, g_slot[1], &g_slot[1]);

    for (uint16_t i = 2; i < sizeof(g_slot)/sizeof(uint16_t); i+=4)
        tdma_assign_slot(inst->tdma, slot_cb, g_slot[i], &g_slot[i]);


    return 0;
}

void stop_tdma(void)
{
    dw1000_dev_instance_t * inst = hal_dw1000_inst(0);
    tdma_stop(inst->tdma);
}

void config_parameter(void)
{
    static char buf[12];
    char *str;
    char name[15] = "uwb/channel";
    char name_buf[15];
    strcpy(name_buf,name);
    str = conf_get_value(name_buf,buf, 1);
    printf("curent channel = %s\n",str);

    strcpy(name_buf,name);
    conf_set_value(name_buf,dat.chan_num);

    strcpy(name_buf,name);
    str = conf_get_value(name_buf, buf , 1);
    printf("changed channel = %s\n",str);

    uwbcfg_apply();
}

