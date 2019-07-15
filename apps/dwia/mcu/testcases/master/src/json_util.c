#include <assert.h>
#include <string.h>
#include "json_util.h"
#include "json/json.h"

char
test_jbuf_read_next(struct json_buffer *jb) {
    char c;
    struct test_jbuf  *ptjb = (struct test_jbuf*) jb;

    if((ptjb->start_buf + ptjb->current_position) <= ptjb->end_buf) {
        c = *(ptjb->start_buf + ptjb->current_position);
        ptjb->current_position++;
        return c;
    }
    return '\0';
}

/* this goes backward in the buffer one character */
char
test_jbuf_read_prev(struct json_buffer *jb) {
    char c;
    struct test_jbuf  *ptjb = (struct test_jbuf*) jb;
    if(ptjb->current_position) {
        ptjb->current_position--;
        c = *(ptjb->start_buf + ptjb->current_position);
        return c;
    }

    /* can't rewind */
    return '\0';

}

int
test_jbuf_readn(struct json_buffer *jb, char *buf, int size) {
    struct test_jbuf  *ptjb = (struct test_jbuf*) jb;

    int remlen;

    remlen = ptjb->end_buf - (ptjb->start_buf + ptjb->current_position);
    if (size > remlen) {
        size = remlen;
    }

    memcpy(buf, ptjb->start_buf + ptjb->current_position, size);
    ptjb->current_position += size;
    return size;
}

void
test_buf_init(struct test_jbuf *ptjb, char *string) {
    /* initialize the decode */
    ptjb->json_buf.jb_read_next = test_jbuf_read_next;
    ptjb->json_buf.jb_read_prev = test_jbuf_read_prev;
    ptjb->json_buf.jb_readn = test_jbuf_readn;
    ptjb->start_buf = string;
    ptjb->end_buf = string + strlen(string);
    /* end buf points to the NULL */
    ptjb->current_position = 0;
}



