#include <os/os_dev.h>
#include "os/mynewt.h"
/*
1 byte command
  0 t0 6 bits represent command.
  7th bit represents whether it is a response message.
*/
#define DW_HCI_START                  0
#define DW_HCI_STOP                   1
#define DW_HCI_GET_DATA               2

#define DW_HCI_TASK_MASK              (4)
#define DW_HCI_START_MASK             (DW_HCI_START << DW_HCI_TASK_MASK)
#define DW_HCI_STOP_MASK              (DW_HCI_STOP << DW_HCI_TASK_MASK)
#define DW_HCI_GET_DATA_MASK          (DW_HCI_GET_DATA << DW_HCI_TASK_MASK)

#define DW_HCI_TDMA                   1
#define DW_HCI_TDMA_START             (DW_HCI_TDMA | DW_HCI_START_MASK )
#define DW_HCI_TDMA_STOP              (DW_HCI_TDMA | DW_HCI_STOP_MASK )
#define DW_HCI_TDMA_GET_DATA          (DW_HCI_TDMA | DW_HCI_GET_DATA_MASK )

#define DW_HCI_TWR_SS                 2
#define DW_HCI_TWR_SS_START           (DW_HCI_TWR_SS | DW_HCI_START_MASK )
#define DW_HCI_TWR_SS_STOP            (DW_HCI_TWR_SS | DW_HCI_STOP_MASK )
#define DW_HCI_TWR_SS_GET_DATA        (DW_HCI_TWR_SS | DW_HCI_GET_DATA_MASK )

#define DW_HCI_TWR_DS                 3
#define DW_HCI_TWR_DS_START           (DW_HCI_TWR_DS | DW_HCI_START_MASK )
#define DW_HCI_TWR_DS_STOP            (DW_HCI_TWR_DS | DW_HCI_STOP_MASK )
#define DW_HCI_TWR_DS_GET_DATA        (DW_HCI_TWR_DS | DW_HCI_GET_DATA_MASK )

#define DW_HCI_NRNG_SS                4
#define DW_HCI_NRNG_SS_START          (DW_HCI_NRNG_SS | DW_HCI_START_MASK )
#define DW_HCI_NRNG_SS_STOP           (DW_HCI_NRNG_SS | DW_HCI_STOP_MASK )
#define DW_HCI_NRNG_SS_GET_DATA       (DW_HCI_NRNG_SS | DW_HCI_GET_DATA_MASK )

#define DW_HCI_NTDOA                  5
#define DW_HCI_NTDOA_START            (DW_HCI_NTDOA | DW_HCI_START_MASK )
#define DW_HCI_NTDOA_STOP             (DW_HCI_NTDOA | DW_HCI_STOP_MASK )
#define DW_HCI_NTDOA_GET_DATA         (DW_HCI_NTDOA | DW_HCI_GET_DATA_MASK )

#define DW_HCI_PDOA                   6
#define DW_HCI_PDOA_START             (DW_HCI_PDOA | DW_HCI_START_MASK )
#define DW_HCI_PDOA_STOP              (DW_HCI_PDOA | DW_HCI_STOP_MASK )
#define DW_HCI_PDOA_GET_DATA          (DW_HCI_PDOA | DW_HCI_GET_DATA_MASK )

#define DW_HCI_EDM                    7
#define DW_HCI_EDM_START              (DW_HCI_EDM | DW_HCI_START_MASK )
#define DW_HCI_EDM_STOP               (DW_HCI_EDM | DW_HCI_STOP_MASK )
#define DW_HCI_EDM_GET_DATA           (DW_HCI_EDM | DW_HCI_GET_DATA_MASK )

#define DW_HCI_UWBCFG                 8
#define DW_HCI_UWBCFG_SET_PARAMS      (DW_HCI_EDM | DW_HCI_START_MASK )
#define DW_HCI_UWBCFG_GET_PARAMS      (DW_HCI_EDM | DW_HCI_GET_DATA_MASK )

#define DW_HCI_RESP                   (1<<7)
#define DW_HCI_MAX_COMMANDS           DW_HCI_UWBCFG
#define DW_HCI_MAX_TASKS              DW_HCI_GET_DATA

/* Initializes spi, tasks, events,event queues */
void dwm_hci_init(void);
/* Initialize the GPIOs */
void dwm_hci_gpio_init(void);

/* Configure the SPI slave instance. */
void dwm_hci_spi_init(void);

/* Create a seperate task for processing HCI events */
void dwm_hci_task_init(void);

/* Create eventq for the HCI events */
void dwm_hci_eventq_init(struct os_eventq *hci_eventq);

/* Create events for the HCI events */
void dwm_hci_events_init(struct os_event * ev, os_event_fn * fn , void *arg);

/*Register the call backs for HCI_SPI events */
//void dwm_hci_reg_cbs(cbs);

/* Call back to be called when a cmd was received */
void dwm_hci_cmd_rx_cb(void * cmd);

/* Fill the argument */
void dwm_event_set_arg(struct os_event *ev, void* cmd);

/* get the opcode */
uint8_t dwm_get_opcode(char * cmd);

/* Send the command and  to the app Layer */
void dwm_hci_cmd(uint8_t type, char * data);

/* Process the command received. */
char * dwm_hci_process_cmd(uint8_t type);

/* Prepare the response data to be sent to host. */
void dwm_hci_prep_resp_data(uint8_t type, char * data );

/* Send the HCI prepared Data */
void dwm_hci_data_tx(char * data);

/* Send a GPIO interrupt to notify that the data is ready */
void dwm_hci_data_ready_signal(void);

