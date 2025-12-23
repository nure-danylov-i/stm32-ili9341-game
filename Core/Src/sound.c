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

const struct Recipe sfxGameStart[13] = {
		{ waveSaw, 200, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSaw, 400, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSaw, 500, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSaw, 200, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSaw, 400, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSaw, 500, 0.1f }, { waveSquare, 0, 0.025f },
		{ waveSaw, 800, 0.25f }
};

const struct Recipe sfxGameEnd[7] = {
		{ waveSquare, 500, 0.1f }, {waveSquare, 0, 0.025f },
		{ waveSquare, 500, 0.1f }, {waveSquare, 0, 0.025f },
		{ waveSquare, 500, 0.1f }, {waveSquare, 0, 0.025f },
		{ waveSaw, 300, 0.625f }
};

const struct Recipe sfxShot[3] = {
		{ waveSaw, 1000, 0.05f },
		{ waveSaw, 500, 0.05f },
		{ waveSaw, 250, 0.05f }
};

const struct Recipe sfxExplosion[3] = {
		{ waveNoise, 1000, 0.05f },
		{ waveNoise, 660, 0.075f },
		{ waveNoise, 220, 0.05f },
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
		{ waveNoise, 1000, 0.125f },
		{ waveNoise, 400, 0.125f },
		{ waveSquare, 200, 0.025f},
		{ waveNoise, 200, 0.025f},
		{ waveSquare, 200, 0.025f},
		{ waveNoise, 100, 0.025f},
		{ waveSquare, 100, 0.025f},
};

const struct Recipe sfxPause[7] = {
		{ waveSquare, 1000, 0.05f }, {waveSquare, 0, 0.05f },
		{ waveSquare, 800, 0.05f }, {waveSquare, 0, 0.05f },
		{ waveSquare, 1000, 0.05f }, {waveSquare, 0, 0.05f },
		{ waveSquare, 800, 0.05f },
};

struct SFX sfx[7] = {0};

uint16_t SquareWavetable[WAVETABLE_LENGTH];
uint16_t SawWavetable[WAVETABLE_LENGTH];

struct Oscilator osc[4] = {0};

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
	srand(1234);
    for (uint16_t i = 0; i < WAVETABLE_LENGTH; i++)
    {
    	SquareWavetable[i] = (i < (WAVETABLE_LENGTH >> 1)) ? 0 : 0xFFF;
    	SawWavetable[i] = (float)i / WAVETABLE_LENGTH * 4096;
    }

    // Assign recipe arrays to SFX structs
    sfx[0].recipes = sfxGameStart;
    sfx[0].recipeCount = 13;
    sfx[1].recipes = sfxGameEnd;
    sfx[1].recipeCount = 7;
    sfx[2].recipes = sfxShot;
    sfx[2].recipeCount = 3;
    sfx[3].recipes = sfxExplosion;
    sfx[3].recipeCount = 3;
    sfx[4].recipes = sfxDamage;
    sfx[4].recipeCount = 3;
    sfx[5].recipes = sfxPickup;
    sfx[5].recipeCount = 5;
    sfx[6].recipes = sfxPlayerExplosion;
    sfx[6].recipeCount = 7;
    sfx[7].recipes = sfxPause;
    sfx[7].recipeCount = 7;
}

static uint16_t GetNoiseSample()
{
	return rand() % 4096;
}

void SoundCallback()
{
	uint16_t outputValue = 0;
	//uint8_t activeCount = 0;
	for (uint8_t i = 0; i < 4; i++) {

		if (!osc[i].active) {
			continue;
		}
		//activeCount++;

		osc[i].increment = osc[i].frequency * WAVETABLE_LENGTH / SAMPLE_RATE;

		uint16_t prevIndex = (int)osc[i].phase;

		osc[i].phase += osc[i].increment;

		if (osc[i].phase >= WAVETABLE_LENGTH) {
			osc[i].phase -= WAVETABLE_LENGTH;
		}

		uint16_t index = (int)osc[i].phase;

		switch (osc[i].sfx->recipes[osc[i].recipeCurrent].waveType)
		{
		case waveSquare:
			outputValue += SquareWavetable[index];
			break;
		case waveNoise:
			if (prevIndex != index) {
				osc[i].cachedNoiseSample = GetNoiseSample();
			}
			outputValue = osc[i].cachedNoiseSample;
			break;
		case waveSaw:
			outputValue += SawWavetable[index];
		}

		osc[i].samplesLeft -= 1;
		if (osc[i].samplesLeft == 0)
		{
			osc[i].recipeCurrent++;
			if (osc[i].recipeCurrent >= osc[i].sfx->recipeCount)
			{
				osc[i].active = 0;
			}
			else
			{
				osc[i].frequency = osc[i].sfx->recipes[osc[i].recipeCurrent].frequency;
				osc[i].samplesLeft = osc[i].sfx->recipes[osc[i].recipeCurrent].durationSec * SAMPLE_RATE;
			}
		}
	}

	if (outputValue > 0xFFF) {
		outputValue = 0xFFF;
	}

	HAL_DAC_SetValue(config.hdac, config.channel, DAC_ALIGN_12B_R, outputValue);
}

void PlaySound(enum SoundType sound)
{
	if (config.init == 0)
	{
		return;
	}

	for (uint8_t i = 0; i < 4; i++) {

		if (osc[i].active)
			continue;

		osc[i].sfx = &sfx[sound];

		osc[i].recipeCurrent = 0;

		osc[i].frequency = osc[i].sfx->recipes[osc[i].recipeCurrent].frequency;
		osc[i].samplesLeft = osc[i].sfx->recipes[osc[i].recipeCurrent].durationSec * SAMPLE_RATE;

		osc[i].active = 1;
		break;
	}

	return;
}
