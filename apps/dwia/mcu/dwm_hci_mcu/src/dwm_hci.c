#include "os/mynewt.h"
#include "dwm_hci.h"
#include "dwm_hci_app.h"

#define HCI_TASK_STACK_SZ  OS_STACK_ALIGN(512)
struct os_eventq hci_eventq;     //!< Structure of os_eventq that has event queue
struct os_event hci_event;          //!< Structure of os_event that tirgger interrupts
struct os_task hci_task;     //!< Structure of os_task that has interrupt task
uint8_t hci_task_prio = 8;           //!< Priority of the interrupt task
os_stack_t hci_task_stack[HCI_TASK_STACK_SZ]  //!< Stack of the interrupt task
__attribute__((aligned(OS_STACK_ALIGNMENT)));

/* HCI task handler to process the hci events. */
static void dwm_hci_task(void *arg);

/* Interpret the command received from the host. */
static void dwm_hci_decode_cmd(struct os_event * ev);

/* Initializes spi, tasks, events,event queues */
void dwm_hci_init(void)
{
    dwm_hci_cmds_init();
    dwm_hci_gpio_init();
    dwm_hci_spi_init();
    dwm_hci_eventq_init(&hci_eventq);
    dwm_hci_events_init(&hci_event, &dwm_hci_decode_cmd,NULL);
    dwm_hci_task_init();
}

/* Create a seperate task for processing HCI events */
void dwm_hci_task_init(void)
{
    os_task_init(&hci_task, "hci_task",
            dwm_hci_task,
            NULL,//(void *) inst,
            hci_task_prio, OS_WAIT_FOREVER,
            hci_task_stack,
            HCI_TASK_STACK_SZ);
}

/* HCI task handler to process the hci events. */
static void dwm_hci_task(void *arg)
{
    while (1) {
        os_eventq_run(&hci_eventq);
    }
}

/* Create eventq for the HCI events */
void dwm_hci_eventq_init(struct os_eventq *hci_eventq)
{
    os_eventq_init(hci_eventq);
}

/* Create events for the HCI events */
void dwm_hci_events_init(struct os_event * ev, os_event_fn * fn , void *arg)
{
    memset(ev, 0, sizeof(*ev));
    ev->ev_queued = 0;
    ev->ev_cb = fn;
    ev->ev_arg = arg;
}

/*Register the call backs for HCI_SPI events */
//void dwm_hci_reg_cbs();

/* Call back to be called when a cmd was received */
void dwm_hci_cmd_rx_cb(void * cmd)
{
    struct os_event *ev;

    /* Get an event structure off the queue */
    //    ev = &g_ble_ll_hci_cmd_ev;
    //    if (ble_npl_event_is_queued(ev)) {
    //        return BLE_ERR_MEM_CAPACITY;
    //    }
    ev = &hci_event;
    /* Fill out the event and post to Link Layer */
    dwm_event_set_arg(ev, cmd);
    os_eventq_put(&hci_eventq, ev);
}

/* Fill the argument */
void dwm_event_set_arg(struct os_event *ev, void* cmd)
{
    ev->ev_arg = cmd;
}
static char null_buffer[255];
/* Interpret the command received from the host. */
static void dwm_hci_decode_cmd(struct os_event * ev)
{
    char * cmd = (char *)ev->ev_arg; // cmd is the SPI rx buffer
    printf("reveived cmd %d\n",cmd[0]);
    uint8_t opcode = dwm_get_opcode(cmd);
    if(opcode == 0)
    {
        memset(null_buffer,0,255);
        dwm_hci_data_tx(null_buffer);
        return;
    }
    char * data = dwm_hci_process_cmd(opcode);
    if(data == NULL)
    {
        data = null_buffer;
        strcpy(data,"  SERVICE NOT AAVAILABLE\n");
    }
    dwm_hci_prep_resp_data(opcode, data);
    data[1] = strlen(data+2);
    dwm_hci_data_tx(data);
    dwm_hci_data_ready_signal();
}

/* get the opcode */
uint8_t dwm_get_opcode(char * cmd)
{
    uint8_t opcode = cmd[0] & 0x7f;
    return opcode;
}

/* Send the command and  to the app Layer */
void dwm_hci_cmd(uint8_t type, char * data);

/* Read the data from app as per the commad. */
char * dwm_hci_process_cmd(uint8_t type)
{
    return dwm_cmd_process(type);
}

/* Prepare the response data to be sent to host. */
void dwm_hci_prep_resp_data(uint8_t type, char * data )
{
    data[0] = (type | DW_HCI_RESP);
}
