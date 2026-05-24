/*
 * music.h
 *
 *  Created on: May 24, 2026
 *      Author: ZhuanZ（无密码）
 */

#ifndef INC_MUSIC_H_
#define INC_MUSIC_H_

#include <stdint.h>

typedef struct
{
    uint16_t frequency; // 音符频率
    float period;       // 音符持续时间，单位为拍
} Bate;

/* 乐谱对应 */
#define P0 0 // 休止符频率

#define L1 262  // 低音频率
#define L2 294
#define L3 330
#define L4 349
#define L5 392
#define L6 440
#define L7 494

#define M1 523  // 中音频率
#define M2 587
#define M3 659
#define M4 698
#define M5 784
#define M6 880
#define M7 988

#define H1 1047 // 高音频率
#define H2 1175
#define H3 1319
#define H4 1397
#define H5 1568
#define H6 1760
#define H7 1976

extern const Bate ZhiYinNiTaiMei[];
void Music_Init(void);
uint8_t Music_IsPlaying(void);
void Music_Stop(void);
void Music_Update(void);
void Music_PlaySong(const Bate *song, uint16_t len);
void Music_PlayJiNiTaiMei(void);

#endif /* INC_MUSIC_H_ */
