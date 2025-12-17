/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "boing.h"
#include "ili9341_gfx.h"
#include "ili9341.h"

#include "bitmaps.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define SOUND_SAMPLES 1000
#define MAX_OBJECTS 32

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;
ADC_HandleTypeDef hadc4;

DAC_HandleTypeDef hdac1;
DMA_HandleTypeDef hdma_dac1_ch1;

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_tx;

TIM_HandleTypeDef htim6;
TIM_HandleTypeDef htim7;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
ili9341_t *lcd;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_SPI1_Init(void);
static void MX_TIM6_Init(void);
static void MX_TIM7_Init(void);
static void MX_DAC1_Init(void);
static void MX_ADC3_Init(void);
static void MX_ADC4_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

enum ObjectType {
		typePlayer, typeEnemy, typeCirclingEnemy, typeShootingEnemy,
		typeBullet, typeEnemyBullet, typeHeart, typeParticle, typeTest
};

enum SoundType
{
	soundGameStart, soundGameEnd, soundShot, soundExplosion, soundDamage, soundPickup, soundPlayerExplosion, soundPause
};

enum GameState
{
	gsSplashScreen, gsWait, gsPlaying, gsGameOver
};

struct Object {
	int16_t x;
	int16_t y;
	int16_t w;
	int16_t h;
	int16_t old_x;
	int16_t old_y;
	int8_t vx;
	int8_t vy;
	int8_t alive;
	int8_t state;
	enum ObjectType type;
};

enum GameState gameState;

const int8_t enemySpeeds[8] = {
		-2, -1, 0, 0, 1, 2, 0, 0
};

const int8_t enemyCircleSpeeds[25] = {
		0, 0,
		1, 1, 1,
		2, 2, 2,
		3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
		2, 2, 2,
		1, 1, 1,
		0,
};

const int8_t enemyCircleSpeedsY[50] = {
		3, 3, 3, 3, 3,
		2, 2, 2, 2,
		1, 1, 1,
		0, 0,
		-1, -1, -1,
		-2, -2, -2, -2,
		-3, -3, -3, -3, -3, -3, -3, -3, -3,
		-2, -2, -2, -2,
		-1, -1, -1,
		0, 0,
		1, 1, 1,
		2, 2, 2, 2,
		3, 3, 3, 3,
};

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

#define SOUND_SHOT_LENGTH 1200
#define SOUND_DAMAGE_LENGTH 1200
#define SOUND_PAUSE_LENGTH 2800
#define SOUND_PICKUP_LENGTH 4000
#define SOUND_EXPLOSION_LENGTH 3000
#define SOUND_GAMESTART_LENGTH 8000
#define SOUND_GAMEEND_LENGTH 8000

uint16_t soundWaveformShot[SOUND_SHOT_LENGTH];
uint16_t soundWaveformPause[SOUND_PAUSE_LENGTH];
uint16_t soundWaveformExplosion[SOUND_EXPLOSION_LENGTH];
uint16_t soundWaveformDamage[SOUND_DAMAGE_LENGTH];
uint16_t soundWavefromGameStart[SOUND_GAMESTART_LENGTH];
uint16_t soundWaveformGameEnd[SOUND_GAMEEND_LENGTH];
uint16_t soundWaveformPickup[SOUND_PICKUP_LENGTH];

enum SoundType previousSound = soundGameStart;

uint16_t touch_x = 0;
uint16_t touch_y = 0;
int8_t test_var = 0;
unsigned int score = 0;
unsigned int highscore = 0;
uint16_t difficulty = 0;
uint8_t pause = 0;
uint16_t gameOver = 0;
int8_t score_upd_flag = 0;
int8_t life_upd_flag = 0;

uint32_t nLehmer = 0;

ili9341_color_t color_bg = 0x0000;
ili9341_color_t color_bg_score = 0x0000;

struct Object *player = {0};

int8_t playerLife = 0;
const int8_t playerLifeMax = 3;
int8_t playerInvul = 0;
uint8_t wait = 0;

uint8_t shootingEnemyPresent = 0;

struct Object objects[MAX_OBJECTS] = {0};
int objectCount;

unsigned int frameCounter;

uint32_t joystick[2];

