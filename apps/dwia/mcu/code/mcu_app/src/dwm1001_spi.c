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
#include <dwm1001_spi.h>
#ifdef ARCH_sim
#include <mcu/mcu_sim.h>
#endif

/**
 * @brief Configures the SPI parameters.
 *
 * @param spi_cfg_data   SPI config parameters to use
 * @param arg            Argument to be passed to callback function
 *
 * @return none
 */

void dwm1001_spi_cfg(struct spi_cfg_data * params, void *arg)
{
    struct hal_spi_settings my_spi;
    my_spi.data_order = HAL_SPI_MSB_FIRST;
    my_spi.data_mode = HAL_SPI_MODE3;
    my_spi.baudrate = MYNEWT_VAL(SPI_BAUDRATE);
    my_spi.word_size = HAL_SPI_WORD_SIZE_8BIT;
    assert(hal_spi_config(params->spi_num, &my_spi) == 0);
    assert(hal_spi_set_txrx_cb(params->spi_num, params->txrx_cb, arg) == 0);
}

/*!
 * @brief In this example dwm1001_spi_transfer is used for non-blocking interface to send a buffer and store received values. Can be used for
 * both master and slave SPI types. The user must configure the callback (using hal_spi_set_txrx_cb); the txrx callback is executed at
 * interrupt context when the buffer is sent.
 *
 * input parameters
 *
 * @param spi_cfg_data   SPI config parameters to use
 *
 * @return int 0 on success, non-zero error code on failure.
 */

int  dwm1001_spi_transfer(struct spi_cfg_data * params)
{
    int rc;
    rc = hal_spi_txrx_noblock(params->spi_num, params->txbuf, params->rxbuf, params->txlen);
    return rc;
}

/**
 * spi_ss_write
 *
 * @brief Write a value (either high or low) to the specified pin.
 *
 * @param spi_ss_num slave select pin to set
 * @param val Value to set pin (0:low 1:high)
 */

void dwm1001_spi_ss_write(int spi_ss_num, int val)
{
    hal_gpio_write(spi_ss_num, val);
}

/**
 * @brief Enables the SPI. This does not start a transmit or receive operation;
 * it is used for power mgmt. Cannot be called when a SPI transfer is in
 * progress.
 *
 * @param spi_num    SPI master number
 *
 * @return int 0 on success, non-zero error code on failure.
 */

void dwm1001_spi_enable(int spi_num)
{
    hal_spi_enable(spi_num);
}

/**
 * @brief Disables the SPI. Used for power mgmt. It will halt any current SPI transfers
 * in progress.
 *
 * @param spi_num    SPI master number
 *
 * @return int 0 on success, non-zero error code on failure.
 */

void dwm1001_spi_disable(int spi_num)
{
    hal_spi_disable(spi_num);
}
