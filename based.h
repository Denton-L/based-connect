#ifndef BASED_H
#define BASED_H

#include <unistd.h>

#define BOSE_CHANNEL 8

#define NC_HIGH 0x01
#define NC_LOW 0x03
#define NC_OFF 0x00

#define VP_ON 0x21
#define VP_OFF 0x01

#define AO_NEVER 0
#define AO_5MIN 5
#define AO_20MIN 20
#define AO_40MIN 40
#define AO_60MIN 60
#define AO_180MIN 180

#define PL_EN 0x21
#define PL_FR 0x22
#define PL_IT 0x23
#define PL_DE 0x24
#define PL_ES 0x26
#define PL_PT 0x27
#define PL_ZH 0x28
#define PL_KO 0x29
#define PL_NL 0x2e
#define PL_JA 0x2f
#define PL_SV 0x32

ssize_t set_name(int sock, const char *name);
ssize_t noise_cancelling(int sock, char level);
ssize_t voice_prompts(int sock, char setting);
ssize_t auto_off(int sock, unsigned char minutes);
ssize_t prompt_language(int sock, char language);

#endif