void UART_Printf(const char* fmt, ...);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
uint32_t RandLehmer();
void GenerateWaveform(const uint16_t *tune, uint16_t *waveform, unsigned int length);
void GenerateSounds();
void PlaySound(enum SoundType sound);
struct Object *CreateObject(int16_t x, int16_t y, int16_t w, int16_t h, int8_t vx, int8_t vy, enum ObjectType type);
void DeleteObject(struct Object *obj);
int16_t GetObjectScore(struct Object *obj);
void CreateExplosion(int16_t x, int16_t y);
void ClearObject(struct Object *obj, ili9341_color_t color);
void DrawObject(struct Object *obj);
void DrawScore();
void CheckCollisions(struct Object* obj);
void UpdateEnemy(struct Object* obj);
void UpdateCirclingEnemy(struct Object* obj);
void UpdateShootingEnemy(struct Object* obj);
void UpdateParticle(struct Object* obj);
void SpawnEnemy();
void InitGame();
void RestartGame();
void EndGame();
void UpdateGame();
void DrawButton(uint16_t y, char str[], uint8_t pressed);
uint8_t WaitForButton(uint16_t y);
void SplashScreen();
void DrawHighscore();

void UART_Printf(const char* fmt, ...)
{
    char buff[256];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buff, sizeof(buff), fmt, args);
    HAL_UART_Transmit(&huart2, (uint8_t*)buff, strlen(buff),
                      HAL_MAX_DELAY);
    va_end(args);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM6)
	{
		if (gameState == gsSplashScreen)
		{
			if (WaitForButton(220))
			{
				DrawButton(220, "Play", 1);
				gameState = gsWait;
			}
		}
		else if (gameState == gsWait)
		{
			wait++;
			if (wait >= 5)
			{
				wait = 0;
				InitGame();
				gameState = gsPlaying;
			}
		}
		else if (gameState == gsPlaying)
		{
			UpdateGame();
			if (gameOver >= 75)
			{
				EndGame();
				gameState = gsGameOver;
			}
		}
		else if (gameState == gsGameOver)
		{
			gameOver++;
			if (gameOver >= 750)
			{
				SplashScreen();
				gameState = gsSplashScreen;
			}
			if (WaitForButton(200))
			{
				DrawButton(200, "Play again", 1);
				gameState = gsWait;
			}
		}
		if (__HAL_TIM_GET_FLAG(htim, TIM_FLAG_UPDATE))
		{
			UART_Printf("Timer overrun - UpdateGame() too slow!\r\n");
		}
	}
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if (GPIO_Pin == GPIO_PIN_13)
	{
		if (gameOver)
			return;
		pause = !pause;
		if (pause)
		{
			HAL_TIM_Base_Stop_IT(&htim6);
			PlaySound(soundPause);
		}
		else
			HAL_TIM_Base_Start_IT(&htim6);

	}
	else
	{
		__NOP();
	}
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef* hdac)
{
	HAL_TIM_Base_Stop(&htim7);
}

void HAL_DAC_ErrorCallbackCh1(DAC_HandleTypeDef *hdac)
{
	UART_Printf("DAC error! Error: %d, DAC state: %d\r\n", hdac->ErrorCode, hdac->State);
}

void HAL_DAC_DMAUnderrunCallbackCh1(DAC_HandleTypeDef *hdac)
{
	UART_Printf("DAC underrun! Error: %d, DAC state: %d\r\n", hdac->ErrorCode, hdac->State);
}

uint32_t RandLehmer()
{
	const uint32_t A = 48271;

	uint32_t low  = (nLehmer & 0x7fff) * A;
	uint32_t high = (nLehmer >> 15)    * A;
	uint32_t x = low + ((high & 0xffff) << 15) + (high >> 16);

	x = (x & 0x7fffffff) + (x >> 31);
	return nLehmer = x;
}

void GenerateWaveform(const uint16_t *tune, uint16_t *waveform, unsigned int length)
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

void GenerateSounds()
{
	  for (int i = 0; i < SOUND_SAMPLES*3; i++)
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
	if (!(sound == soundShot || previousSound == soundPlayerExplosion))
	{
		HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);
		HAL_TIM_Base_Stop(&htim7);
	}

	switch(sound)
	{
	case soundShot:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformShot, SOUND_SHOT_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundExplosion:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformExplosion, SOUND_EXPLOSION_LENGTH / 3, DAC_ALIGN_12B_R);
		break;
	case soundPlayerExplosion:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformExplosion, SOUND_EXPLOSION_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundDamage:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformDamage, SOUND_DAMAGE_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundGameStart:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWavefromGameStart, SOUND_GAMESTART_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundGameEnd:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformGameEnd, SOUND_GAMEEND_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundPickup:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformPickup, SOUND_PICKUP_LENGTH, DAC_ALIGN_12B_R);
		break;
	case soundPause:
		HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t*)soundWaveformPause, SOUND_PAUSE_LENGTH, DAC_ALIGN_12B_R);
		break;
	}
	HAL_TIM_Base_Start(&htim7);
	previousSound = sound;
}

