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

struct spi_cfg_data spi_data;
struct uwbcommand cfg_data = {"init", "tdma", "5", "0xDEC1"};
struct uwbcommand * data = &cfg_data;
struct uwbcommand config_data = {"deinit", "tdma", "5", "NULL"};
struct uwbcommand * val = &config_data;
int g_rx_len ;

/* Task 1 */
#define TASK1_PRIO (1)
#define TASK1_STACK_SIZE    OS_STACK_ALIGN(1024)
struct os_task task1;

/* Global test semaphore */
struct os_sem g_test_sem;

/* For LED toggling */
int g_led_pin;
int gpio_int_pin = 26;
int g_inited = 0;
struct os_callout gpio_callout;

#define TX_RX_BUF_LEN 255
uint8_t g_spi_tx_buf[TX_RX_BUF_LEN];
uint8_t g_spi_rx_buf[TX_RX_BUF_LEN];
void timer_ev_cb(struct os_event *ev);
extern char _buf[];

static void
spi_m_irq_handler(void *arg, int len)
{
    struct spi_cfg_data *cb;
    dwm1001_spi_ss_write(spi_data.spi_ss_num, 1);
    assert(arg == spi_data.arg);
    cb = (struct spi_cfg_data *)arg;
    assert(len == cb->txlen);

    os_sem_release(&g_test_sem);
}

void
spim_task_handler(void *arg)
{
    int i =0;
    int count = 0;
    int rc;

    /* Use SS pin for testing */
    hal_gpio_init_out(spi_data.spi_ss_num, 1);
    memset(g_spi_tx_buf, 0, TX_RX_BUF_LEN);
    spi_data.txlen = TX_RX_BUF_LEN;
    dwm1001_spi_cfg(&spi_data, spi_data.arg);
    dwm1001_spi_enable(spi_data.spi_num);

    while (1) {
        /* Toggle the LED */
        hal_gpio_toggle(g_led_pin);
        os_sem_pend(&g_test_sem, OS_TIMEOUT_NEVER);
        json_decode((char *)spi_data.rxbuf);

        printf("\nAfter decoding, range data:");
        printf("\nutime = %lu", (uint32_t)utime_val);
        printf("\ntof = %lu", (uint32_t)tof_val);
        printf("\nrange = %lu", (uint32_t)range_val);
        printf("\nres_tra = %lu", (uint32_t)res_req_val);
        printf("\nrec_req = %lu", (uint32_t)rec_tra_val);
        printf("\nskew = %lu", (uint32_t)skew_val);

        assert(hal_gpio_read(spi_data.spi_ss_num) == 1);
        dwm1001_spi_ss_write(spi_data.spi_ss_num, 0);
        g_inited = 1 ;
        count++;
        if(count == 10)
        {
            json_encode(val);
            count = 0;
        }
        else
        {
            json_encode(data);
        }
        memcpy(spi_data.txbuf, _buf, spi_data.txlen);
        rc = dwm1001_spi_transfer(&spi_data);
        assert(!rc);
        os_sem_pend(&g_test_sem, OS_TIMEOUT_NEVER);
        console_printf("\ntransmitted %d: ", spi_data.txlen);
        for (i = 0; i < spi_data.txlen; i++) {
            console_printf("%2x", spi_data.txbuf[i]);
        }
        console_printf("\n");
        console_printf("received:%d ", spi_data.txlen);
        for (i = 0; i < spi_data.txlen; i++) {
            console_printf("%2c", spi_data.rxbuf[i]);
        }
        console_printf("\n\n");
        if(count == 0) os_time_delay(OS_TICKS_PER_SEC *10);
    }
}

static void
gpio_irq_handler(void *arg){
    os_sem_release(&g_test_sem);
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

    spi_data.spi_num = MYNEWT_VAL(SPITEST_M_NUM);
    spi_data.txlen = TX_RX_BUF_LEN;
    spi_data.txrx_cb = spi_m_irq_handler;
    spi_data.txbuf = g_spi_tx_buf;
    spi_data.rxbuf = g_spi_rx_buf;
    spi_data.spi_ss_num = MYNEWT_VAL(SPITEST_SS_PIN);
    spi_data.arg = &spi_data;

    printf("spi master \n");
    pstack = malloc(sizeof(os_stack_t)*TASK1_STACK_SIZE);
    assert(pstack);

    os_task_init(&task1, "spim", spim_task_handler, NULL,
            TASK1_PRIO, OS_WAIT_FOREVER, pstack, TASK1_STACK_SIZE);
    hal_gpio_irq_init(gpio_int_pin, gpio_irq_handler, NULL, HAL_GPIO_TRIG_RISING, HAL_GPIO_PULL_DOWN);
    hal_gpio_irq_enable(gpio_int_pin);
    os_callout_init(&gpio_callout, os_eventq_dflt_get(), timer_ev_cb, NULL);
    os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC);

}

void timer_ev_cb(struct os_event *ev)
{
    if(!g_inited)
    {
        json_encode(data);
        memcpy(g_spi_tx_buf, _buf, 255);
        assert(hal_gpio_read(spi_data.spi_ss_num) == 1);
        dwm1001_spi_ss_write(spi_data.spi_ss_num, 0);
        /* Send non-blocking */
        int rc;
        rc = dwm1001_spi_transfer(&spi_data);
        assert(rc == 0 );

    }
    os_callout_reset(&gpio_callout, OS_TICKS_PER_SEC);
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
main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();
    init_tasks();
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    /* Never returns */
    assert(0);

    return rc;
}
