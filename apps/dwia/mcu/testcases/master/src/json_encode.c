#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "json_util.h"

extern char _buf[JSON_BUF_SIZE];
static uint16_t idx=0;

static void
_json_fflush(){
    _buf[idx] = '\0';
    printf("\n\n");
    printf("\n%s", _buf);
    idx=0;
}

static int
json_write(void *buf, char* data, int len) {
    // write(STDOUT_FILENO, data, len);  TODOs: This is the prefered approach

    if (idx + len > JSON_BUF_SIZE)
    _json_fflush();

    for (uint16_t i=0; i< len; i++)
    _buf[i+idx] = data[i];
    idx+=len;

    return len;
}

int json_encode(struct uwbcommand * data){

    struct json_encoder encoder;
    struct json_value value;
    int rc;

    /* reset the state of the internal test */
    memset(&encoder, 0, sizeof(encoder));
    encoder.je_write = json_write;
    encoder.je_arg = NULL;

    rc = json_encode_object_start(&encoder);
    JSON_VALUE_STRING(&value, data->uwbValue);
    rc |= json_encode_object_entry(&encoder, "uwbValue", &value);
    JSON_VALUE_STRING(&value, data->rangeprofile);
    rc |= json_encode_object_entry(&encoder, "rangeprofile", &value);
    JSON_VALUE_STRING(&value, data->channel);
    rc |= json_encode_object_entry(&encoder, "channel", &value);
    JSON_VALUE_STRING(&value, data->destination);
    rc |= json_encode_object_entry(&encoder, "destination", &value);

    rc |= json_encode_object_finish(&encoder);
    assert(rc == 0);
    _json_fflush();

    return 0;
}