struct Object *CreateObject(int16_t x, int16_t y, int16_t w, int16_t h, int8_t vx, int8_t vy, enum ObjectType type)
{
	struct Object* obj = NULL;
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		if (!objects[i].alive)
		{
			obj = &objects[i];
			break;
		}
	}

	if (obj == NULL)
		return NULL;

	obj->alive = 1;
	obj->x = x;
	obj->y = y;
	obj->w = w;
	obj->h = h;
	obj->vy = vy;
	obj->vx = vx;
	obj->old_x = x;
	obj->old_y = y;
	obj->type = type;
	obj->state = 0;

	if (type == typeParticle)
	{
		obj->state = 8;
	}

	objectCount++;

	return obj;
}

void DeleteObject(struct Object *obj)
{
	ClearObject(obj, color_bg);

	int16_t  _x = obj->x, _y = obj->y;
	uint16_t _w = obj->w, _h = obj->h;
	if (ili9341_my_clip_rect(lcd, &_x, &_y, &_w, &_h) == ibTrue)
	{
		ili9341_fill_rect(lcd, color_bg, _x, _y, _w, _h);
	}


	if (obj->type == typeShootingEnemy)
		shootingEnemyPresent = 0;
	obj->alive = 0;
	objectCount--;
}

int16_t GetObjectScore(struct Object *obj)
{
	switch (obj->type)
	{
	case typeEnemy:
		return 100;
	case typeCirclingEnemy:
		return 150;
	case typeShootingEnemy:
		return 300;
	default:
		return 0;
	}
}

void CreateExplosion(int16_t x, int16_t y)
{
	CreateObject(x, y, 4, 4, 4, 4, typeParticle);
	CreateObject(x, y, 4, 4, -4, 4, typeParticle);
	CreateObject(x, y, 4, 4, -4, -4, typeParticle);
	CreateObject(x, y, 4, 4, 4, -4, typeParticle);
	CreateObject(x, y, 4, 4, 0, 5, typeParticle);
	CreateObject(x, y, 4, 4, 0, -5, typeParticle);
}


void ClearObject(struct Object *obj, ili9341_color_t color)
{
	int16_t regX, regW, regY, regH;
	int8_t skipX = 0;

	if (obj->x < obj->old_x)
	{
	    regX = obj->x + obj->w;
	    regW = obj->old_x - obj->x;
	}
	else if (obj->x > obj->old_x)
	{
	    regX = obj->old_x;
	    regW = obj->x - obj->old_x;
	}
	else
	    skipX = 1;

	if (regW > obj->w)
	{
	    regX = obj->old_x;
	    regW = obj->w;
	}


	int8_t skipY = 0;

	if (obj->y < obj->old_y)
	{
	    regY = obj->y + obj->h;
	    regH = obj->old_y - obj->y;
	}
	else if (obj->y > obj->old_y)
	{
	    regY = obj->old_y;
	    regH = obj->y - obj->old_y;
	}
	else
	    skipY = 1;

	if (regH > obj->h)
	{
	    regY = obj->old_y;
	    regH = obj->h;
	}

	if ((!skipX))
		ili9341_fill_rect(lcd, color, regX, (obj->y < 20 ? 20 : obj->y), regW, obj->h);
	if ((!skipY))
		ili9341_fill_rect(lcd, color, skipX ? obj->x : obj->old_x, (regY < 20 ? 20 : regY), obj->w, regH);
}

