#ifndef BASED_H
#define BASED_H

#define BOSE_CHANNEL 8

#define MAX_NAME_LEN 0xfe

enum NoiseCancelling {
	NC_HIGH = 0x01,
	NC_LOW = 0x03,
	NC_OFF = 0x00
};

enum AutoOff {
	AO_NEVER = 0,
	AO_5_MIN = 5,
	AO_20_MIN = 20,
	AO_40_MIN = 40,
	AO_60_MIN = 60,
	AO_180_MIN = 180
};

enum PromptLanguage {
	PL_OFF = 0x01,
	PL_EN = 0x21,
	PL_FR = 0x22,
	PL_IT = 0x23,
	PL_DE = 0x24,
	PL_ES = 0x26,
	PL_PT = 0x27,
	PL_ZH = 0x28,
	PL_KO = 0x29,
	PL_NL = 0x2e,
	PL_JA = 0x2f,
	PL_SV = 0x32
};

int set_name(int sock, const char *name);
int set_noise_cancelling(int sock, enum NoiseCancelling level);
int set_auto_off(int sock, enum AutoOff minutes);
int set_prompt_language(int sock, enum PromptLanguage language);

#endif
