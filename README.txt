based-connect
=============

If you own a Bose device, you'll know that Bose Connect is not available on
Linux. This program attempts to reverse engineer that app in order to give the
device Linux support.

Usage
-----

./based-connect [options] <address>
	address: The Bluetooth address of the device.

Options:
	-h, --help
		Print the help message.

	-n <name>, --name=<name>
		Change the name of the device.

	-c <level>, --noise-cancelling=<level>
		Change the noise cancelling level.
		level: high, low, off

	-o <minutes>, --auto-off=<minutes>
		Change the auto-off time.
		minutes: never, 5, 20, 40, 60, 180

	-l <language>, --prompt-language=<language>
		Change the voice-prompt language.
		language: en, fr, it, de, es, pt, zh, ko, nl, ja, sv

	-v <switch>, --voice-prompts=<switch>
		Change whether voice-prompts are on or off.
		switch: on, off

	-d, --device-status
		Print the device status information. This includes its name, language,
		voice-prompts, auto-off and noise cancelling settings.

	-p <status>, --pairing=<status>
		Change whether the device is pairing.
		status: on, off

	-f, --firmware-version
		Print the firmware version on the device.

	-s, --serial-number
		Print the serial number of the device.

	-b, --battery-level
		Print the battery level of the device as a percent.

	-a, --paired-devices
		Print the devices currently connected to the device.
		!: indicates the current device
		*: indicates other connected devices
                S: indicates a device that music is shared to

        --connect-device=<address>
                Attempt to connect to the device at address.

        --connect-music-share=<puppet-address>,<source-address>
                Attempt to connect to the puppet device at address
                (secondary headphones) and share music from source
                address (computer).

	--disconnect-device=<address>
		Disconnect the device at address.

	--remove-device=<address>
		Remove the device at address from the pairing list.

	--device-id
		Print the device id followed by the index revision.

Building
--------

Create a build directory (e.g., `mkdir build`) and enter it. Run
```sh
cmake ..
```
to create a Makefile, and then run
```sh
cmake --build .
```
to build the project.

On Windows, you'll need `vcpkg` with the `getopt` package installed. Run
```powershell
cmake -DCMAKE_TOOLCHAIN_FILE="$env:VCPKG_ROOT\scripts\buildsystems\vcpkg.cmake" ..
```
to create the build files.

Installing
----------

Run `make install` to install the program. The `PREFIX` and `DESTDIR` variables
are assignable and have the traditional meaning.

Alternatively, if you run Arch Linux, you can download it from the AUR here:

https://aur.archlinux.org/packages/based-connect-git/

Running
-------

On Windows, use
```powershell
Get-PnpDevice -class Bluetooth | Select-Object -Property FriendlyName,
DeviceID
```
to enumerate the Bluetooth device ID. The device ID needs to be written as
AA:BB:CC:DD:EE:FF.

Dependencies
------------

* BlueZ
	* bluez-libs on Arch Linux
	* libbluetooth-dev on Debian and Ubuntu

Disclaimer
----------

This has only been tested on Bose QuietComfort 35's with firmware 1.3.2, 1.2.9,
1.06 and SoundLink II's with firmware 2.1.1. I cannot ensure that this program
works on any other devices.

Todo
----

* Implement "getters" for current headphone state
	* Current status of all setters currently implemented
	* Date of manufacturing
* Get/set volume
* Port to MacOS (and maybe Windows)
* Firmware updates?

Firmware Details
----------------

See details.txt for partly reverse engineered but unimplemented (and unknown)
details.
