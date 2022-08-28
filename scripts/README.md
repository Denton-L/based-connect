# Scripts

## analyze\_trace.py
This is a dissector for recorded bluetooth PCAP files. Record your trace on an Android phone by activating the "HCI snoop log" in developer mode ([described here](https://source.android.com/docs/core/bluetooth/verifying_debugging#debugging-with-logs)). Then pull the bugreport with `adb bugreport my-bugreport.zip` and get the bluetooth log files from `FS/data/misc/bluetooth/logs` in the zip. I imported the first `.cfa` file into Wireshark and then added the others with Wireshark's `Merge ...` menu option. Save the PCAP, and call the python dissector script on it. Known messages will be decoded as far as the protocol is known, so that the traffic is easier to read.
