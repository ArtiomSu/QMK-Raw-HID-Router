# QMK Raw HID Router

This is a router program that facilitates communication between qmk devices which are using the raw hid feature.

It is very efficient, using 0% cpu and 0.5MB RAM.

For example if you want to change the dpi/scroll speed of your qmk mouse/trackball using your qmk keyboard. Or change other settings via one qmk device to another.

You can also use this to send some stuff to the pc, like controlling stuff in obs for example if you don't want to use normal key combos. Currently sending info strings to the pc is supported. This is handy to see for example what dpi setting you are on.

You can also send a broadcast message from one qmk device to the rest of the qmk devices. This saves the need to send multiple packets from the qmk firmware if you want to send a message to many devices.

Written in C since qmk firmware is also using C so its easier to keep track of things and reuse code. 

```txt
   +-----------------+
   |                 |
   |   QMK Keyboard  |
   |                 |
   +--------+--------+
            ^
            |  Data
            v
   +--------------------+
   |                    |
   | QMK RAW HID Router |
   |                    |
   +--------+-----------+
            ^
            |  Forwarded Data
            v
   +-----------------+
   |                 |
   |   QMK Trackball |
   |                 |
   +-----------------+

```
# Table of Contents

- [Running](#running)
- [Installing on PC](#installing-on-pc)
  - [Linux](#linux)
    - [Compiling on linux](#compiling-on-linux)
  - [Macos](#macos)
    - [Compiling on Macos](#compiling-on-macos)
  - [Windows](#windows)
    - [Compiling on Windows](#compiling-on-windows)
- [Installing on QMK Devices](#installing-on-qmk-devices)
- [Cannot open hid device on linux fix](#cannot-open-hid-device-on-linux-fix)
- [Routing protocol](#routing-protocol)
  - [Special PIDs and VIDs](#special-pids-and-vids)
- [Videos](#videos)
- [References](#references)


# Running

After going through the installations steps, you can check if its working by running

```sh
./qmk_hid_router --show
```
to see all of your devices

If you are having some issues you can turn on logging which will help you see the raw packets.

```sh
./qmk_hid_router -vv
```

You can unplug and plugin devices as you please, the program handles hotswapping and refreshes the device list every 10 seconds. You can decrease it if you are removing and connecting devices regularly. 

Without any command line arguements, the only thing you will see is error messages or info packets that are send specifically for the pc to display as text.

You can change the latency by running.

```sh
./qmk_hid_router -r 1
```

In this case this will set the latency to 1 millisecond, which still doesn't have any load on the cpu. 10 milliseconds is the default which is more than enough, but if you want it faster can just set it as 1 and it will be fine.

You can disable refreshing the device list with

```sh
./qmk_hid_router --no-refresh
```
Refreshing the device list on windows memory leaks slowly, so its best to use this. If you aren't disconnecting your devices often this won't make a big difference.

```sh
./qmk_hid_router --help
```
This will print the help menu.

You can enable multiple options at once like so

```sh
./qmk_hid_router -vv -r 1 --no-refresh
```

# Installing on PC

You will also need to install hidapi.

## Linux

```sh
pacman -Sy hidapi
```
### Compiling on linux

```sh

gcc qmk_hid_router.c qmk_hid_router_utilities.c qmk_hid_router_protocol.c -o qmk_hid_router -I/usr/include/hidapi -lhidapi-hidraw && ./qmk_hid_router
```

## Macos

```sh
brew install hidapi
```

### Compiling on Macos

```sh
gcc qmk_hid_router.c qmk_hid_router_utilities.c qmk_hid_router_protocol.c -o qmk_hid_router -I/opt/homebrew/include/hidapi -L/opt/homebrew/lib -lhidapi
```

## Windows

you will need to download the zip [here](https://github.com/libusb/hidapi/releases) and extract it into the project into `hidapi-win` folder

Then using [chocolatey](https://chocolatey.org/install) install `choco install mingw` (this is to get gcc on windows)

Make sure your pid and vid is completely different between your devices otherwise windows will ignore one of them.

Windows binary can be found in the github releases as it takes alot of files to compile it unlike the other two OS's. If the binary doesn't work, you are probably missing some shared libraries so you will just have to compile it.

### Compiling on Windows

```sh
gcc qmk_hid_router.c qmk_hid_router_utilities.c qmk_hid_router_protocol.c -o qmk_hid_router -I./hidapi-win/include -L./hidapi-win/x64 -lhidapi
```
You will need to have `hidapi.dll` in the same folder as the exe otherwise it won't run. The .dll is found in the first zip inside `x64` folder, so you can just move it to wherever your .exe is.


# Installing on QMK Devices

This uses the [raw hid feature](https://docs.qmk.fm/features/rawhid).

There is an example in this repo inside `qmk_keyboard_example`. 

You just need to copy most of it into your own project. 

See video for more details, where I add this feature to the ploopy adept.

### Examples of my keyboards configured with this, that you can use are reference.

[TrackBall](https://github.com/ArtiomSu/qmk_firmware/tree/artiomsu_trackball/keyboards/artiomsu_trackball)

[Dactyl Manuform Keyboard](https://github.com/ArtiomSu/qmk_firmware/tree/artiom_dactyl/keyboards/artiomsu_dactyl)

[Ploopy Adept (Coming soon)](https://github.com/ArtiomSu/qmk_firmware/tree/artiomsu_ploopy_adept/keyboards/ploopy_adept)

# Cannot open hid device on linux fix

create `/usr/lib/udev/rules.d/99-usb-qmk-raw-hid.rules`

sudo chmod 644 /usr/lib/udev/rules.d/99-usb-qmk-raw-hid.rules

inside the file put in
```
KERNEL=="hidraw*", SUBSYSTEM=="hidraw", MODE="0666", TAG+="uaccess", TAG+="udev-acl"
```

then 
```sh
sudo udevadm control --reload-rules
sudo udevadm trigger
```

# Routing protocol
The router generally forwards packets as is. However there are some packets that are treated differently.

The picos can only accept raw hid packets that are 32 bytes in length, this is how it is in qmk for picos. Other mcus can handle 64 bytes so you can update the code here to support that, but I just use picos for everything.

You must follow the `qmk_hid_packet` structure defined in `qmk_hid_router.h`.

The `payload` and `operation` can be whatever you want just make sure the qmk devices understand each other. The router doesn't look at the `operation` unless its `HID_RAW_OP_INFO` which will then just display the message from the qmk device on the pc.

when the router receives a packet and forwards it to another qmk device it will change the `to_vid` and `to_pid` to the qmk device that sent the packet. This way the qmk device receiving the packet knows who sent it and you can write logic to only accept packets from 1 specific device for example. Its done this way so that you have 4 more bytes available for the payload otherwise it would take up more fields in the structure.

you can specify custom pid and vid for your qmk device inside the `keyboard.json`, this acts almost like a mac address in this protocol so make sure that all of your devices are unique. And make sure you specify the qmk raw hid usage and usage page inside `config.h`, you can use a custom usage page if you want but make sure you update it everywhere and make sure none of your other usb devices are using the same usage page otherwise it could break the actuall device. 

### Special PIDs and VIDs

On the topic of vid and pid, there are special pids and vids for example if you use `struct raw_hid_client pc = {0x0000, 0x0000};` this will be handled by the pc instead of forwarding it, this is where the info packet comes from.

And if you use `struct raw_hid_client broadcast = {0xffff, 0xffff};` this will broadcast the packet amongst all qmk devices in this case the `to_vid` and `to_pid` remains unchanged from `0xffff` this is so that the qmk device can be configured to ignore broadcasts otherwise it would think its a direct packet. This isn't the best since now the device doesn't know who the packet is from, but I think its an ok compromise to have more bytes available in the payload.


# Videos

### Installation

# References

https://github.com/libusb/hidapi/blob/master/hidtest/test.c