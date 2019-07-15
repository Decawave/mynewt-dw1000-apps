#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "json_util.h"
#include "json/json.h"


struct uwb dat;
struct test_jbuf tjb;
int rc;

struct json_attr_t test_attr[5] = {
    [0] = {
          .attribute = "uwbValue",
          .type = t_string,
          .addr.string = dat.value,
          .nodefault = true,
          .len = sizeof(dat.value)
          },

    [1] = {
          .attribute = "rangeprofile",
          .type = t_string,
          .addr.string = dat.profile,
          .nodefault = true,
          .len = sizeof(dat.profile)
    },

   [2] = {
          .attribute = "channel",
          .type = t_string,
          .addr.string = dat.chan_num,
          .nodefault = true,
          .len = sizeof(dat.chan_num)
    },

    [3] = {
        .attribute = "destination",
        .type = t_string,
        .addr.string = dat.dest_addr,
        .nodefault = true,
        .len = sizeof(dat.dest_addr)
    },

    [4] = {
          .attribute = NULL
          }
};

void json_decode(char *_buf)
{

test_buf_init(&tjb, _buf);

rc = json_read_object(&tjb.json_buf, test_attr);

}





