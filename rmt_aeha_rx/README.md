# _RMT AEHA_RX Example_

This example uses the remote control (RMT) peripheral to receive codes for the AEHA infrared remote protocol.

## How to Use Example

### Hardware Required

* A development board with ESP32 SoC (e.g., ESP32-DevKitC, ESP-WROVER-KIT, etc.)
* A USB cable for Power supply and programming
* IR receiver Module

You need to connect a IR receiver to GPIO19. 

The RX pin can be modified in top of the main/infrared_aeha_main.c file.

```
#define RMT_RX_GPIO_NUM  19     /*!< GPIO number for receiver */
```

### Configure the Project

```
make menuconfig
```

* Set serial port under Serial Flasher Options.

### Build and Flash

Build the project and flash it to the board, then run monitor tool to view serial output:

```
make -j4 flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)

See the Getting Started Guide for full steps to configure and use ESP-IDF to build projects.

## Example Output

Run this example, you will see the following output log:
```
AEHA: RMT RCV rx_size=200
AEHA: RMT RCV offset=0 item_num=50 res=49
AEHA: RMT RCV --- customer: 0x2002 parity: 0x00 index: 4
AEHA: RMT RCV --- data[0]: 0x08
AEHA: RMT RCV --- data[1]: 0x00
AEHA: RMT RCV --- data[2]: 0x3d
AEHA: RMT RCV --- data[3]: 0xbd
AEHA: RMT RCV offset=50 item_num=0 res=-1

```