void DrawObject(struct Object *obj)
{
	switch (obj->type)
	{
	case typePlayer:

		if (playerLife <= 0)
		{
			return;
		}

		const uint8_t *bmp;
		switch (obj->state) {
		case 1:
			bmp = bitmap_ship_left;
			break;
		case 2:
			bmp = bitmap_ship_right;
			break;
		default:
			bmp = bitmap_ship_idle;
			break;
		}
		if ((playerInvul & 0x01)) {
			ili9341_my_draw_bmp_2b(lcd, ILI9341_RED, color_bg, ILI9341_RED, ILI9341_BLACK,
					obj->x, obj->y, obj->w, obj->h, bmp, 0);
		}
		else {
			ili9341_my_draw_bmp_2b(lcd, ILI9341_WHITE, color_bg, ILI9341_LIGHTGREY, ILI9341_DARKGREY,
								obj->x, obj->y, obj->w, obj->h, bmp, 0);
		}
		break;
	case typeEnemy:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_MAGENTA, color_bg, ILI9341_DARKGREY, ILI9341_DARKGREY, obj->x, obj->y, obj->w, obj->h, bitmap_enemy, 0);
		break;
	case typeCirclingEnemy:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_ORANGE, color_bg, ILI9341_DARKGREY, ILI9341_DARKGREY, obj->x, obj->y, obj->w, obj->h, bitmap_enemy_circle, 0);
		break;
	case typeShootingEnemy:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_RED, color_bg, ILI9341_DARKGREY, ILI9341_DARKGREY, obj->x, obj->y, obj->w, obj->h, bitmap_enemy_fire, 0);
		break;
	case typeTest:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_WHITE, color_bg, ILI9341_YELLOW, ILI9341_BLACK, obj->x, obj->y, obj->w, obj->h, bitmap_smile, 0);
		break;
	case typeHeart:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_RED, color_bg, ILI9341_WHITE, ILI9341_WHITE, obj->x, obj->y, obj->w, obj->h, bitmap_heart, 0);
		break;
	case typeBullet:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_BLUE, color_bg, ILI9341_CYAN, ILI9341_CYAN, obj->x, obj->y, obj->w, obj->h, bitmap_bullet, 0);
		break;
	case typeEnemyBullet:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_RED, color_bg, ILI9341_PINK, ILI9341_PINK, obj->x, obj->y, obj->w, obj->h, bitmap_bullet, 0);
		break;
	case typeParticle:
		ili9341_my_draw_bmp_2b(lcd, ILI9341_YELLOW, color_bg, ILI9341_WHITE, ILI9341_WHITE, obj->x, obj->y, obj->w, obj->h, bitmap_particle, 0);
		break;
	default:
		ili9341_fill_rect(lcd, ILI9341_YELLOW, obj->x, obj->y, obj->w, obj->h);
		break;

	}
}

void DrawScore()
{
	if (score_upd_flag)
	{
		ili9341_text_attr_t text_attr = {0};
		text_attr.bg_color = color_bg_score;
		text_attr.fg_color = score_upd_flag == 2 ? ILI9341_YELLOW : ILI9341_WHITE;
		text_attr.font = &ili9341_font_11x18;
		text_attr.origin_y = 1;
		text_attr.origin_x = 4;

		char str[20] = {0};
		sprintf(str, "score: %07u", score);
		ili9341_draw_string(lcd, text_attr, str);
		score_upd_flag--;
	}

	if (life_upd_flag)
	{
		for (int8_t i = 0; i < playerLifeMax; i++)
		{
			ili9341_my_draw_bmp_2b(lcd, (playerLife < (i+1)) ? (life_upd_flag == 2 ? ILI9341_YELLOW : ILI9341_BLACK) : ILI9341_RED, color_bg_score, ILI9341_WHITE, ILI9341_WHITE,
					lcd->screen_size.width - 4 - (i+1)*16, 2, 16, 16, bitmap_heart, 1);
		}
		life_upd_flag--;
	}
}



void CheckCollisions(struct Object* obj)
{
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
		struct Object *obj2 = &objects[i];
		if (!obj2->alive) continue;
		if (obj2->type == typeBullet ||
				obj2->type == typePlayer || obj2->type == typeParticle) continue;
		if (!(obj2->x > obj->x + obj->w ||
				obj2->x + obj2->w < obj->x ||
				obj2->y > obj->y + obj->h ||
				obj2->y + obj2->h < obj->y))
		{
			if (obj->type == typeBullet)
			{
				if (obj2->type == typeEnemyBullet)
					continue;

				CreateExplosion(obj->x, obj->y);
				score += GetObjectScore(obj2);
				DeleteObject(obj2);
				DeleteObject(obj);
				score_upd_flag = 2;
				PlaySound(soundExplosion);
			}
			else if (obj->type == typePlayer && playerLife > 0)
			{
				if (obj2->type == typeEnemyBullet)
					DeleteObject(obj2);

				if (obj2->type == typeHeart)
				{
					playerLife++;
					DeleteObject(obj2);
					if (playerLife > playerLifeMax)
						playerLife = playerLifeMax;
					life_upd_flag = 2;
					PlaySound(soundPickup);
					break;
				}

				playerLife--;
				if (playerLife < 1)
				{
					playerLife = 0;
					CreateExplosion(obj->x + obj->w / 2, obj->y + obj->h);
					CreateExplosion(obj->x, obj->y);
					PlaySound(soundPlayerExplosion);
					DeleteObject(player);
				}
				else
				{
					playerInvul = 24;
					PlaySound(soundDamage);
				}
				life_upd_flag = 2;
			}

			break;
		}
	}
}

