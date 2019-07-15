# DWM1001 SPI-test Example

	This app verifies the SPI communication between the two dwm1001 devices whose SPI pins are externally connected to each other.
One device should be configured as SPI Master and other should acts as SPI Slave. Master Initiates the communication by periodically sending
the stream of data(32 bytes) at a time, slave device receives message fom master and ranges with node device based on the init message and sendranging data in JSON format.

## Build commands for three targets

1. Flash SPI Master on device one

```no-highlight

newt target create spi_master
newt target set spi_master app=apps/dwia/mcu/testcases/master
newt target set spi_master bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set spi_master build_profile=optimized
newt build spi_master
newt create-image spi_master 1.0.0
newt load spi_master

```

2. Flash SPI Slave on device two

```no-highlight

newt target create tag
newt target set tag app=apps/dwia/mcu/code/mcu_app
newt target set tag bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set tag build_profile=optimized
newt build tag
newt create-image tag 1.0.0
newt load tag

```

3. Flash node on device three

```no-highlight

newt target create node
newt target set node app=apps/twr_node_tdma
newt target set node bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set node build_profile=optimized
newt build node
newt create-image node 1.0.0
newt load node

```
Connect node, tag and master.
Run master and tag to see the logs.

On the master console you should see the following expected result
....
2019-07-10 12:29:37,844 - INFO # utime = 42464679
2019-07-10 12:29:37,845 - INFO # tof = 1130233856
2019-07-10 12:29:37,851 - INFO # range = 1065699385
2019-07-10 12:29:37,858 - INFO # res_tra = 50348053
2019-07-10 12:29:37,859 - INFO # rec_req = 50347810
2019-07-10 12:29:37,861 - INFO # skew = 2147483648
2019-07-10 12:29:37,882 - INFO # {"uwbValue": "init","rangeprofile": "tdma","channel": "5","destination": "0xDEC1"}
2019-07-10 12:29:37,884 - INFO # transmitted 255: 
2019-07-10 12:29:37,913 - INFO # 002998 received:255 {"utime": 43506549,"tof": 1130233856,"range": 1065699385,"res_req": 50348153, "rec_tra": 50347903, "skew": 2147483648}
2019-07-10 12:29:37,939 - INFO # 003002 
2019-07-10 12:29:37,939 - INFO # 
2019-07-10 12:29:38,881 - INFO # 
2019-07-10 12:29:38,883 - INFO # utime = 43506549
2019-07-10 12:29:38,884 - INFO # tof = 1130233856
2019-07-10 12:29:38,890 - INFO # range = 1065699385
2019-07-10 12:29:38,897 - INFO # res_tra = 50348153
2019-07-10 12:29:38,898 - INFO # rec_req = 50347903
2019-07-10 12:29:38,900 - INFO # skew = 2147483648
2019-07-10 12:29:38,921 - INFO # {"uwbValue": "init","rangeprofile": "tdma","channel": "5","destination": "0xDEC1"}
2019-07-10 12:29:38,922 - INFO # transmitted 255: 
2019-07-10 12:29:38,952 - INFO # 003131 received:255 {"utime": 44247922,"tof": 1129971712,"range": 1065542001,"res_req": 50348008, "rec_tra": 50347765, "skew": 2147483648}
....
