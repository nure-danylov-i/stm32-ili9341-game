#ifndef INC_SOUND_H_
#define INC_SOUND_H_

#include <stdint.h>

#include <stm32f3xx_hal.h>

#define SAMPLE_RATE 8000

#define WAVETABLE_LENGTH 100

enum SoundType
{
	soundGameStart = 0,
	soundGameEnd,
	soundShot,
	soundExplosion,
	soundDamage,
	soundPickup,
	soundPlayerExplosion,
	soundPause
};

enum WaveType {
	waveSquare = 0,
	waveNoise,
	waveSaw,
};

struct Recipe {
	enum WaveType waveType;
	float frequency;
	float durationSec;
};

struct SFX {
	const struct Recipe *recipes;
	uint16_t recipeCount;
};


struct Oscilator {
	float frequency;
	float phase;
	float increment;
	uint8_t active;
	uint32_t samplesLeft;

	struct SFX *sfx;
	uint16_t recipeCurrent;

	uint16_t cachedNoiseSample;
};

uint8_t InitSound(DAC_HandleTypeDef *hdac, uint32_t channel, TIM_HandleTypeDef *htim);
void PlaySound(enum SoundType sound);
void SoundCallback();

#endif /* INC_SOUND_H_ */
