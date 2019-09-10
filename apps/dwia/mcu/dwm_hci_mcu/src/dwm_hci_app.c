#include "dwm_hci_app.h"
#include "dwm_hci.h"

static dw_cmd_cb dw_cmds_cbs[10][4];

/* Decode the commad according the service type and call the specific commad processor */
char * dwm_cmd_process(uint8_t cmd)
{
    //memcpy(data, ds_twr_data_buf, 254);
    uint8_t service_type = cmd & (~(3 << 4));
    uint8_t task_type    = (cmd >>4) & 3;
    if((service_type > DW_HCI_MAX_COMMANDS) || (task_type > DW_HCI_MAX_TASKS))
        service_type = task_type = 0;
    dw_cmd_cb cmd_cb = dw_cmds_cbs[service_type][task_type];
    char * pointer = NULL ;
    if(cmd_cb!= NULL)
        pointer = cmd_cb();
    return pointer;
}

void dwm_hci_cmds_init(void)
{
    dw_cmds_cbs[0][0] = dw_cmds_cbs[0][1] = dw_cmds_cbs[0][2] = dw_cmds_cbs[0][3] = NULL;
    dw_cmds_cbs[DW_HCI_TWR_DS][DW_HCI_START]    = dw_twr_ds_start ;
    dw_cmds_cbs[DW_HCI_TWR_DS][DW_HCI_STOP]     = dw_twr_ds_stop ;
    dw_cmds_cbs[DW_HCI_TWR_DS][DW_HCI_GET_DATA] = dw_twr_ds_get_data ;

    dw_cmds_cbs[DW_HCI_TWR_SS][DW_HCI_START]    = dw_twr_ss_start ;
    dw_cmds_cbs[DW_HCI_TWR_SS][DW_HCI_STOP]     = dw_twr_ss_stop ;
    dw_cmds_cbs[DW_HCI_TWR_SS][DW_HCI_GET_DATA] = dw_twr_ss_get_data ;

    dw_cmds_cbs[DW_HCI_TDMA][DW_HCI_START]    = dw_tdma_start ;
    dw_cmds_cbs[DW_HCI_TDMA][DW_HCI_STOP]     = dw_tdma_stop ;
    dw_cmds_cbs[DW_HCI_TDMA][DW_HCI_GET_DATA] = dw_tdma_get_data ;

    dw_cmds_cbs[DW_HCI_NRNG_SS][DW_HCI_START]    = dw_nrng_ss_start ;
    dw_cmds_cbs[DW_HCI_NRNG_SS][DW_HCI_STOP]     = dw_nrng_ss_stop ;
    dw_cmds_cbs[DW_HCI_NRNG_SS][DW_HCI_GET_DATA] = dw_nrng_ss_get_data ;

    dw_cmds_cbs[DW_HCI_PDOA][DW_HCI_START]    = dw_pdoa_start ;
    dw_cmds_cbs[DW_HCI_PDOA][DW_HCI_STOP]     = dw_pdoa_stop ;
    dw_cmds_cbs[DW_HCI_PDOA][DW_HCI_GET_DATA] = dw_pdoa_get_data ;

    dw_cmds_cbs[DW_HCI_NTDOA][DW_HCI_START]    = dw_ntdoa_start ;
    dw_cmds_cbs[DW_HCI_NTDOA][DW_HCI_STOP]     = dw_ntdoa_stop ;
    dw_cmds_cbs[DW_HCI_NTDOA][DW_HCI_GET_DATA] = dw_ntdoa_get_data ;

    dw_cmds_cbs[DW_HCI_EDM][DW_HCI_START]    = dw_edm_start ;
    dw_cmds_cbs[DW_HCI_EDM][DW_HCI_STOP]     = dw_edm_stop ;
    dw_cmds_cbs[DW_HCI_EDM][DW_HCI_GET_DATA] = dw_edm_get_data ;

    dw_cmds_cbs[DW_HCI_UWBCFG][DW_HCI_START]    = dw_uwbcfg_init ;
    dw_cmds_cbs[DW_HCI_UWBCFG][DW_HCI_STOP]     = dw_uwbcfg_set_params ;
    dw_cmds_cbs[DW_HCI_UWBCFG][DW_HCI_GET_DATA] = dw_uwbcfg_get_params ;
}

char * dw_pdoa_start(void)
{    return NULL;};
char * dw_pdoa_stop(void)
{    return NULL;};
char * dw_pdoa_get_data(void)
{    return NULL;};

char * dw_ntdoa_start(void)
{    return NULL;};
char * dw_ntdoa_stop(void)
{    return NULL;};
char * dw_ntdoa_get_data(void)
{    return NULL;};

char * dw_edm_start(void)
{    return NULL;};
char * dw_edm_stop(void)
{    return NULL;};
char * dw_edm_get_data(void)
{    return NULL;};

char * dw_uwbcfg_init(void)
{    return NULL;};
char * dw_uwbcfg_set_params(void)
{    return NULL;};
char * dw_uwbcfg_get_params(void)
{    return NULL;};
