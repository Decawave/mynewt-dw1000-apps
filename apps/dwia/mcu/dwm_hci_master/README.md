These apps are tested with 1.3.0 released version of mynewt-dw1000-core.
# Introduction:
   SPI Based HCI is a communication protocol that involves master and slave , where master sends HCI type commands to the slave via SPI interface and expects the response for each command.
The Implementation was explained with below mentioned three apps.
1. dwm_hci_mcu
2. dwm_hci_master
3. super-node

### dwm_hci_mcu:
This app is a combination of dw1000 services like n_ranges , twr_ss, twr_ds, tdma with SPI-HCI support.
This app will receive the HCI based commands form the hci_master via SPI, decodes the HCI commands, thereby starts/stops the ranging, sends the ranging data to the hci_master via SPI.
For each command mcu app will prepare the response data and send a GPIO signal to master, saying the data is ready.

### dwm_hci_master:
This app enables user to input the HCI commands through shell, sends those commands to the dwm_hci_mcu app via SPI and waits for GPIO Signal. Upon receiving the GPIO signal , master reads the data from the dwm_hci_mcu by doing a dummy SPI transfer.

### super-node:
super-node acts as ccp/pan master, also responds for nranges request , twr_ss request and twr_ds request from the mcu app.

## Hardware setup:
It is required to Interface two dwm1001-dev boards via SPI lines with external connector wires as mentioned in the below table. The pin numbers mentioned below represents the RPI header number. Connect the same pin numbers with wires.

 |PIN_TYPE|DWM1001-DEV-RPI-HEADER|
 |-----|-----|
 |GPIO-IRQ|15|
 |MISO|21|
 |MOSI|19|
 |CLK|23|
 |CS|24|
 |GND|25|
 

**Note:**
Flash the dwm_hci_mcu and dwm_hci_master apps on to the each of the externally interfaced boards.
Flash the super-node on to a third dwm1001-dev board.

## Building target for the super-node

```no-highlight
newt target create super_node
newt target set super_node app=apps/super-node
newt target set super_node bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set super_node build_profile=debug
newt target amend super_node syscfg=PANMASTER_ISSUER=1
newt create-image super-node 0.1.0 
newt load super_node
```
## BUilding target for HCI MASTER
```
newt target create hci_master
newt target set hci_master app=apps/dwm_hci_master
newt target set hci_master bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set hci_master build_profile=debug
newt create-image hci_master 0.1.0 
newt load hci_master
```
## Building target for MCU TAG
```
newt target create mcu_tag
newt target set mcu_tag app=apps/dwm_hci_mcu
newt target set mcu_tag bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt target set mcu_tag build_profile=debug
newt target amend mcu_tag syscfg=NRNG_NTAGS=4:NRNG_NNODES=8:NRNG_NFRAMES=16:NODE_START_SLOT_ID=0:NODE_END_SLOT_ID=7
newt run mcu_tag 0.1.0
```
# Testing Procedure:
1. Open the serial console of dwm_hci_master, and input the commands in the shell, which will be sent to the dwm_hci_mcu node via SPI.
2. Type hci_send command for the list of HCI commands

```
> hci_send

Less No. of arguments

usage case::: hci_send  SERVICE_TYPE_VALUE   TASK_TYPE_VALUE


         Available SERVICE_TYPES and TASK_TYPES
---------------------------------------------------------------
|    SERVICE_TYPE   : value  |   TASK_TYPE           : value  |
---------------------------------------------------------------
     DW_HCI_TDMA    :   1    |   DW_HCI_START        :   0
     DW_HCI_TWR_SS  :   2    |   DW_HCI_STOP         :   1
     DW_HCI_TWR_DS  :   3    |   DW_HCI_GET_DATA     :   2
     DW_HCI_NRNG_SS :   4    |
     DW_HCI_NTDOA   :   5    |
     DW_HCI_PDOA    :   6    |
     DW_HCI_EDM     :   7    |
---------------------------------------------------------------
```
3. Start the any one of the listed services.
Ex: Starting the TWR_SS service
```
hci_send 2 0
command :  2
002271 compat> response command :0
response command :130
002271 data-len 15
002271 twr_ss_STARTED
```
4. Read the data.
Ex: Reading the data of TWR_SS service
```
hci_send 2 2
hci_send 2 2
command : 34
006047 compat> response command :0
response command :162
006047 data-len 141
006047 {"utime": 139594817463165, "twr": {"raz": ["0.403","0.000","0.000"],"uid": "41a1"},"uid": "5ba7", "diag": {"rssi": "-78.438","los": "1.000"}}
```
5. Stop the service
Ex: Stopping the TWR_SS service
```
hci_send 2 1
hci_send 2 1
command : 18
007295 compat> response command :0
response command :146
007296 data-len 15
007296 twr_ss_STOPPED
007296 
```

**NOTE:**
Current mcu app supports only DW_HCI_TDMA, DW_HCI_TWR_SS , DW_HCI_TWR_DS, DW_HCI_NRNG_SS.

