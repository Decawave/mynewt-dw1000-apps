#include <assert.h>
#include <string.h>
#if MYNEWT_VAL(TDMA_ENABLED)
#include <tdma/tdma.h>
#endif

int slot_assign(void);
void stop_tdma(void);
void config_parameter(void);
void slot0_cb(struct os_event *ev);
void slot_cb(struct os_event *ev);
void pan_slot_timer_cb(struct os_event * ev);

