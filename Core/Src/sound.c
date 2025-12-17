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

uint16_t soundWaveformShot[SOUND_SHOT_LENGTH];
uint16_t soundWaveformPause[SOUND_PAUSE_LENGTH];
uint16_t soundWaveformExplosion[SOUND_EXPLOSION_LENGTH];
uint16_t soundWaveformDamage[SOUND_DAMAGE_LENGTH];
uint16_t soundWavefromGameStart[SOUND_GAMESTART_LENGTH];
uint16_t soundWaveformGameEnd[SOUND_GAMEEND_LENGTH];
uint16_t soundWaveformPickup[SOUND_PICKUP_LENGTH];

enum SoundType previousSound = soundGameStart;

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
	  for (int i = 0; i < SOUND_EXPLOSION_LENGTH; i++)
	  {
		  soundWaveformExplosion[i] = rand() % 4095;

		  if (i > 1000)
		  {
			  soundWaveformExplosion[i] = soundWaveformExplosion[i] * 3 / 4;
			  if (i > 2000)
				  soundWaveformExplosion[i] = soundWaveformExplosion[i] * 3 / 4;
			  if ((i / 20) % 2 == 0)
				  soundWaveformExplosion[i] = 0;
		  }
	  }

	  GenerateWaveform(tuneShot, soundWaveformShot, SOUND_SHOT_LENGTH);
	  GenerateWaveform(tuneDamage, soundWaveformDamage, SOUND_DAMAGE_LENGTH);
	  GenerateWaveform(tunePickup, soundWaveformPickup, SOUND_PICKUP_LENGTH);
	  GenerateWaveform(tuneGameStart, soundWavefromGameStart, SOUND_GAMESTART_LENGTH);
	  GenerateWaveform(tuneGameEnd, soundWaveformGameEnd, SOUND_GAMEEND_LENGTH);
	  GenerateWaveform(tunePause, soundWaveformPause, SOUND_PAUSE_LENGTH);
}

void PlaySound(enum SoundType sound)
{
	if (config.init == 0)
	{
		return;
	}

	if (!(sound == soundShot || previousSound == soundPlayerExplosion))
	{
		HAL_DAC_Stop_DMA(config.hdac, config.channel);
		HAL_TIM_Base_Stop(config.htim);
	}

	switch(sound)
	{
	case soundShot:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformShot, SOUND_SHOT_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundExplosion:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformExplosion, SOUND_EXPLOSION_LENGTH / 3, DAC_ALIGN_12B_R);
		break;
	case soundPlayerExplosion:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformExplosion, SOUND_EXPLOSION_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundDamage:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformDamage, SOUND_DAMAGE_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundGameStart:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWavefromGameStart, SOUND_GAMESTART_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundGameEnd:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformGameEnd, SOUND_GAMEEND_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundPickup:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformPickup, SOUND_PICKUP_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundPause:
		HAL_DAC_Start_DMA(config.hdac, config.channel, (uint32_t*)soundWaveformPause, SOUND_PAUSE_LENGTH, DAC_ALIGN_12B_R);
		break;
	}
	HAL_TIM_Base_Start(config.htim);
	previousSound = sound;

}
