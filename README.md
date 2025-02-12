# QMK Raw HID Router

This is a router program that facilitates communication between qmk devices which are using the raw hid feature.

For example if you want to change the dpi/scroll speed of your qmk mouse/trackball using your qmk keyboard. Or change other settings via one qmk device to another.

You can also use this to send some stuff to the pc, like controlling stuff in obs for example if you don't want to use normal key combos. Currently sending info strings to the pc is supported. This is handy to see for example what dpi setting you are on.

Written in C since qmk firmware is also using C so its easier to keep track of things and reuse code. 

```txt
   +-----------------+
   |                 |
   |   QMK Keyboard  |
   |                 |
   +--------+--------+
            |
            |  Data
            v
   +--------------------+
   |                    |
   | QMK RAW HID Router |
   |                    |
   +--------+-----------+
            |
            |  Forwarded Data
            v
   +-----------------+
   |                 |
   |   QMK Trackball |
   |                 |
   +-----------------+

```
# Running

After going through the installations steps, you can check if its working by running

`./qmk_hid_router --show` to see all of your devices

If you are having some issues you can turn on logging which will help you see the raw packets.

`./qmk_hid_router -vv`

You can unplug and plugin devices as you please, the program handles hotswapping and refreshes the device list every 20 seconds. You can decrease it if you are removing and connecting devices regularly. 

Without any command line arguements, the only thing you will see is error messages or info packets that are send specifically for the pc to display as text.

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

## Windows (W.I.P. need to check this myself) 

you will need to download the zip [here](https://github.com/libusb/hidapi/releases) and extract it into the project into `hidapi-win` folder

Then using [chocolatey](https://chocolatey.org/install) install `choco install mingw` (this is to get gcc on windows)

Make sure your pid and vid is completely different between your devices otherwise windows will ignore one of them.

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

# Videos

### Installation

# References

https://github.com/libusb/hidapi/blob/master/hidtest/test.c