# Toplevel syscfg

## Overview
This project is used to build the default syscfg and sysinit for the cmake build enviorement

Master node (only one allowed per network):
```no-highlight
newt target create syscfg
newt target set syscfg app=apps/syscfg
#newt target set syscfg bsp=@apache-mynewt-core/hw/bsp/native
newt target set syscfg bsp=@mynewt-dw1000-core/hw/bsp/dwm1001
newt build syscfg
```
