based-connect
=============

TODO: fill out this description

Development Notes
-----------------

* Changing names

In order to change the name of the device, it appears that the following SPP
packet is sent: `01 02 02 l n*`, where l is the number of characters in the new
name and n* is the new name as a string.

For example, here is the full packet for setting the name to "new test":

0000   02 02 20 15 00 11 00 85 04 43 ff 19 02 01 02 02
0010   08 6e 65 77 20 74 65 73 74 38

For some reason, the following SPP packet will always be `01 01 05 00`. Not
sure if this is necessary.
