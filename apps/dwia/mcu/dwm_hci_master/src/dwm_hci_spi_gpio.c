#include <os/os_dev.h>
#include "os/mynewt.h"
#include "dwm_hci.h"
#include "dwm1001_spi.h"
#include "dwm_hci_app.h"
#include <console/console.h>
extern struct spi_cfg_data spi_data;
/* For LED toggling */
int g_led_pin;
int gpio_int_pin = 26;

/* Configure the SPI slave instance. */
void dwm_hci_spi_init(void)
{
    dwm1001_spi_master_init();
}

static void
gpio_irq_handler(void *arg){
    //  void dwm_hci_data_tx(char * data)
    // Needed to put this spi_transfer in a Queue
    {
        memset(spi_data.txbuf,0,spi_data.txlen);
        assert(hal_gpio_read(spi_data.spi_ss_num) == 1);
        dwm1001_spi_ss_write(spi_data.spi_ss_num, 0);
        int rc = dwm1001_spi_transfer(&spi_data);
        assert(!rc);
    }
}

/* Initialize the GPIO as OUT PUT ping */
void dwm_hci_gpio_init(void)
{
    g_led_pin = LED_BLINK_PIN;
    hal_gpio_init_out(g_led_pin, 1);
    hal_gpio_irq_init(gpio_int_pin, gpio_irq_handler, NULL, HAL_GPIO_TRIG_RISING, HAL_GPIO_PULL_DOWN);
    hal_gpio_irq_enable(gpio_int_pin);
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
