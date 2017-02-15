HID++ library and tools
=======================

I wrote this library as a personal hack tool for testing Logitech's HID++ protocol. It also contains tools for configuring complex mouse profiles in persistent or temporary memory.

For simpler profiles, you may prefer to use [libratbag](https://github.com/libratbag/libratbag).

Documentation for HID++ 1.0 mice, can be found on [my G500 repository](https://github.com/cvuchener/g500/tree/master/doc). Other varied documents (HID++ 1.0 receivers, HID++ 2.0 devices) from Logitech can be found on [this Google drive](https://drive.google.com/folderview?id=0BxbRzx7vEV7eWmgwazJ3NUFfQ28).


Building
--------

The library has no dependency except for C++11 standard library and Linux hidraw API.

`hidpp-list-devices` depends on **libudev**. Profile tools use **TinyXML2** for parsing and writing profiles.

Use cmake to build the library and tools.

```
mkdir build
cd build
cmake ..
make
```


Commands
--------

Most commands use a hidraw device node for interacting with the device. For some wireless devices, you need to add a *device index* with the `-d` or `--device` in order to access the device instead of the receiver. Use `hidpp-list-devices` to discover plugged devices and their respective hidraw node and device index.

### Check HID++ protocol

    hidpp-check-device /dev/hidrawX

This command print the protocol version of the device if it supports HID++. Otherwise it return a non-zero code.


### List HID++ devices

    hidpp-list-devices

List every HID++ devices that can be opened (run as root if the device is not visible).


### Discover HID++ features or registers

    hidpp-list-features /dev/hidrawX

List every feature or register available on the device. For HID++ 1.0, register are discovered by trying to read or write them, this command does not try writing unless the `-w` or `--write` options are given.


### Dump and write HID++ 1.0 device memory

    hidpp10-dump-page /dev/hidrawX page

Dump the content of page *page* in the HID++ 1.0 device memory in stdout.

    hidpp10-write-page /dev/hidrawX page

Write the content of stdin in page *page* of the HID++ 1.0 device.


### On-board profiles

Profiles are stored in XML format, see *profile_format.md* for details.

    hidpp-persistent-profiles /dev/hidrawX read [file]

Read the persistent profiles from the device and write them in XML format in *file* or stdout.

    hidpp-persistent-profiles /dev/hidrawX write [file]

Write the persistent profiles from the XML in *file* or stdin to the device.

Supported devices:
 - G9 (experimental, untested)
 - G9x, G500, G500s
 - G700, G700s (experimental, untested)
 - HID++2.0 or later supporting On-board profiles (feature 0x8100) with profile format 1, 2 or 3 (only format 2 was tested with a G502 spectrum, other formats may be incomplete) and macro format 1. Use `hidpp20-onboard-profiles-get-description` to get the format used by the device.


### HID++ 1.0 profile management

    hidpp10-load-temp-profile /dev/hidrawX [file]

Write the profile from the XML in *file* or stdin to the device temporary memory and load it.

    hidpp10-active-profile /dev/hidrawX current

Get the index of the current profile (or `default` if the factory default is loaded).

    hidpp10-active-profile /dev/hidrawX load index

Load the profile *index* from persistent memory.

    hidpp10-active-profile /dev/hidrawX load-default

Load the factory default profile.

    hidpp10-active-profile /dev/hidrawX load-address page [offset]

Load the profile at the given address (default value for *offset* is 0).

    hidpp10-active-profile reload

Reload the current profile from persistent memory (useful to get back to the last profile after `hidpp10-load-temp-profile`).


### Changing mouse resolution

    hidpp10-mouse-resolution /dev/hidrawX get

Get the current resolution from the mouse.

    hidpp10-mouse-resolution /dev/hidrawX set x_dpi [y_dpi]

Set the current resolution for the mouse. If only one resolution is given, both axes use *x_dpi*. Some mice do not support per-axis resolution.

Supported devices: G5, G9, G9x, G500, G500x, G700, G700s


### Advanced HID++ 1.0 commands

    hidpp10-raw-command /dev/hidrawX command read|write short|long [parameters...]

Used for raw interaction with HID++ 1.0 register *command*. Parameters are hexadecimal and default are zeroes.


### Advanced HID++ 2.0 or later commands

    hidpp20-call-function /dev/hidrawX feature_index function [parameters...]

Call the low-level function given by `feature_index` and `function`. Parameters are hexadecimal and default are zeroes.

