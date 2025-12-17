#include "sound.h"
#include <stdlib.h> // rand()

struct SoundConfig
{
	uint8_t init;
	DAC_HandleTypeDef *hdac;
	uint32_t channel;
	TIM_HandleTypeDef *htim;
};

static struct SoundConfig config = {0};


const uint16_t tuneGameEnd[40] = {
    500, 500, 500, 500, 0,
    500, 500, 500, 500, 0,
    500, 500, 500, 500, 0,
    300, 300, 300, 300, 300,
    300, 300, 300, 300, 300,
    300, 300, 300, 300, 300,
    300, 300, 300, 300, 300,
    300, 300, 300, 300, 300,
};

const uint16_t tuneGameStart[40] = {
    200, 200, 200, 200, 0,
    400, 400, 400, 400, 0,
    500, 500, 500, 500, 0,
    200, 200, 200, 200, 0,
    400, 400, 400, 400, 0,
    500, 500, 500, 500, 0,
    800, 800, 800, 800, 800,
    800, 800, 800, 800, 800,
};

const uint16_t tunePickup[20] = {
    400, 400, 400, 400, 0,
    600, 600, 600, 600, 0,
    800, 800, 800, 800, 800,
	800, 0, 0, 0, 0,
};

const uint16_t tuneShot[6] = {
    1000, 1000, 500, 500, 250, 250,
};

const uint16_t tuneDamage[6] = {
    330, 330, 0, 0, 330, 330,
};

const uint16_t tunePause[14] = {
    1000, 1000, 0, 0, 800, 800, 0, 0,
	1000, 1000, 0, 0, 800, 800
};

//uint16_t soundWaveformShot[SOUND_SHOT_LENGTH];
//uint16_t soundWaveformPause[SOUND_PAUSE_LENGTH];
//uint16_t soundWaveformExplosion[SOUND_EXPLOSION_LENGTH];
//uint16_t soundWaveformDamage[SOUND_DAMAGE_LENGTH];
//uint16_t soundWavefromGameStart[SOUND_GAMESTART_LENGTH];
//uint16_t soundWaveformGameEnd[SOUND_GAMEEND_LENGTH];
//uint16_t soundWaveformPickup[SOUND_PICKUP_LENGTH];

uint16_t SquareWavetable[WAVETABLE_LENGTH];

enum SoundType previousSound = soundGameStart;

struct Oscilator osc = {0};

struct Voice voice = {0};

struct SFX sfx[7] = {0};

static void GenerateWaveform(const uint16_t *tune, uint16_t *waveform, unsigned int length);
static void GenerateSounds();

uint8_t InitSound(DAC_HandleTypeDef *hdac, uint32_t channel, TIM_HandleTypeDef *htim)
{
	if (config.init == 1)
	{
		return 0;
	}

	config.hdac = hdac;
	config.channel = channel;
	config.htim = htim;

	GenerateSounds();

	HAL_DAC_Start(config.hdac, config.channel);

	config.init = 1;
	return 1;
}

static void GenerateWaveform(const uint16_t *tune, uint16_t *waveform, unsigned int length)
{
	  for (int i = 0; i < length; i++)
	  {
	      int idx = i / 200;
	      int freq = tune[idx];

	      if (freq == 0) {
	    	  waveform[i] = 0;
	    	  continue;
	      }

	      if ((i / (8000 / freq / 2) ) % 2 == 0)
	      {
	    	  waveform[i] = 4095;
	      }
	      else
	    	  waveform[i] = 0;
	  }

}

static void GenerateSounds()
{
	  // Fill in the wavetables
	  for (uint16_t i = 0; i < WAVETABLE_LENGTH; i++)
	  {
		  SquareWavetable[i] = (i < (WAVETABLE_LENGTH >> 1)) ? 0 : 0xFFF;
	  }

//		soundGameStart = 0,
//		soundGameEnd,
//		soundShot,
//		soundExplosion,
//		soundDamage,
//		soundPickup,
//		soundPlayerExplosion,
//		soundPause

//	  sfx[0].tune = tuneGameStart;
//	  sfx[0].tuneLength = SOUND_GAMESTART_LENGTH / SAMPLES_PER_NOTE;
//	  sfx[1].tune = tuneGameEnd;
//	  sfx[1].tuneLength = SOUND_GAMEEND_LENGTH / SAMPLES_PER_NOTE;
//	  sfx[2].tune = tuneGameEnd;
//	  sfx[2].tuneLength = SOUND_GAMEEND_LENGTH / SAMPLES_PER_NOTE;
}



void SoundCallback()
{
	uint16_t freq = 440;
	osc.active = 1;

	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

	osc.increment = freq * WAVETABLE_LENGTH / SAMPLE_RATE;

	uint16_t outputValue = 0;
	if (osc.active) {
		int index = (int)osc.phase;
		outputValue = SquareWavetable[index];
		osc.phase += osc.increment;

		if (osc.phase >= WAVETABLE_LENGTH) {
			osc.phase -= WAVETABLE_LENGTH;
		}
	}

	HAL_DAC_SetValue(config.hdac, config.channel, DAC_ALIGN_12B_R, outputValue);
}

void PlaySound(enum SoundType sound)
{
	if (config.init == 0)
	{
		return;
	}

	HAL_TIM_Base_Start_IT(config.htim);
	return;

//	if (!(sound == soundShot || previousSound == soundPlayerExplosion))
//	{
//		HAL_DAC_Stop_DMA(config.hdac, config.channel);
//		HAL_TIM_Base_Stop(config.htim);
//	}
//
//	switch(sound)
//	{
//	case soundShot:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformShot, SOUND_SHOT_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	case soundExplosion:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformExplosion, SOUND_EXPLOSION_LENGTH / 3, DAC_ALIGN_12B_R);
//		break;
//	case soundPlayerExplosion:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformExplosion, SOUND_EXPLOSION_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	case soundDamage:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformDamage, SOUND_DAMAGE_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	case soundGameStart:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWavefromGameStart, SOUND_GAMESTART_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	case soundGameEnd:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformGameEnd, SOUND_GAMEEND_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	case soundPickup:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformPickup, SOUND_PICKUP_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	case soundPause:
//		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformPause, SOUND_PAUSE_LENGTH, DAC_ALIGN_12B_R);
//		break;
//	}
//	HAL_TIM_Base_Start(config.htim);
//	previousSound = sound;

}
