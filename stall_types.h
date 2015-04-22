#ifndef STALL_TYPES_H
#define STALL_TYPES_H

#include "stall_ini.h"

typedef unsigned char u8;


enum keys_t
{
	KEY_UP,
	KEY_DOWN,
	KEY_LEFT,
	KEY_RIGHT,
	KEY_MAX
};

enum state_t
{
	STATE_MENU,
	STATE_WAVESTART,  /* show label "wave 1" */
	STATE_WAVEEND,     /* when enemies blows */
	STATE_WAVECLEARED, /* show label "wave cleared" */
	STATE_SCORE,      /* show label "your score xxxx" */
	STATE_MAIN
};

/* our menu before game starts */
enum focus_t
{
	FOCUS_START,
	FOCUS_OPTIONS,
	FOCUS_EXIT,
	FOCUS_MAX
};

enum bonus_num_t
{
	BONUS_ARMOR_BREAKER,
	BONUS_TRIPLE_SHOT,
	BONUS_DEF_SHIELD,
	BONUS_RANDOM_BOMB,
	BONUS_RAM,
	BONUS_AUTO_AIM_MISSILES,
	BONUS_ACCELERATION
};

/* mental types of enemy */
enum mental_t
{
	MENTAL_AGGRESSIVE, /* close combat, kamikaze etc */
	MENTAL_MIDDLE,     /* just middle, prefer strafe */
	MENTAL_SLOW,       /* slow, short path, dont like to move */
	MENTAL_COWARD,     /* paranoia, get far from player */
	MENTAL_MAX
};

enum move_t
{
	MOVE_DONT,
	MOVE_RIGHT,
	MOVE_LEFT,
	MOVE_UP,
	MOVE_DOWN
};

enum bullet_type_t
{
	BULLET_TYPE_CENTER,
	BULLET_TYPE_RIGHT,
	BULLET_TYPE_LEFT,
	BULLET_TYPE_MISSILE
};

typedef struct rect_s
{
	int x1;
	int y1;
	int x2;
	int y2;
} rect_t;

typedef struct dead_parts_s
{
	int alive;
	int speed;
	rect_t crd; 
	SDL_Rect pic; /* part of enemy's ship sprite */
	int way[2];   /* way to move 0 - x, 1 - y */
} dead_parts_t;

typedef struct player_ship_s
{
	rect_t crd; 
	unsigned int speed;
	unsigned int bullet_speed;
	unsigned int alive;
	u8 ram;                    /* can ram enemies and stay alive */
	unsigned int dead_frame; //dead animation

} player_ship_t;

typedef struct enemy_ship_s
{
	rect_t crd; 
	unsigned int alive;
	unsigned int speed;
	unsigned int bullet_speed;
	unsigned int time_anim; 	
	unsigned int anim;
	enum mental_t mental; 
	enum move_t move;  
	unsigned int in_action; /* already has job to do */
	int goal_x; /* coords to the goal in x1 */
	int goal_y;

} enemy_ship_t;

typedef struct bullet_s
{
	rect_t crd; 
	int ship_id;
	unsigned int time_anim; /* time used for animation */
	unsigned int anim;      /* num of animation frame */
	unsigned int exist;
	enum bullet_type_t btype; /* type of bullet - center, right, left, missile */
	int target_id; /* if missile - we calc path to target every bullet move */
} bullet_t;


typedef struct bonus_defshield_s
{
	rect_t crd; /* based on player's coordinates */
	int energy; /* max 100, min 0 */
	u8 blinking; /* in blinking state - between the disappear */
	u8 show; /* 1 when show */
	/* TODO - add array of animation frames here */
	SDL_Surface *img;
	

} bonus_defshield_t;

typedef struct bonus_s
{
	rect_t crd; 
	int scr_duration;  /* on screen duration */
	int plr_duration;  /* on player duration */
	int scr_lifetime;  /* time remaining on screen in seconds */
	int plr_lifetime;  /* time remaining on player in seconds */
	char *name;
	u8 active;     /* present on the screen */
	u8 on_player;  /* currently player has it */
	unsigned int chance[WAVE_NUM]; /* chance for every wave in percents for sec*/
} bonus_t;

typedef struct wave_s
{
	unsigned int enemy_speed;
	unsigned int enemy_width;
	unsigned int enemy_height;
	unsigned int dead_parts_num;
	unsigned int phases[WAVE_PHASES][2]; /* [0] -duration, [1] - enemyfreq */
	unsigned int duration;    /* duration in seconds */
	unsigned int start_time;    /* start_time in SDL_GetTicks() */
	unsigned int player_bullets_color;
	SDL_Surface *bg;
	SDL_Surface *enemy;
} wave_t;

typedef struct global_s
{
	enum state_t state;
	enum focus_t focus;         /* selected line in menu */
	unsigned int enemies_alive;
	unsigned int enemy_bullets;
	unsigned int score;
	unsigned int hi_score;
	unsigned int prev_time;   /* TODO: maybe dont need it anymore */
	unsigned int frame_start; /* measure time for 1 frame rendering */
	unsigned int frame_end;
	unsigned int wave;        /* 0 when wave1, 1 when wave2 etc */
	unsigned int phase;       /* phase number */
	unsigned int wavestart_time; /* when we switched to STATE_WAVESTART */  
	unsigned int waveend_time; /* when we switched to STATE_WAVEEND */  
	unsigned int wavecleared_time; /* when we switched to STATE_WAVECLEARED */ 
	unsigned int wavescore_time; /* when we switched to STATE_WAVESCORE */
	unsigned int fps;        
	SDL_Surface *screen;
	SDL_Surface *text;
	SDL_Event event;
} global_t;

#endif
