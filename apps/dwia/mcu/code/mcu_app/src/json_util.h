#ifndef _JSON_ENCODE_H_
#define _JSON_ENCODE_H_

#include <assert.h>
#include <string.h>
#include "json/json.h"

struct uwb
{
    char value[7];
    char profile[7];
    char chan_num[10];
    char dest_addr[6];
};

/* a test structure to hold the json flat buffer and pass bytes
 * to the decoder */
struct test_jbuf {
    /* json_buffer must be first element in the structure */
    struct json_buffer json_buf;
    char * start_buf;
    char * end_buf;
    int current_position;
};

char test_jbuf_read_next(struct json_buffer *jb);
char test_jbuf_read_prev(struct json_buffer *jb);
int test_jbuf_readn(struct json_buffer *jb, char *buf, int size);
void test_buf_init(struct test_jbuf *ptjb, char *string);
void json_decode(char * buf);

#endif

