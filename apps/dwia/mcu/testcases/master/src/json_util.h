#ifndef _JSON_ENCODE_H_
#define _JSON_ENCODE_H_

#include <assert.h>
#include <string.h>
#include "json/json.h"

#define JSON_BUF_SIZE (1024*8)
char _buf[JSON_BUF_SIZE];

struct uwbcommand
{
    char uwbValue[7];
    char rangeprofile[7];
    char channel[10];
    char destination[6];
};

long long unsigned int utime_val;
long long unsigned int tof_val;
long long unsigned int range_val;
long long unsigned int res_req_val;
long long unsigned int rec_tra_val;
long long unsigned int skew_val;

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

int json_encode(struct uwbcommand * data);
void json_decode(char * buf);

#endif

