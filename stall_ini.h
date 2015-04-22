/* here all defines in game. can change it quick*/

#ifndef STALL_INI_H
#define STALL_INI_H


#define ERROR_DBG
#define DEBUG

#ifdef ERROR_DBG
 #define error(x...) printf("[error] " x)
#else
 #define error(x...)
#endif

#ifdef DEBUG
 #define debug(x...) printf(x)
#else
 #define debug(x...)
#endif

/* game features */
//#define GOD_MODE
#define SHOW_FPS

/* global game options */
#define WAVE_NUM 15
#define WAVE_PHASES 6
#define TIME_WAVE_END 2000
#define BONUS_NUM 7

/* window */
#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define GAME_WIDTH WINDOW_WIDTH 
#define GAME_HEIGHT (WINDOW_HEIGHT - 20)

/* interface */
#define SHIELD_BAR_X WINDOW_WIDTH/2 - 15
#define SHIELD_BAR_W 100
#define TIME_BAR_X WINDOW_WIDTH - 120
#define TIME_BAR_W 100

/* bonus options */
#define BONUS_WIDTH  40
#define BONUS_HEIGHT 40
#define BONUS_DEFSHIELD_SIZE 5
#define BONUS_DEFSHIELD_BULLET    25
#define BONUS_DEFSHIELD_DEATHPART 50
#define BONUS_DEFSHIELD_ENEMY     100
#define BONUS_DEFSHIELD_BLINKING_TIME 5
#define BONUS_DEFSHIELD_BLINK_FREQ 300
#define BONUS_RANDOMBOMB_ENEMIES 5 /* explode 5 enemies */
#define BONUS_AIM_ERRVALUE 10000   /* used as max for pts and if no target found */


/* time for showing labels */
#define TIME_SHOW_WAVE_LABEL 3000
#define TIME_SHOW_SCORE 5000

/* enemy */
//#define ENEMY_WIDTH  52
//#define ENEMY_HEIGHT 53
#define MAX_ENEMIES 100
#define ENEMY_FREQ 100  /* in ms */
#define ENEMY_ANIM_FRAMES 4
#define ENEMY_ANIM_SPEED 6

/* dead parts v2.0 */
#define DEAD_PARTS_MAX_NUM 4
#define TOTAL_DEAD_PARTS MAX_ENEMIES * DEAD_PARTS_MAX_NUM

/* enemy bullets */
#define ENEMY_BULLETS  100
#define ENEMY_BULLET_WIDTH 4
#define ENEMY_BULLET_HEIGHT 9

#define ENEMY_BULLET_COLOR 0xFF0000FF
#define ENEMY_BULLET_SPEED 4

#define BULLET_ANIM_FRAMES 2
#define BULLET_ANIM_SPEED 2

/* player bullets */
#define PLAYER_BULLET_SPEED 7
#define PLAYER_BULLETS 50
#define PLAYER_BULLET_WIDTH 2
#define PLAYER_BULLET_HEIGHT 5
#define PLAYER_BULLET_COLOR 0xFF0000FF

/* ram bonus anim */
#define RAM_ANIM_FRAMES 3
#define RAM_ANIM_SPEED 5

/* player */
#define PLAYER_WIDTH  41
#define PLAYER_HEIGHT 45
#define PLAYER_SPEED 5
#define PLAYER_ACC_SPEED 8

/* points */
#define ENEMY_COST 25
#define DEAD_PART_COST 10
#define BULLET_COST 1



#define ENEMY_COLOR 0x00FF00FF
#define BG_COLOR 0x000000FF

/* fps */
#define FPS_RATE_ALLOWED 60 /* so 16.66 ms per frame */
#define TIME_PER_FRAME (1000 / FPS_RATE_ALLOWED)


#endif