void UpdateEnemy(struct Object* obj)
{
	  obj->vx = enemySpeeds[((frameCounter) / 10) % 8];
}

void UpdateCirclingEnemy(struct Object* obj)
{
	  int32_t time = frameCounter / 2;

	  if (obj->state == 0)
	  {
		  if (obj->y >= 50)
			  obj->state = 1;
		  return;
	  }

	  if (obj->x > 110)
	  {
		  time -= 25;
	  }

	  if (time % 50 < 25)
		  obj->vx = -enemyCircleSpeeds[time % 25];
	  else
		  obj->vx = enemyCircleSpeeds[time % 25];

	  if (obj->x > 110)
		  obj->vx = -obj->vx;

	  obj->vy = enemyCircleSpeedsY[time % 50] + 1;
}

void UpdateShootingEnemy(struct Object* obj)
{
	if (playerLife <= 0)
	{
		obj->vy = 3;
		return;
	}

	  if (obj->state == 0)
	  {
		  int16_t minY = player->y - 180;
		  if (obj->y > (minY < 60 ? 60 : minY))
		  {
			  obj->vy = 0;
		  	  obj->state = 1;
		  }
	  }

	  else if (obj->state == 1)
	  {
		  if (obj->x < player->x - 3)
		  {
			  obj->vx = 3;
		  }
		  else if (obj-> x >= player->x + player->w + 3 - obj->w)
		  {
			  obj->vx = -3;
		  }
		  else
		  {
			  obj->vx = 0;
			  obj->state = 2;
		  }
	  }
	  else if (obj->state == 2)
	  {
		  if (frameCounter % 43 == 0) {
			  CreateObject(obj->x + (obj->w / 2) - 2, obj->y + obj->h + 8, 4, 8, 0, 6, typeEnemyBullet);
			  obj->state = 1;
		  }
	  }
}

void UpdateParticle(struct Object* obj)
{
	  obj->state--;
	  if (obj->state <= 0)
		  DeleteObject(obj);
}

void SpawnEnemy()
{
	static uint8_t side = 0;

	if (difficulty > 15 && difficulty % 25 == 0)
	{
		if (shootingEnemyPresent == 0)
		{
			CreateObject(RandLehmer() % 216, 0, 20, 16, 0, 3, typeShootingEnemy);
		}
		shootingEnemyPresent = 1;
	}
	else if (difficulty > 5 && difficulty % 3 == 0)
	{
		if (side == 0)
			CreateObject(25, 10, 20, 16, 0, 3, typeCirclingEnemy);
		else
			CreateObject(205, 10, 20, 16, 0, 3, typeCirclingEnemy);
		side = !side;
	}
	else
		CreateObject(RandLehmer() % 216, 0, 20, 16, 0, 3, typeEnemy);
}

unsigned int DifficultyToTiming(uint16_t d)
{
	if (d > 30)
		return 15;
	if (d > 15)
		return 25;
	else
		return 50;
}

void InitGame()
{
	UART_Printf("Game initialized!\r\n");

	// Мигнути два рази
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_Delay(25);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_Delay(25);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_Delay(25);
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_RESET);

	score = 0;
	difficulty = 0;
	gameOver = 0;
	frameCounter = 0;
	objectCount = 0;
	nLehmer = 21-3;
	player = CreateObject(96, 280, 48, 24, 0, 0, typePlayer);
	playerLife = playerLifeMax;

	ili9341_fill_screen(lcd, color_bg);
	ili9341_fill_rect(lcd, color_bg_score, 0, 0, lcd->screen_size.width, 20);
	ili9341_draw_line(lcd, ILI9341_WHITE, 0, 19, lcd->screen_size.width, 19);

	score_upd_flag = 1;
	life_upd_flag = 1;

	HAL_Delay(100);

	PlaySound(soundGameStart);
}

