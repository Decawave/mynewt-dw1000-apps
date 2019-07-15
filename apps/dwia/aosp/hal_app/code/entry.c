/*
// Copyright (c) 2015 Intel Corporation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
*/

#include <hardware/sensors.h>
#include <utils/Log.h>
//#include "enumeration.h"
//#include "control.h"
//#include "description.h"
//#include "utils.h"

#include <errno.h>

/* This is the UWB Virtual Sensors HAL module entry points file */

static int init_count;


static int close_module (__attribute__((unused)) hw_device_t *device)
{
	if (init_count == 0)
		return -EINVAL;

	init_count--;

	if (init_count == 0) {
		ALOGD("Closing UWB virtual sensors HAL module\n");
		//delete_enumeration_data();
		//delete_control_data();
	}

	return 0;
}


static int initialize_module(const struct hw_module_t *module, const char *id,
				struct hw_device_t** device)
{
	static struct sensors_poll_device_1 poll_device;

	if (strcmp(id, SENSORS_HARDWARE_POLL))
                return -EINVAL;

	poll_device.common.tag		= HARDWARE_DEVICE_TAG;
	poll_device.common.version	= SENSORS_DEVICE_API_VERSION_0_1;
	poll_device.common.module	= (struct hw_module_t*) module;
	poll_device.common.close	= close_module;

/*
	poll_device.activate		= activate;
	poll_device.setDelay		= set_delay;
	poll_device.poll		= poll;
	poll_device.batch		= batch;
	poll_device.flush		= flush;
*/
	*device = &poll_device.common;

        if (init_count == 0) {
		ALOGD("Initializing UWB virtual sensors HAL module\n");
		//allocate_control_data();
		//enumerate_sensors();
	}

	init_count++;
	return 0;
}


static struct hw_module_methods_t module_methods = {
	.open = initialize_module
};


/* Externally visible module descriptor */
struct sensors_module_t __attribute__ ((visibility ("default")))
	HAL_MODULE_INFO_SYM = {
		.common = {
			.tag = HARDWARE_MODULE_TAG,
			.version_major = 0,
			.version_minor = 1,
			.id = SENSORS_HARDWARE_MODULE_ID,
			.name = "UWB virtual sensors HAL",
			.author = "PathPartner",
			.methods = &module_methods,
		},
		.get_sensors_list = 0
};

