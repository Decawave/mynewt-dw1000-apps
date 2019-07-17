/*
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
#include "os/mynewt.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"
#include "stats/stats.h"
#include "config/config.h"
#include <console/console.h>
#include "dwm1001_spi.h"
#ifdef ARCH_sim
#include <mcu/mcu_sim.h>
#endif
#include "json_util.h"
#include "tdma_lib.h"

struct spi_cfg_data spi_data;
int g_rx_len ;
extern char rng_buf[255];
extern struct uwb dat;

/* Task 1 */
#define TASK1_PRIO (1)
#define TASK1_STACK_SIZE    OS_STACK_ALIGN(1024)
struct os_task task1;

/* Global test semaphore */
struct os_sem g_test_sem;

/* For LED toggling */
int g_led_pin;
int gpio_int_pin = 26;
int slot_init = 0;
struct os_callout gpio_callout;

#define TX_RX_BUF_LEN 255
uint8_t g_spi_tx_buf[TX_RX_BUF_LEN];
uint8_t g_spi_rx_buf[TX_RX_BUF_LEN];

void
spi_s_irq_handler(void *arg, int len)
{
    assert(arg == spi_data.arg);
    g_rx_len = len;
    /* Post semaphore to task waiting for SPI slave */
    os_sem_release(&g_test_sem);
}

void
spis_task_handler(void *arg)
{
    int rc;
//    int j =0;
    dwm1001_spi_cfg(&spi_data, spi_data.arg);
    dwm1001_spi_enable(spi_data.spi_num);

    /* Make the default character 0x77 */
    hal_spi_slave_set_def_tx_val(spi_data.spi_num, 0x77);

    /*  Fill buffer with 0x77 for first transfer. */
    spi_data.txlen = TX_RX_BUF_LEN;
    memset(spi_data.txbuf, '@', spi_data.txlen);
    rc = dwm1001_spi_transfer(&spi_data);
    while (1) {
        /* Wait for semaphore from ISR */
        os_sem_pend(&g_test_sem, OS_TIMEOUT_NEVER);
        json_decode((char *)spi_data.rxbuf);
        printf("\nAfter decoding init message:");
        printf("\n%s", dat.value);
        printf("\n%s", dat.profile);
        printf("\n%s", dat.chan_num);
        printf("\n%s", dat.dest_addr);

        if((strcmp(dat.value, "init") && strcmp(dat.profile, "tdma") && strcmp(dat.chan_num, "5") && strcmp(dat.dest_addr, "0xDEC1")) == 0)
        {
            /* Start Ranging + Generate GPIO signal to Master */
            if(!slot_init)
            {
            config_parameter();
            slot_assign();
            slot_init = 1;
            }
            os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC);
        }
        else if (strncmp(dat.value,"deinit",6) == 0 )
        {
            /* Stop Ranging + Stop GPIO interrupts */
            stop_tdma();
            os_callout_stop(&gpio_callout);
            slot_init = 0;
        }
        //spi_data.txlen = spi_data.txlen ? spi_data.txlen:32;
        memcpy(spi_data.txbuf, rng_buf, spi_data.txlen);
        //memset(spi_data.txbuf, ++j, spi_data.txlen);
        //spi_data.txbuf[0] = 'M';
        rc = dwm1001_spi_transfer(&spi_data);
        assert(rc == 0);
        console_printf("\nData rcvd %2d: ",g_rx_len);
        /*for (i = 0; i < g_rx_len; i++)
            console_printf("%2x ", spi_data.rxbuf[i]);
        console_printf("\n");  */
        memset(rng_buf, 0, spi_data.txlen);
        /* Toggle the LED */
        hal_gpio_toggle(g_led_pin);
    }
}

void timer_ev_cb(struct os_event *ev)
{
    hal_gpio_write(gpio_int_pin, 1);
    hal_gpio_write(gpio_int_pin, 0);
//    os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC);
}
/**
 * init_tasks
 *
 * Called by main.c after sysinit(). This function performs initializations
 * that are required before tasks are running.
 *
 * @return int 0 success; error otherwise.
 */
static void
init_tasks(void)
{
    os_stack_t *pstack;

    (void)pstack;

    /* Initialize global test semaphore */
    os_sem_init(&g_test_sem, 0);
    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);

    spi_data.spi_num = MYNEWT_VAL(SPITEST_S_NUM);
    spi_data.txlen = TX_RX_BUF_LEN;
    spi_data.txrx_cb = spi_s_irq_handler;
    spi_data.txbuf = g_spi_tx_buf;
    spi_data.rxbuf = g_spi_rx_buf;

    spi_data.arg = &spi_data;

    printf("spi slave\n");
    pstack = malloc(sizeof(os_stack_t)*TASK1_STACK_SIZE);
    assert(pstack);

    os_task_init(&task1, "spis", spis_task_handler, NULL,
            TASK1_PRIO, OS_WAIT_FOREVER, pstack, TASK1_STACK_SIZE);
    hal_gpio_init_out(gpio_int_pin, 1);
    os_callout_init(&gpio_callout, os_eventq_dflt_get(), timer_ev_cb, NULL);
    //os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC);
}

/**
 * main
 *
 * The main task for the project. This function initializes the packages, calls
 * init_tasks to initialize additional tasks (and possibly other objects),
 * then starts serving events from default event queue.
 *
 * @return int NOTE: this function should never return!
 */
int
spi_slave(void)
{

    init_tasks();
    return 0;
}
