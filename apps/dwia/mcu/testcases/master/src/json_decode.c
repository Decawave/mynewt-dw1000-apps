#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "json_util.h"
#include "json/json.h"


struct test_jbuf tjb;
int rc;

struct json_attr_t test_attr[7] = {
    [0] = {
          .attribute = "utime",
          .type = t_uinteger,
          .addr.uinteger = &utime_val,
          .nodefault = true,
          },

    [1] = {
          .attribute = "tof",
          .type = t_uinteger,
          .addr.uinteger = &tof_val,
          .nodefault = true,
    },

    [2] = {
        .attribute = "range",
        .type = t_uinteger,
        .addr.uinteger = &range_val,
        .nodefault = true,
    },

    [3] =  {
        .attribute = "res_req",
        .type = t_uinteger,
        .addr.uinteger = &res_req_val,
        .nodefault = true,
    },

    [4] =  {
        .attribute = "rec_tra",
        .type = t_uinteger,
        .addr.uinteger = &rec_tra_val,
        .nodefault = true,
    },

    [5] =  {
        .attribute = "skew",
        .type = t_uinteger,
        .addr.uinteger = &skew_val,
        .nodefault = true,
    },

    [6] = {
        .attribute = NULL
    }


};

void json_decode(char *_buf)
{

test_buf_init(&tjb, _buf);

rc = json_read_object(&tjb.json_buf, test_attr);

}