void RestartGame()
{
	UART_Printf("Restarting game...\r\n");
	InitGame();
	HAL_TIM_Base_Start_IT(&htim6);

}

void EndGame()
{
	UART_Printf("Game over! Score: %07u\r\n", score);

	uint8_t newHigh = 0;
	if (score > highscore)
	{
		newHigh = 1;
		highscore = score;
	}

	for (int i = 0; i < MAX_OBJECTS; i++)
		if (objects[i].alive)
			DeleteObject(&objects[i]);

	ili9341_fill_screen(lcd, ILI9341_BLACK);

	ili9341_text_attr_t text_attr = {0};
	text_attr.bg_color = color_bg_score;
	text_attr.fg_color = newHigh ? ILI9341_YELLOW : ILI9341_WHITE;
	text_attr.font = /* newHigh ? &ili9341_font_11x18 : */ &ili9341_font_16x26;
	text_attr.origin_y = 120;
	text_attr.origin_x = newHigh ? 8 : 40;

	char str[20] = {0};
	ili9341_draw_string(lcd, text_attr, newHigh ? "NEW HIGHSCORE!" : "GAME OVER!");

	text_attr.fg_color = ILI9341_WHITE;
	text_attr.font = &ili9341_font_11x18;
	text_attr.origin_y = 150;
	text_attr.origin_x = 43;
	sprintf(str, "score: %07u", score);
	ili9341_draw_string(lcd, text_attr, str);

	DrawButton(200, "Play again", 0);

	if (newHigh == 0)
	{
		DrawHighscore();
	}

	if (newHigh == 0)
	{
		PlaySound(soundGameEnd);
	}
	else
	{
		PlaySound(soundPickup);
	}

}

