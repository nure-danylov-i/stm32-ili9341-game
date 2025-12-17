#ifndef INC_SOUND_H_
#define INC_SOUND_H_

#include <stdint.h>

#define SOUND_SAMPLES 1000

#define SOUND_SHOT_LENGTH 1200
#define SOUND_DAMAGE_LENGTH 1200
#define SOUND_PAUSE_LENGTH 2800
#define SOUND_PICKUP_LENGTH 4000
#define SOUND_EXPLOSION_LENGTH 3000
#define SOUND_GAMESTART_LENGTH 8000
#define SOUND_GAMEEND_LENGTH 8000

enum SoundType
{
	soundGameStart,
	soundGameEnd,
	soundShot,
	soundExplosion,
	soundDamage,
	soundPickup,
	soundPlayerExplosion,
	soundPause
};

void GenerateWaveform(const uint16_t *tune, uint16_t *waveform, unsigned int length);
void GenerateSounds();
void PlaySound(enum SoundType sound);

#endif /* INC_SOUND_H_ */
