
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

#ifndef __DW1001_SPI_H__
#define __DW1001_SPI_H__

#include <assert.h>
#include <string.h>
#include "os/mynewt.h"
#include "bsp/bsp.h"
#include "hal/hal_gpio.h"
#include "hal/hal_spi.h"

#ifdef __cplusplus
extern "C" {
#endif

struct spi_cfg_data
{
    int spi_num;
    int spi_ss_num;
    int txlen;
    hal_spi_txrx_cb txrx_cb;
    uint8_t *txbuf;
    uint8_t *rxbuf;
    void *arg;
};

int  dwm1001_spi_transfer(struct spi_cfg_data *);
void dwm1001_spi_cfg(struct spi_cfg_data *, void *arg);
void dwm1001_spi_ss_write(int spi_ss_num, int val);
void dwm1001_spi_disable(int spi_num);
void dwm1001_spi_enable(int spi_num);
int spi_slave(void);
#ifdef __cplusplus
}
#endif

#endif /* __DW1001_SPI_H__ */

