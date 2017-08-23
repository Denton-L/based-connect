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

ssize_t set_name(int sock, const char *name);
ssize_t noise_cancelling(int sock, char level);
ssize_t voice_prompts(int sock, char setting);
ssize_t auto_off(int sock, unsigned char minutes);

#endif