void UpdateGame()
{
	HAL_ADC_Start(&hadc3);
	HAL_ADC_PollForConversion(&hadc3, 10);
	joystick[0] = HAL_ADC_GetValue(&hadc3);
	HAL_ADC_Stop(&hadc3);

	HAL_ADC_Start(&hadc4);
	HAL_ADC_PollForConversion(&hadc4, 10);
	joystick[1] = HAL_ADC_GetValue(&hadc4);
	HAL_ADC_Stop(&hadc4);

	int16_t spd_joy_y = joystick[1];
	int16_t spd_joy_x = joystick[0];

	spd_joy_x -= 32;
	spd_joy_y -= 32;
	spd_joy_x /= 10;
	spd_joy_y /= -10;

	ili9341_touch_pressed_t touchPressed = ili9341_touch_coordinate(lcd, &touch_x, &touch_y);

	// Обробка введення та швидкості гравця
	if (playerLife > 0 && ((touchPressed == itpPressed) || spd_joy_x != 0 || spd_joy_y != 0))
	{
		int16_t spd_x;
		int16_t spd_y;
		if (touchPressed == itpPressed)
		{
			touch_y = 320 - touch_y;
			touch_x -= 48;
			touch_x -= 12;

			spd_x = touch_x - player->x;
			spd_y = touch_y - player->y;

			spd_x /= (1 + abs(spd_x) / 4);
			spd_y /= (1 + abs(spd_y) / 4);
		}
		else
		{
			spd_x = spd_joy_x;
			spd_y = spd_joy_y;
		}

	  UART_Printf("spd_x: %d, spd_y: %d\r\n", spd_x, spd_y);

	  player->vx = spd_x;
	  player->vy = spd_y;

	  if (spd_x > 0) player->state = 2;
	  else if (spd_x < 0) player->state = 1;

	  // Стрільба
	  if (frameCounter % 13 == 0 && !playerInvul)
	  {
		  CreateObject(player->x + (player->w / 2) - 2, player->y - 8, 4, 8, 0, -8, typeBullet);
		  PlaySound(soundShot);
	  }

	}
	else
	{
	  player->state = 0;
	  player->vx = 0;
	  player->vy = 0;
	}

	// Оновлення об'єктів
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
	  struct Object *obj = &objects[i];

	  if (!obj->alive)
		  continue;

	  if ((obj->y >= 320 || obj->y < 0) && obj->type != typePlayer)
	  {
		  DeleteObject(obj);
		  continue;
	  }

	  // Оновлення ворогів
	  if (obj->type == typeEnemy)
		  UpdateEnemy(obj);

	  if (obj->type == typeCirclingEnemy)
		  UpdateCirclingEnemy(obj);

	  if (obj->type == typeShootingEnemy)
		  UpdateShootingEnemy(obj);

	  // Оновлення положень об'єкітв
	  obj->old_x = obj->x;
	  obj->old_y = obj->y;

	  obj->y += obj->vy;
	  obj->x += obj->vx;

	  // Оновлення частинок
	  if (obj->type == typeParticle)
		  UpdateParticle(obj);
	}

	// Створення об'єктів
	if (frameCounter > 15 && frameCounter % DifficultyToTiming(difficulty) == 0)
	{
		SpawnEnemy();
		difficulty++;

		if (difficulty > 16000)
			difficulty = 16000;
	}

	if (playerLife < playerLifeMax && frameCounter % 1600 == 0)
	{
		CreateObject(RandLehmer() % 216, 0, 16, 16, 0, 2, typeHeart);
	}

	if (player->y > lcd->screen_size.height - player->h)
		player->y = lcd->screen_size.height - player->h;

	if (player->y < 21)
		player->y = 21;

	for (int i = 0; i < MAX_OBJECTS; i++)
	{
	  struct Object *obj = &objects[i];

	  if (!obj->alive)
		  continue;

	  if (obj->x < 0)
		  obj->x = 0;
	  if (obj->x >= lcd->screen_size.width - obj->w)
		  obj->x = lcd->screen_size.width - obj->w - 1;

	  // Перевірка зіткнень
	  if (obj->type == typeBullet || (obj->type == typePlayer && !playerInvul))
		  CheckCollisions(obj);

	  // Вибіркове очищення
	  ClearObject(obj, color_bg);
	}

	// Виведення об'єктів на дисплей
	for (int i = 0; i < MAX_OBJECTS; i++)
	{
	  struct Object *obj = &objects[i];

	  if (obj->alive)
		  DrawObject(obj);
	}

	if (score_upd_flag || life_upd_flag)
	{
	  DrawScore();
	}

	// Оновлення змінних
	if (playerInvul > 0)
	  playerInvul--;

	if (playerLife <= 0)
	{
		gameOver++;
		if(gameOver >= 75)
		{
			gameOver = 75;
		}

	}

	frameCounter++;
}

void DrawButton(uint16_t y, char str[], uint8_t pressed)
{
	ili9341_text_attr_t text_attr = {0};
	text_attr.bg_color = pressed ? ILI9341_GREEN : color_bg_score;
	text_attr.fg_color = ILI9341_WHITE;
	text_attr.font = &ili9341_font_11x18;
	text_attr.origin_y = y + 10;
	text_attr.origin_x = 120 - (strlen(str) * 11) / 2;

	if (pressed)
	{
		ili9341_fill_rect(lcd, ILI9341_GREEN, 55, y, 130, 38);
	}

	ili9341_draw_rect(lcd, ILI9341_WHITE, 55, y, 130, 38);
	ili9341_draw_string(lcd, text_attr, str);
}

uint8_t WaitForButton(uint16_t y)
{
	if (ili9341_touch_coordinate(lcd, &touch_x, &touch_y) == itpPressed)
	{
		touch_y = 320 - touch_y;
		touch_x -= 48;
		if (touch_x > 55 && touch_x < 185 && touch_y > y && touch_y < y + 38)
		{
			return 1;
		}
	}

	return 0;
}

void DrawHighscore()
{
	char str[20] = {0};

	ili9341_text_attr_t text_attr = {0};
	text_attr.bg_color = color_bg_score;
	text_attr.fg_color = ILI9341_DARKGREY;
	text_attr.font = &ili9341_font_11x18;
	sprintf(str, "highscore: %07u", highscore);
	text_attr.origin_y = 290;
	text_attr.origin_x = 120 - (strlen(str) * 11) / 2;
	ili9341_draw_string(lcd, text_attr, str);
}

