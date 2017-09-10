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
		minutes: never, 5, 20, 40, 50, 180

	-l <language>, --prompt-language=<language>
		Change the voice-prompt language.
		language: off, en, fr, it, de, es, pt, zh, ko, nl, ja, sv

	-p <status>, --pairing=<status>
		Change whether the device is pairing.
		status: on, off

	-f, --firmware-version
		Print the firmware version on the device.

	-s, --serial-number
		Print the serial number of the device.

	-b, --battery-level
		Print the battery level of the device as a percent.

	-d, --paired-devices
		Print the devices currently connected to the device.
		!: indicates the current device
		*: indicates other connected devices

	--connect-device=<address>
		Attempts to connect to the device at address.

	--disconnect-device=<address>
		Disconnects the device at address.

	--remove-device=<address>
		Removes the device at address from the pairing list.

Building
--------

Simply run `make -j` to build the program. The executable produced will be
called `based-connect`.

Installing
----------

Run `make install` to install the program. The `PREFIX` and `DESTDIR` variables
are assignable and have the traditional meaning.

Alternatively, if you run Arch Linux, you can download it from the AUR here:

https://aur.archlinux.org/packages/based-connect-git/

Dependencies
------------

* BlueZ
	* bluez-libs on Arch Linux
	* libbluetooth-dev on Debian and Ubuntu

Disclaimer
----------

This has only been tested on Bose QuietComfort 35's with firmware 1.3.2, 1.2.9,
1.06 and SoundLink II's with firmware 2.1.1. I cannot ensure that this program
works on any other devies.

Todo
----

* Implement "getters" for current headphone state
	* Current status of all setters currently implemented
	* Date of manufacturing
* Get/set volume
* Port to MacOS (and maybe Windows)
* Firmware updates?
