HID++ library and tools
=======================

I wrote this library as a personal hack tool for testing Logitech's HID++ protocol. It also contains tools for configuring complex mouse profiles in persistent or temporary memory.

For simpler profiles, you may prefer to use [libratbag](https://github.com/libratbag/libratbag).

Documentation for HID++ 1.0 mice, can be found on [my G500 repository](https://github.com/cvuchener/g500/tree/master/doc). Other varied documents (HID++ 1.0 receivers, HID++ 2.0 devices) from Logitech can be found on [this Google drive](https://drive.google.com/folderview?id=0BxbRzx7vEV7eWmgwazJ3NUFfQ28). Peter Wu has some HID++ 2.0 features documentation on [his website](https://lekensteyn.nl/files/logitech/).


Building
--------

Building requires a C++14 compiler and cmake.

The library can be built with different HID backend (using the `HID_BACKEND` cmake variable, default is set according the current operating system).
 - `linux` uses Linux hidraw and **libudev**.
 - `windows` uses Microsoft Windows HID API.

Profile tools use **TinyXML2** for parsing and writing profiles.

Use cmake to build the library and tools.

```
mkdir build
cd build
cmake ..
make
```

Library and tools can be installed with `make install`.

### CMake options

 - `INSTALL_UDEV_RULES`: install an udev rule for adding user access to HID++ devices. This will add a file in `/etc/udev/rules.d` (not in `CMAKE_INSTALL_PREFIX`). Run `udevadm control --reload` and `udevadm trigger` after the installation for updating udev rules and already present devices.


Commands
--------

Most commands use a device path for interacting with the device. For some devices (wireless or older HID++ 1.0 devices), you need to add a *device index* with the `-d` or `--device` in order to access the device instead of the receiver. Use `hidpp-list-devices` to discover plugged devices and their respective device path and index.

### Check HID++ protocol

    hidpp-check-device *device_path*

This command print the protocol version of the device if it supports HID++. Otherwise it return a non-zero code.


### List HID++ devices

    hidpp-list-devices

List every HID++ devices that can be opened (run as root if the device is not visible).


### Discover HID++ features or registers

    hidpp-list-features *device_path*

List every feature or register available on the device. For HID++ 1.0, register are discovered by trying to read or write them, this command does not try writing unless the `-w` or `--write` options are given.


### Dump and write HID++ 1.0 device memory

    hidpp10-dump-page *device_path* *page*

Dump the content of page *page* in the HID++ 1.0 device memory in stdout.

    hidpp10-write-page *device_path* *page*

Write the content of stdin in page *page* of the HID++ 1.0 device.


### On-board profiles

Profiles are stored in XML format, see *profile_format.md* for details.

    hidpp-persistent-profiles *device_path* read [*file*]

Read the persistent profiles from the device and write them in XML format in *file* or stdout.

    hidpp-persistent-profiles *device_path* write [*file*]

Write the persistent profiles from the XML in *file* or stdin to the device.

Supported devices:
 - G9 (experimental, untested)
 - G9x, G500, G500s
 - G700, G700s (experimental, untested)
 - HID++2.0 or later supporting On-board profiles (feature 0x8100) with profile format 1, 2 or 3 (only format 2 was tested with a G502 spectrum, other formats may be incomplete) and macro format 1. Use `hidpp20-onboard-profiles-get-description` to get the format used by the device.


### HID++ 1.0 profile management

    hidpp10-load-temp-profile *device_path* [*file*]

Write the profile from the XML in *file* or stdin to the device temporary memory and load it.

    hidpp10-active-profile *device_path* current

Get the index of the current profile (or `default` if the factory default is loaded).

    hidpp10-active-profile *device_path* load *index*

Load the profile *index* from persistent memory.

    hidpp10-active-profile *device_path* load-default

Load the factory default profile.

    hidpp10-active-profile *device_path* load-address *page* [*offset*]

Load the profile at the given address (default value for *offset* is 0).

    hidpp10-active-profile reload

Reload the current profile from persistent memory (useful to get back to the last profile after `hidpp10-load-temp-profile`).


### Changing mouse resolution

    hidpp-mouse-resolution *device_path* get

Get the current resolution from the mouse.

    hidpp-mouse-resolution *device_path* set *x_dpi* [*y_dpi*]

Set the current resolution for the mouse. If only one resolution is given, both axes use *x_dpi*. Some mice do not support per-axis resolution.

    hidpp-mouse-resolution *device_path* info

Print informations about supported resolutions.

Supported devices: G5, G9, G9x, G500, G500x, G700, G700s, HID++2.0 or later devices with “Adjustable DPI” feature (0x2201).


### Advanced HID++ 1.0 commands

    hidpp10-raw-command *device_path* *command* read|write short|long [*parameters*...]

Used for raw interaction with HID++ 1.0 register *command*. Parameters are hexadecimal and default are zeroes.


### Advanced HID++ 2.0 or later commands

    hidpp20-call-function *device_path* *feature_index* *function* [*parameters*...]

Call the low-level function given by `feature_index` and `function`. Parameters are hexadecimal and default are zeroes.

