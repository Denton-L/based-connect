import sys
import binascii
import os
import subprocess
import argparse

PRODUCT_INFO = {0: "FUNCTION_BLOCK_INFO",
 1: "BMAP_VERSION",
 2: "ALL_FUNCTION_BLOCKS",
 3: "PRODUCT_ID_VARIANT",
 4: "GET_ALL_FUNCTIONS",
 5: "FIRMWARE_VERSION",
 6: "MAC_ADDRESS",
 7: "SERIAL_NUMBER",
 10: "HARDWARE_REVISION",
 11: "COMPONENT_DEVICES",
 }

DEVICE_MANAGEMENT = {
	0: "FUNCTION_BLOCK_INFO",
	1: "CONNECT",
	2: "DISCONNECT",
	3: "REMOVE_DEVICE",
	4: "LIST_DEVICES",
	5: "INFO",
	6: "EXTENDED_INFO",
	7: "CLEAR_DEVICE_LIST",
	8: "PAIRING_MODE",
	9: "LOCAL_MAC_ADDRESS",
	10: "PREPARE_P2P",
	11: "P2P_MODE",
	12: "ROUTING",
}

AUDIO_MANAGEMENT = {0: "FUNCTION_BLOCK_INFO",
1: "SOURCE",
2: "GET_ALL",
3: "CONTROL",
4: "STATUS",
5: "VOLUME",
6: "NOW_PLAYING",
}

SETTINGS = {
0: "FUNCTION_BLOCK_INFO",
1: "GET_ALL",
2: "PRODUCT_NAME",
3: "VOICE_PROMPTS",
4: "STANDBY_TIMER",
5: "CNC",
6: "ANR",
7: "BASS_CONTROL",
8: "ALERTS",
9: "BUTTONS",
10: "MULTIPOINT",
11: "SIDETONE",
21: "IMU_VOLUME_CONTROL",
}

STATUS = {
0: "FUNCTION_BLOCK_INFO",
1: "GET_ALL_FUNCTIONS",
2: "BATTERY_LEVEL",
3: "AUX_CABLE_DETECTION",
4: "MIC_LEVEL",
5: "CHARGER_DETECT",
}

CONTROL = {
0: "FUNCTION_BLOCK_INFO",
1: "GET_ALL",
2: "CHIRP",
}

VPA = {
0: "FUNCTION_BLOCK_INFO",
1: "GET_ALL",
2: "SUPPORTED_VPAS",
}

FUNCTION_BLOCKS = {
	0: ("PRODUCT_INFO", PRODUCT_INFO),
	1: ("SETTINGS", SETTINGS),
	2: ("STATUS", STATUS),
	3: ("FIRMWARE_UPDATE", {}),
	4: ("DEVICE_MANAGEMENT", DEVICE_MANAGEMENT),
	5: ("AUDIO_MANAGEMENT", AUDIO_MANAGEMENT),
	6: ("CALL_MANAGEMENT", {}),
	7: ("CONTROL", CONTROL),
	8: ("DEBUG", {}),
	9: ("NOTIFICATION", {}),
	10: ("RESERVED_BOSEBUILD_1", {}),
	11: ("RESERVED_BOSEBUILD_2", {}),
	12: ("HEARING_ASSISTANCE", {}),
	13: ("DATA_COLLECTION", {}),
	14: ("HEART_RATE", {}),
	16: ("VPA", VPA),
	21: ("AUGMENTED_REALITY", {}),
}

OPERATORS = {
0: "SET",
1: "GET",
2: "SET_GET",
3: "STATUS",
4: "ERROR",
5: "START",
6: "RESULT",
7: "PROCESSING",
}

ERRORS = {
	1: ("LENGTH", "Invalid length"),
	2: ("CHKSUM", "Invalid Checksum"),
	3: ("FBLOCK_NOT_SUPP", "FBlock not supported"),
	4: ("FUNC_NOT_SUPP", "Function not supported"),
	5: ("OP_NOT_SUPP", "Operator is not supported for that function."),
	6: ("INVALID_DATA", "Data values sent to headset are incorrect."),
	7: ("DATA_UNAVAILABLE", "Requested data is not available"),
	8: ("RUNTIME", "Failure to read/write the information requested that is temporary."),
	9: ("TIMEOUT", "Timeout related errors."),
	10: ("INVALID_STATE", "Action requested is not applicable to the current state."),
	11: ("DEVICE_NOT_FOUND", "Device not found in Paired Device List"),
	12: ("BUSY", "Device is busy to service the BMAP message"),
	13: ("NOCONN_TIMEOUT", "Soundlink device fails to connect to a device in the Paired Device List"),
	14: ("NOCONN_KEY", "Soundlink device fails to connect to a device because the pairing information has been deleted from the source device."),
	15: ("OTA_UPDATE", "OTA firmware update cannot be initialized because an update is already in progress"),
	16: ("OTA_LOW_BATT", "OTA firmware update cannot be initialized because product battery voltage is too low"),
	17: ("OTA_NO_CHARGER", "OTA firmware update cannot be applied because charger is not connected"),
	-1: ("FBLOCK_SPECIFIC", "Error code is Function Block specific and an extra byte will be included in the payload to differentiate between different FBlock specific error codes. Refer to the respective FBlock section for a list of error codes for that particular FBlock."),
}

TSHARK_WINDOWS = r'C:\Program Files\Wireshark\tshark.exe'
TSHARK = TSHARK_WINDOWS if os.path.exists(TSHARK_WINDOWS) else "tshark"

def parse_packet(p):
	try:
		return ((p[0], p[1], p[2], p[4: 4 + p[3]]), p[4 + p[3]:])
	except IndexError:
		return (None, b'')

def parse_packets(p):
	rest = p
	while rest:
		(p, rest) = parse_packet(rest)
		yield p


def parse_packets_readable(p):
	for p in parse_packets(p):
		if p is None:
			yield None
		else:
			yield (f"{FUNCTION_BLOCKS[p[0]][0]} ({p[0]})", \
				   f"{FUNCTION_BLOCKS[p[0]][1].get(p[1], 'UNKNOWN')} ({p[1]})", \
				   f"{OPERATORS.get(p[2], 'UNKNOWN')} ({p[2]})", \
				   f"{ERRORS[p[3][0]]} ({p[3][0]})" if p[2] == 4 and len(p[3]) == 1 else p[3])
		
	

def parse_args():
	parser = argparse.ArgumentParser()
	parser.add_argument("tracefile", help = "Wireshark pcap file")
	return parser.parse_args()


def main(args):
	proc = subprocess.run((TSHARK, "-r", args.tracefile, "-e", "hci_h4.direction", "-e", "btspp.data", "-Tfields"), capture_output = True)

	text = proc.stdout.decode(encoding = "ascii").replace("\r\n", "\n")

	a = [x.split("\t") for x in text.split("\n") if len(x.split("\t")) > 1 and x.split("\t")[1]]
	b = [(x[0], binascii.unhexlify(x[1])) for x in a]
	c = [x[1] for x in b if x[0] == '0x00']
	#d = [(FUNCTION_BLOCKS[y[0]][0], FUNCTION_BLOCKS[y[0]][0].get(y[1], ""), y[2], y[3]) for y in parse_packets(x[1][0]) if x[0] == '0x00' else x[1] for x in b]
	d = [("=>" if x[0] == '0x00' else "<=", list(parse_packets_readable(x[1]))) for x in b]

	for x in d:
		print(x)


if __name__ == "__main__":
	main(parse_args())