void SplashScreen()
{
	ili9341_fill_screen(lcd, ILI9341_BLACK);

		ili9341_text_attr_t text_attr = {0};
		text_attr.bg_color = color_bg_score;
		text_attr.fg_color = ILI9341_WHITE;
		text_attr.font = &ili9341_font_11x18;
		text_attr.origin_y = 54;
		text_attr.origin_x = 15;
		ili9341_draw_string(lcd, text_attr, "STM32 SPACE SHOOTER");

		ili9341_my_draw_bmp_2b(lcd, ILI9341_WHITE, color_bg, ILI9341_LIGHTGREY, ILI9341_DARKGREY,
							56, 108, 128, 80, bitmap_logo, 0);

		DrawButton(220, "Play", 0);

		if (highscore != 0)
		{
			DrawHighscore();
		}

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USART2_UART_Init();
  MX_SPI1_Init();
  MX_TIM6_Init();
  MX_TIM7_Init();
  MX_DAC1_Init();
  MX_ADC3_Init();
  MX_ADC4_Init();
  /* USER CODE BEGIN 2 */

  lcd = ili9341_new(
      &hspi1,
      GPIOA, GPIO_PIN_9,
      GPIOB, GPIO_PIN_6,
	  GPIOC, GPIO_PIN_7,
      isoPortrait,
      GPIOA,  GPIO_PIN_8,
	  GPIOB, GPIO_PIN_10,
      itsSupported,
      itnNormalized);

  ili9341_calibrate_scalar(lcd, 400, 409, 3875, 3763);

  GenerateSounds();

  __HAL_TIM_CLEAR_FLAG(&htim6, TIM_SR_UIF);
  __HAL_TIM_CLEAR_FLAG(&htim7, TIM_SR_UIF);

  	gameState = gsSplashScreen;

	SplashScreen();

	//InitGame();
	HAL_TIM_Base_Start_IT(&htim6);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2|RCC_PERIPHCLK_ADC34;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  PeriphClkInit.Adc34ClockSelection = RCC_ADC34PLLCLK_DIV1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_MultiModeTypeDef multimode = {0};
  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc3.Init.Resolution = ADC_RESOLUTION_6B;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DMAContinuousRequests = DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the ADC multi-mode
  */
  multimode.Mode = ADC_MODE_INDEPENDENT;
  if (HAL_ADCEx_MultiModeConfigChannel(&hadc3, &multimode) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_5;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief ADC4 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC4_Init(void)
{

  /* USER CODE BEGIN ADC4_Init 0 */

  /* USER CODE END ADC4_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC4_Init 1 */

  /* USER CODE END ADC4_Init 1 */

  /** Common config
  */
  hadc4.Instance = ADC4;
  hadc4.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV1;
  hadc4.Init.Resolution = ADC_RESOLUTION_6B;
  hadc4.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc4.Init.ContinuousConvMode = DISABLE;
  hadc4.Init.DiscontinuousConvMode = DISABLE;
  hadc4.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc4.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc4.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc4.Init.NbrOfConversion = 1;
  hadc4.Init.DMAContinuousRequests = DISABLE;
  hadc4.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc4.Init.LowPowerAutoWait = DISABLE;
  hadc4.Init.Overrun = ADC_OVR_DATA_OVERWRITTEN;
  if (HAL_ADC_Init(&hadc4) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_4;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc4, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC4_Init 2 */

  /* USER CODE END ADC4_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_Trigger = DAC_TRIGGER_T7_TRGO;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 7;
  hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief TIM6 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM6_Init(void)
{

  /* USER CODE BEGIN TIM6_Init 0 */

  /* USER CODE END TIM6_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM6_Init 1 */

  /* USER CODE END TIM6_Init 1 */
  htim6.Instance = TIM6;
  htim6.Init.Prescaler = 35999;
  htim6.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim6.Init.Period = 79;
  htim6.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim6) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim6, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM6_Init 2 */

  /* USER CODE END TIM6_Init 2 */

}

/**
  * @brief TIM7 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM7_Init(void)
{

  /* USER CODE BEGIN TIM7_Init 0 */

  /* USER CODE END TIM7_Init 0 */

  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM7_Init 1 */

  /* USER CODE END TIM7_Init 1 */
  htim7.Instance = TIM7;
  htim7.Init.Prescaler = 71;
  htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim7.Init.Period = 124;
  htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM7_Init 2 */

  /* USER CODE END TIM7_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 38400;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();
  __HAL_RCC_DMA2_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
  /* DMA2_Channel3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA2_Channel3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA2_Channel3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : PB10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA8 PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 1);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_4, GPIO_PIN_SET);
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
