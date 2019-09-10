#include <os/os_dev.h>
#include "os/mynewt.h"
#include "dwm_hci.h"
#include "dwm1001_spi.h"
#include "dwm_hci_app.h"
#include "console/console.h"
struct spi_cfg_data spi_data;
int g_rx_len ;

uint8_t g_spi_tx_buf[TX_RX_BUF_LEN];
uint8_t g_spi_rx_buf[TX_RX_BUF_LEN];

/* For LED toggling */
int g_led_pin;
int gpio_int_pin = 26;

/* Configure the SPI slave instance. */
void dwm_hci_spi_init(void)
{
  dwm1001_spi_slave_init();
}

/* Initialize the GPIO as OUT PUT ping */
void dwm_hci_gpio_init(void)
{
  g_led_pin = LED_BLINK_PIN;
  hal_gpio_init_out(g_led_pin, 1);
  hal_gpio_init_out(gpio_int_pin, 0);
}

/* Handler to be called after SPI transfer. */
void dwm_hci_spi_handler(void *cmd, int len)
{
  dwm_hci_cmd_rx_cb(cmd);
  hal_gpio_toggle(g_led_pin);
}

/* Transfer the data to the host */
void dwm_hci_data_tx(char * data)
{
   memcpy(spi_data.txbuf,data,spi_data.txlen);
   dwm1001_spi_transfer(&spi_data);
}

/* Send a GPIO HIgh signal to the host indicating that slave is ready for communication */
void dwm_hci_slave_ready_signal(void)
{
    hal_gpio_write(gpio_int_pin, 1);
    hal_gpio_write(gpio_int_pin, 0);
}

/* Send a GPIO signal to host indicating the requested data is ready. */
void dwm_hci_data_ready_signal(void)
{
  hal_gpio_write(gpio_int_pin, 1);
  hal_gpio_write(gpio_int_pin, 0);
}

void
dwm1001_spi_s_irq_handler(void *arg, int len)
{
    assert(arg == spi_data.arg);
    g_rx_len = len;
    /* Post semaphore to task waiting for SPI slave */
    dwm_hci_spi_handler(spi_data.rxbuf,len);
}

void dwm1001_spi_slave_init(void)
{
    spi_data.spi_num = MYNEWT_VAL(SPITEST_S_NUM);
    spi_data.txlen = TX_RX_BUF_LEN;
    spi_data.txrx_cb = dwm1001_spi_s_irq_handler;
    spi_data.txbuf = g_spi_tx_buf;
    spi_data.rxbuf = g_spi_rx_buf;
    spi_data.arg = &spi_data;

    dwm1001_spi_cfg(&spi_data, spi_data.arg);
    dwm1001_spi_enable(spi_data.spi_num);
    hal_spi_slave_set_def_tx_val(spi_data.spi_num, 0x77);
    spi_data.txlen = TX_RX_BUF_LEN;
    memset(spi_data.txbuf, 0, spi_data.txlen);
    assert(dwm1001_spi_transfer(&spi_data) == 0);
}
