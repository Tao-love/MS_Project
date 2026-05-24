/*
 * music.c
 *
 *  Created on: May 24, 2026
 *      Author: ZhuanZ（无密码）
 */

#include "music.h"
#include "tim.h"

/* USER CODE BEGIN PD */
#define MUSIC_TIMER_TICK_HZ 1000000
#define MUSIC_BEAT_MS 500

/* 《只因你太美》前 15 秒（按蜂鸣器单音方式整理） */
		const Bate ZhiYinNiTaiMei[] = {
		    // 第一段：只因你太美，BABY
		    {P0, 0.25}, {L6, 0.25}, {L6, 0.5}, {M3, 0.5}, {M3, 0.5}, // 只因你太美
		    {L6, 1.0}, {P0, 1.75}, {L6, 0.25}, {L6, 1.0},            // BABY (带有切分节奏的延音)

		    // 只因你太美，BABY
		    {P0, 0.25}, {L6, 0.25}, {L6, 0.5}, {M3, 0.5}, {M3, 0.5},
		    {L6, 1.0}, {P0, 1.75}, {L6, 0.25}, {L6, 1.0},

		    // 第二段：只因你实在是太美，BABY
		    // 0 6 6 6 6 6 3 3 (只因你实在是太美 - 连续十六分音符)
		    {P0, 0.25}, {L6, 0.25}, {L6, 0.25}, {L6, 0.25}, {L6, 0.25}, {L6, 0.25}, {M3, 0.5}, {M3, 0.5},
		    {L6, 1.0}, {P0, 1.75}, {L6, 0.25}, {L6, 1.0},

		    // 第三段：只因你太美，BABY。迎面...
		    {P0, 0.25}, {L6, 0.25}, {L6, 0.5}, {M3, 0.5}, {M3, 0.5},
		    {L6, 1.0}, {P0, 1.75}, {L6, 0.25}, {L6, 1.0},
		    {P0, 1.0}, {M3, 0.5}, {M3, 0.5}                          // 迎面...

};

#define ZhiYinNiTaiMei_LEN (sizeof(ZhiYinNiTaiMei) / sizeof(ZhiYinNiTaiMei[0]))

static const Bate *currentSong = 0;
static uint16_t currentSongLen = 0;
static uint16_t currentNoteIndex = 0;
static uint32_t currentNoteEndTick = 0;
static uint8_t musicPlaying = 0;

static void Music_SetTone(uint16_t frequency);
static void Music_SilenceOutput(void);

static void Music_StartNote(uint16_t index)
{
    uint32_t delay_ms;

    Music_SetTone(currentSong[index].frequency);
    delay_ms = (uint32_t)(currentSong[index].period * (float)MUSIC_BEAT_MS + 0.5f);
    currentNoteEndTick = HAL_GetTick() + delay_ms;
}

static void Music_SilenceOutput(void)
{
    HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COUNTER(&htim16, 0);
}

static void Music_SetTone(uint16_t frequency)
{
    uint32_t arr;

    if (frequency == P0)
    {
        Music_SilenceOutput();
        return;
    }

    arr = MUSIC_TIMER_TICK_HZ / frequency;
    if (arr == 0)
    {
        Music_SilenceOutput();
        return;
    }

    arr -= 1;
    if (arr > 0xFFFF)
    {
        arr = 0xFFFF;
    }

    HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
    __HAL_TIM_SET_AUTORELOAD(&htim16, arr);
    __HAL_TIM_SET_COUNTER(&htim16, 0);
    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, (arr + 1) / 2);
}

void Music_Init(void)
{
    uint32_t pclk2 = HAL_RCC_GetPCLK2Freq();
    uint32_t prescaler;

    if (pclk2 < MUSIC_TIMER_TICK_HZ)
    {
        prescaler = 0;
    }
    else
    {
        prescaler = (pclk2 / MUSIC_TIMER_TICK_HZ) - 1;
    }

    __HAL_TIM_DISABLE(&htim16);
    __HAL_TIM_SET_PRESCALER(&htim16, prescaler);
    __HAL_TIM_SET_AUTORELOAD(&htim16, 0xFFFF);
    __HAL_TIM_SET_COUNTER(&htim16, 0);
    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 0);
    HAL_TIM_GenerateEvent(&htim16, TIM_EVENTSOURCE_UPDATE);

    Music_Stop();
}

uint8_t Music_IsPlaying(void)
{
    return musicPlaying;
}

void Music_Stop(void)
{
    Music_SilenceOutput();
    currentSong = 0;
    currentSongLen = 0;
    currentNoteIndex = 0;
    currentNoteEndTick = 0;
    musicPlaying = 0;
}

void Music_Update(void)
{
    if (musicPlaying == 0)
    {
        return;
    }

    if ((int32_t)(HAL_GetTick() - currentNoteEndTick) < 0)
    {
        return;
    }

    currentNoteIndex++;
    if (currentNoteIndex >= currentSongLen)
    {
        Music_Stop();
        return;
    }

    Music_StartNote(currentNoteIndex);
}

void Music_PlaySong(const Bate *song, uint16_t len)
{
    if (song == NULL || len == 0)
    {
        return;
    }

    currentSong = song;
    currentSongLen = len;
    currentNoteIndex = 0;
    musicPlaying = 1;
    Music_StartNote(currentNoteIndex);
}

void Music_PlayJiNiTaiMei(void)
{
    Music_PlaySong(ZhiYinNiTaiMei, ZhiYinNiTaiMei_LEN);
}
