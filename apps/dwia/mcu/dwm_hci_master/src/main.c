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
#include "dwm_hci.h"
#include "dwm_hci_app.h"
#include "shell/shell.h"

#ifdef ARCH_sim
#include <mcu/mcu_sim.h>
#endif

struct spi_cfg_data spi_data;
int g_rx_len ;
int command_tx = 0;
#define TX_RX_BUF_LEN 255
uint8_t g_spi_tx_buf[TX_RX_BUF_LEN];
uint8_t g_spi_rx_buf[TX_RX_BUF_LEN];

void dwm_spi_m_irq_handler(void *arg, int len)
{
    assert(arg == spi_data.arg);
    struct spi_cfg_data *cb;
    dwm1001_spi_ss_write(spi_data.spi_ss_num, 1);
    cb = (struct spi_cfg_data *)arg;
    assert(len == cb->txlen);
    g_rx_len = len;
    /* Post semaphore to task waiting for SPI slave */
    dwm_hci_spi_handler(spi_data.rxbuf,len);
}

void dwm1001_spi_master_init(void)
{
    spi_data.spi_num = MYNEWT_VAL(SPITEST_M_NUM);
    spi_data.txlen = TX_RX_BUF_LEN;
    spi_data.txrx_cb = dwm_spi_m_irq_handler;
    spi_data.txbuf = g_spi_tx_buf;
    spi_data.rxbuf = g_spi_rx_buf;
    spi_data.spi_ss_num = MYNEWT_VAL(SPITEST_SS_PIN);
    spi_data.arg = &spi_data;

    hal_gpio_init_out(spi_data.spi_ss_num, 1);
    dwm1001_spi_cfg(&spi_data, spi_data.arg);
    dwm1001_spi_enable(spi_data.spi_num);
    memset(spi_data.txbuf, 0, spi_data.txlen);
}

static int dwm_shell_hci_transfer(int argc, char** argv);
static int dwm_shell_hci_transfer(int argc, char** argv)
{
    if(argc != 3)
    {
        if(argc < 3) printf("Less No. of arguments\n" );
        else printf("More No. of arguments\n" );

        printf("\nusage case::: hci_send  SERVICE_TYPE_VALUE   TASK_TYPE_VALUE \n");
        printf("\n\n         Available SERVICE_TYPES and TASK_TYPES            \n");
        printf("---------------------------------------------------------------\n");
        printf("|    SERVICE_TYPE   : value  |   TASK_TYPE           : value  |\n");
        printf("---------------------------------------------------------------\n");
        printf("     DW_HCI_TDMA    :   1    |   DW_HCI_START        :   0     \n");
        printf("     DW_HCI_TWR_SS  :   2    |   DW_HCI_STOP         :   1     \n");
        printf("     DW_HCI_TWR_DS  :   3    |   DW_HCI_GET_DATA     :   2     \n");
        printf("     DW_HCI_NRNG_SS :   4    |                                 \n");
        printf("     DW_HCI_NTDOA   :   5    |                                 \n");
        printf("     DW_HCI_PDOA    :   6    |                                 \n");
        printf("     DW_HCI_EDM     :   7    |                                 \n");
        printf("---------------------------------------------------------------\n");
        return 0;
    }
    int service_type = atoi(argv[1]);
    int task_type    = atoi(argv[2]);

    //printf("service_type = %d\n",service_type);
    //printf("task_type    = %d\n",task_type);

    if((service_type > DW_HCI_NRNG_SS) || (task_type > DW_HCI_MAX_TASKS))
    {
        printf("service_type %d is currently not supported\n",service_type);
        return 0;
    }
    assert(hal_gpio_read(spi_data.spi_ss_num) == 1);
    dwm1001_spi_ss_write(spi_data.spi_ss_num, 0);
    command_tx = spi_data.txbuf[0] = service_type | ( task_type << DW_HCI_TASK_MASK);
    //printf("Transmitting %d bytes\n", spi_data.txlen);
    printf("command : %2d\n", spi_data.txbuf[0]);
    /* Needed to do this in a HCI model Instead of direct HCI SPI transfer */
    int rc = dwm1001_spi_transfer(&spi_data);
    assert(!rc);
    //for (int i = 0; i < spi_data.txlen; i++) {
    //    printf("%2x", spi_data.txbuf[i]);
    //}
    return 1;
}

struct shell_cmd_help help[] = {
    {
        .summary = "Command to send the HCI commands to the HCI-SLAVE",
        .usage = "hci_send commad",
    },
};

struct shell_cmd hci_cli_cmds[] = {
    {
        .sc_cmd = "hci_send",
        .sc_cmd_func = dwm_shell_hci_transfer,
        .help = &help[0],
    },
};
#define CLI_CMD_NUM sizeof(hci_cli_cmds)/sizeof(struct shell_cmd)

/**
 * main
 *
 * The main task for the project. This function initializes the packages, calls
 * init_tasks to initialize additional tasks (and possibly other objects),
 * then starts serving events from default event queue.
 *
 * @return int NOTE: this function should never return!
 */
int main(int argc, char **argv)
{
    int rc;

#ifdef ARCH_sim
    mcu_sim_parse_args(argc, argv);
#endif

    sysinit();
    printf("DWM-HCI-SPI-MASTER-SHELL\n");
    dwm_hci_init();
    for(int i =0; i < CLI_CMD_NUM; i++)
        shell_cmd_register(&hci_cli_cmds[i]);
    while (1) {
        os_eventq_run(os_eventq_dflt_get());
    }

    /* Never returns */
    assert(0);

    return rc;
}
