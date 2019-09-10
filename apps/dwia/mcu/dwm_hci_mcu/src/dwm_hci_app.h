#include<stdint.h>
typedef char * (*dw_cmd_cb)(void);

/* Copy the required data requested by the HCI */
char * dwm_cmd_process(uint8_t type);
/* Hadler to be called when an SPI event occured */
void dwm_hci_spi_handler(void *cmd, int len);
/* Fill the dwm_cmd_cbs_t with the corresponding APIs */
void dwm_hci_cmds_init(void);
/* DW1000 services */
char * dw_twr_ds_start (void);
char * dw_twr_ds_stop (void);
char * dw_twr_ds_get_data(void);
char * dw_twr_ss_start(void);
char * dw_twr_ss_stop(void);
char * dw_twr_ss_get_data(void);
char * dw_tdma_start(void);
char * dw_tdma_stop(void);
char * dw_tdma_get_data(void);
char * dw_nrng_ss_start(void);
char * dw_nrng_ss_stop(void);
char * dw_nrng_ss_get_data(void);
char * dw_pdoa_start(void);
char * dw_pdoa_stop(void);
char * dw_pdoa_get_data(void);
char * dw_ntdoa_start(void);
char * dw_ntdoa_stop(void);
char * dw_ntdoa_get_data(void);
char * dw_edm_start(void);
char * dw_edm_stop(void);
char * dw_edm_get_data(void);
char * dw_uwbcfg_init(void);
char * dw_uwbcfg_set_params(void);
char * dw_uwbcfg_get_params(void);
