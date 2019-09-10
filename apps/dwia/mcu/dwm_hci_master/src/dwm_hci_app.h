#include<stdint.h>

typedef char * (*dw_cmd_cb)(void);

/* Copy the required data requested by the HCI */
char * dwm_cmd_process(uint8_t type);
/* Hadler to be called when an SPI event occured */
void dwm_hci_spi_handler(void *cmd, int len);
