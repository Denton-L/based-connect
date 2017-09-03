based-connect
=============

If you own Bose headphones, you'll know that Bose Connect is not available on
Linux. This program attempts to reverse engineer that app in order to give the
headphones Linux support.

Usage
-----

./based-connect [options] <address>
	address: The Bluetooth address of the headphones.

Options:
	-h, --help
		Print the help message.

	-n <name>, --name=<name>
		Change the name of the headphones.

	-c <level>, --noise-cancelling=<level>
		Change the noise cancelling level.
		level: high, low, off

	-o <minutes>, --auto-off=<minutes>
		Change the auto-off time.
		minutes: never, 5, 20, 40, 50, 180

	-l <language>, --prompt-language=<language>
		Change the voice-prompt language.
		language: off, en, fr, it, de, es, pt, zh, ko, nl, ja, sv

	-f, --firmware-version
		Print the firmware version on the headphones.

Disclaimer
----------

I have only tested these on my own Bose QuietComfort 35's with firmware 1.3.2.
I cannot ensure that this program works on any other devies.

Todo
----

* Implement "getters" for current headphone state
	* Battery
	* Current status of all setters currently implemented
	* Serial number
	* Firmware version
	* Date of manufacturing
* Implement device control
* Get/set volume
* Any TODOs in the code
