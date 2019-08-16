<!--
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
-->

# Decawave TWR_TDMA Example

## Overview

The twr_node_tdma and twr_tag_tdma illustrate the capabilities of the underlying mynewt-dw1000-core library. The default behavior divides the TDMA_PERIOD into TDMA_NSLOTS and allocates slots to a single ranging task. Clock calibration and timescale compensation are enabled within the twr_tag_tdma application; this has the advantage of permitting single-sided two range exchange thus reducing the channel utilization. 

Another advantege of wireless synchronization is the ability to turn-on transeiver is a just-in-time fashion thereby reducing the power comsumption. In these examples transeiver is turned on for approximatly 190 usec per each slot. 

### Under-the-hood

The mynewt-dw1000-core driver implements the MAC layers and exports a MAC extension interface for additional services. One such service is ranging (./lib/rng). The ranging services through the extension interface expose callback to various events within the ranging process. One such callback is the complete_cb which marks the successful completion of the range request. In these examples, we attach to the complete_cb and perform subsequent processing. The available callbacks are defined in the struct _dw1000_mac_interface_t which is defined in dw1000_dev.h

This example also illustrates the clock calibration packet (CCP) and time division multiple access (TDMA) services. Both of which also bind to the MAC interface. In a synchronous network, the CCP service establishes the metronome through superframes transmissions. All epochs are derived from these superframes. The TDMA service divides the superframe period into slots and schedules event about each slot. The transceiver controls the precise timing of all frames to the microsecond. The operating system loosely schedules an event in advance of the desired epoch and this event issues a delay_start request to the transceiver. This loose/tight timing relationship reduces the timing requirements on the OS and permits the dw1000 to operate at optimum efficiency.

1. To erase the default flash image that shipped with the DWM1001 boards.

```no-highlight
$ JLinkExe -device nRF52 -speed 4000 -if SWD
J-Link>erase
J-Link>exit
$ 
```

2. On 1st dwm1001-dev board build the TDMA node (twr_node_tdma) applications for the DWM1001 module. 

```no-highlight

newt target create twr_node_tdma
newt target set twr_node_tdma app=apps/twr_node_tdma
newt target set twr_node_tdma bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set twr_node_tdma build_profile=debug
newt target amend twr_node_tdma syscfg=LOG_LEVEL=1:UWBCFG_DEF_ROLE='"0x1"'
newt run twr_node_tdma 0

```

3. On 2nd dwm1001-dev board build the TDMA tag (twr_tag_tdma) applications for the DWM1001 module. 

```no-highlight

newt target create twr_tag_tdma
newt target set twr_tag_tdma app=apps/twr_tag_tdma
newt target set twr_tag_tdma bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set twr_tag_tdma build_profile=debug
newt run twr_tag_tdma 0

```

5. On the console you should see the following expected result. 

```no-highlight

{"utime": 9484006864249, "wcs": [9483617024080,687524001872,8752525324842,687524001872], "skew": 13752215634709839872}
{"utime": 9484254696807, "twr": {"rng": "1.071","uid": "d22d"},"uid": "55a6", "diag": {"rssi": "-79.562","los": "1.000"}}
{"utime": 9484684610480, "twr": {"rng": "1.061","uid": "d22d"},"uid": "55a6", "diag": {"rssi": "-79.493","los": "1.000"}}
{"utime": 9485113714163, "twr": {"rng": "1.064","uid": "d22d"},"uid": "55a6", "diag": {"rssi": "-79.416","los": "1.000"}}
{"utime": 9485543643708, "twr": {"rng": "1.079","uid": "d22d"},"uid": "55a6", "diag": {"rssi": "-79.593","los": "1.000"}}
{"utime": 9485972719232, "twr": {"rng": "1.082","uid": "d22d"},"uid": "55a6", "diag": {"rssi": "-79.653","los": "1.000"}}


```
