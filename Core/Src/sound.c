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

const struct Recipe sfxGameStart[13] = {
		{ waveSquare, 200, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 400, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 500, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 200, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 400, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 500, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 800, 0.25f }
};

const struct Recipe sfxGameEnd[7] = {
		{ waveSquare, 500, 0.1f }, {waveSquare, 0, 0.025f },
		{ waveSquare, 500, 0.1f }, {waveSquare, 0, 0.025f },
		{ waveSquare, 500, 0.1f }, {waveSquare, 0, 0.025f },
		{ waveSquare, 300, 0.625f }
};

const struct Recipe sfxShot[3] = {
		{ waveSquare, 1000, 0.05f },
		{ waveSquare, 500, 0.05f },
		{ waveSquare, 250, 0.05f }
};

const struct Recipe sfxExplosion[1] = {
		{ waveNoise, 1, 0.125f },
};

const struct Recipe sfxDamage[3] = {
		{ waveSquare, 330, 0.05f },
		{ waveSquare, 0, 0.05f },
		{ waveSquare, 330, 0.05f },
};

const struct Recipe sfxPickup[5] = {
		{ waveSquare, 400, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 600, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSquare, 800, 0.15f },
};

const struct Recipe sfxPlayerExplosion[7] = {
		{ waveNoise, 1, 0.125f },
		{ waveNoise, 100, 0.125f },
		{ waveSquare, 200, 0.025f},
		{ waveNoise, 200, 0.025f},
		{ waveSquare, 200, 0.025f},
		{ waveNoise, 200, 0.025f},
		{ waveSquare, 200, 0.025f},
};

const struct Recipe sfxPause[7] = {
		{ waveSquare, 1000, 0.05f }, {waveSquare, 0, 0.05f },
		{ waveSquare, 800, 0.05f }, {waveSquare, 0, 0.05f },
		{ waveSquare, 1000, 0.05f }, {waveSquare, 0, 0.05f },
		{ waveSquare, 800, 0.05f },
};

struct SFX sfx[7] = {0};

//uint16_t soundWaveformShot[SOUND_SHOT_LENGTH];
//uint16_t soundWaveformPause[SOUND_PAUSE_LENGTH];
//uint16_t soundWaveformExplosion[SOUND_EXPLOSION_LENGTH];
//uint16_t soundWaveformDamage[SOUND_DAMAGE_LENGTH];
//uint16_t soundWavefromGameStart[SOUND_GAMESTART_LENGTH];
//uint16_t soundWaveformGameEnd[SOUND_GAMEEND_LENGTH];
//uint16_t soundWaveformPickup[SOUND_PICKUP_LENGTH];

uint16_t SquareWavetable[WAVETABLE_LENGTH];
uint16_t NoiseWavetable[WAVETABLE_LENGTH];

enum SoundType previousSound = soundGameStart;

struct Oscilator osc = {0};

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
	HAL_TIM_Base_Start_IT(config.htim);

	config.init = 1;
	return 1;
}

static void GenerateSounds()
{
	  // Fill in the wavetables
	  for (uint16_t i = 0; i < WAVETABLE_LENGTH; i++)
	  {
		  SquareWavetable[i] = (i < (WAVETABLE_LENGTH >> 1)) ? 0 : 0xFFF;
		  NoiseWavetable[i] = rand() % 4095;
	  }

//		soundGameStart = 0,
//		soundGameEnd,
//		soundShot,
//		soundExplosion,
//		soundDamage,
//		soundPickup,
//		soundPlayerExplosion,
//		soundPause

	  sfx[0].recipes = sfxGameStart;
	  sfx[0].recipeCount = 13;
	  sfx[1].recipes = sfxGameEnd;
	  sfx[1].recipeCount = 7;
	  sfx[2].recipes = sfxShot;
	  sfx[2].recipeCount = 3;
	  sfx[3].recipes = sfxExplosion;
	  sfx[3].recipeCount = 1;
	  sfx[4].recipes = sfxDamage;
	  sfx[4].recipeCount = 3;
	  sfx[5].recipes = sfxPickup;
	  sfx[5].recipeCount = 5;
	  sfx[6].recipes = sfxPlayerExplosion;
	  sfx[6].recipeCount = 7;
	  sfx[7].recipes = sfxPause;
	  sfx[7].recipeCount = 7;
}



void SoundCallback()
{
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);

	uint16_t outputValue = 0;
	if (osc.active) {

		osc.increment = osc.frequency * WAVETABLE_LENGTH / SAMPLE_RATE;

		int index = (int)osc.phase;
		outputValue = SquareWavetable[index];
		osc.phase += osc.increment;

		if (osc.phase >= WAVETABLE_LENGTH) {
			osc.phase -= WAVETABLE_LENGTH;
		}

		osc.samplesLeft -= 1;
		if (osc.samplesLeft == 0)
		{
			osc.recipeCurrent++;
			if (osc.recipeCurrent >= osc.sfx->recipeCount)
			{
				osc.active = 0;
			}
			else
			{
				osc.frequency = osc.sfx->recipes[osc.recipeCurrent].frequency;
				osc.samplesLeft = osc.sfx->recipes[osc.recipeCurrent].durationSec * SAMPLE_RATE;
			}
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

	if (osc.active)
		return;

	osc.sfx = &sfx[sound];

	osc.recipeCurrent = 0;

	osc.frequency = osc.sfx->recipes[osc.recipeCurrent].frequency;
	osc.samplesLeft = osc.sfx->recipes[osc.recipeCurrent].durationSec * SAMPLE_RATE;

	osc.active = 1;

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
