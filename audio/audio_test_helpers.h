#ifndef AUDIO_TEST_HELPER_H
#define AUDIO_TEST_HELPER_H

#include "Libraries/kio/kio.h"

extern int8 ALaw_Encode(int16 number);
extern int16 ALaw_Decode(int8 number);
extern int8 MuLaw_Encode(int16 number);
extern int16 MuLaw_Decode(int8 number);

#endif


















