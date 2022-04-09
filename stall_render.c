#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include "stall_ini.h"
#include "stall_types.h"
#include "stall.h"

extern global_t global;
extern TTF_Font *font[5];
extern player_ship_t *player_ship; 
extern enemy_ship_t enemy_ship[MAX_ENEMIES];
extern bullet_t player_bullet[PLAYER_BULLETS];
extern bullet_t enemy_bullet[ENEMY_BULLETS];
extern bonus_t bonus[BONUS_NUM];
extern dead_parts_t dead_parts[TOTAL_DEAD_PARTS]; 
extern bonus_defshield_t bonus_defshield;
extern wave_t waves[WAVE_NUM];
extern enum keys_t keys[KEY_MAX];

/* Surfaces */
SDL_Surface *sur_logo;
SDL_Surface *sur_wave1, *sur_wave2, *sur_wave3;
SDL_Surface *sur_wave_cleared;
SDL_Surface *sur_sprites;
SDL_Surface *sur_enemy;
SDL_Surface *sur_enemy_bullet;
SDL_Surface *sur_bonus[BONUS_NUM]; //TODO: rework when every bonus will have anims */
SDL_Surface *sur_bonus_ram[3];  /* frames per anim on player */
SDL_Surface *sur_boss1;
SDL_Surface *sur_blades; //test surface with blaze animation
SDL_Surface *sur_test_anim; //test surface with framenumbers


/* load picture from file to surface and convert it to right display format */
SDL_Surface *load_pic(char *filename)
{
	SDL_Surface *tmp1, *tmp2;
	Uint32 ck;

//	tmp1 = SDL_LoadBMP(filename);
	tmp1 = IMG_Load(filename);
	if (!tmp1)
	{
		error("cant load pic %s \n", filename);
		return NULL;
	}
	tmp2 = SDL_DisplayFormat(tmp1);
	if (!tmp2)
	{
		error("cant SDL_DisplayFormat %s \n", filename);
		return NULL;
	}
	/* set colorkey */
	ck = SDL_MapRGB(tmp2->format,0xFF,0,0xFF);
	SDL_SetColorKey(tmp2,SDL_SRCCOLORKEY,ck);
	SDL_FreeSurface(tmp1);
	return tmp2;
}

SDL_Surface *load_pic_alpha(char *filename)
{
	SDL_Surface *tmp1, *tmp2;

	tmp1 = IMG_Load(filename);
	if (!tmp1)
	{
		debug("cant load pic %s \n", filename);
		return NULL;
	}
	tmp2 = SDL_DisplayFormatAlpha(tmp1);
	if (!tmp2)
	{
		debug("cant SDL_DisplayFormat %s \n", filename);
		return NULL;
	}
	SDL_FreeSurface(tmp1);
	return tmp2;
}

/* load all pics */
void load_pictures()
{
	sur_logo = load_pic("/usr/local/share/stall/img/stall_logo.png");
	sur_wave1 = load_pic("/usr/local/share/stall/img/wave1.png");
	sur_wave2 = load_pic("/usr/local/share/stall/img/wave2.png");
	sur_wave3 = load_pic("/usr/local/share/stall/img/wave3.png");
	sur_wave_cleared = load_pic("/usr/local/share/stall/img/wave_cleared.png");
	sur_sprites = load_pic("/usr/local/share/stall/img/sprites1.png");
	sur_enemy = load_pic("/usr/local/share/stall/img/enemy48x39.png");
	sur_enemy_bullet = load_pic("/usr/local/share/stall/img/blue_bullet.png");

	/* load bgs into wave_t objects */
	waves[0].bg = load_pic("/usr/local/share/stall/img/bg_wave1.png");
	waves[1].bg = load_pic("/usr/local/share/stall/img/bg_wave2.png");
	waves[2].bg = load_pic("/usr/local/share/stall/img/bg_wave3.png");
	waves[3].bg = load_pic("/usr/local/share/stall/img/bg_wave4.png");
	waves[4].bg = load_pic("/usr/local/share/stall/img/bg_wave5.png");

	waves[0].enemy = load_pic("/usr/local/share/stall/img/enemies1.png");
	waves[1].enemy = load_pic_alpha("/usr/local/share/stall/img/enemies2.png");

	/* load boss bgs */
	sur_boss1 = load_pic("/usr/local/share/stall/img/bg_boss1.png");

	/* load array of bonuses */
	sur_bonus[0] = load_pic("/usr/local/share/stall/img/bonus_armor_breaker.png");
	sur_bonus[1] = load_pic("/usr/local/share/stall/img/bonus_triple_shot.png");
	sur_bonus[2] = load_pic_alpha("/usr/local/share/stall/img/bonus_defshield.png");
	sur_bonus[3] = load_pic("/usr/local/share/stall/img/bonus_random_bomb.png");
	sur_bonus[4] = load_pic("/usr/local/share/stall/img/bonus_ram.png");
	sur_bonus[5] = load_pic("/usr/local/share/stall/img/bonus_missiles.png");
	sur_bonus[6] = load_pic("/usr/local/share/stall/img/bonus_acc.png");

	sur_bonus_ram[0] = load_pic("/usr/local/share/stall/img/bonus_ram_00.png");
	sur_bonus_ram[1] = load_pic("/usr/local/share/stall/img/bonus_ram_01.png");
	sur_bonus_ram[2] = load_pic("/usr/local/share/stall/img/bonus_ram_02.png");

	/* bonuses on_player */
	bonus_defshield.img = load_pic("/usr/local/share/stall/img/defshield1.png");

//	sur_test_anim = load_pic("/usr/local/share/stall/img/test_animation.png");
//	sur_blades = load_pic("/usr/local/share/stall/img/strip_saucer_blades.png");
}


/*
   Base function to render text. Now OK.
   Args: font size, label, coords and color.color in 0xRRGGBBAA
*/
void render_text(Uint32 font_size, char str[20], int x, int y, Uint32 color)
{
	SDL_Color clr;
	SDL_Rect rect;
	TTF_Font *right_font;
	clr.r = (color & 0xFF000000) >> 24;
	clr.g = (color & 0x00FF0000) >> 16;
	clr.b = (color & 0x0000FF00) >> 8;
	rect.x = x;
	rect.y = y;

	switch(font_size)
	{
		case 12:
			right_font = font[0];
			break;
		case 14:
			right_font = font[1];
			break;
		case 16:
			right_font = font[2];
			break;
		case 18:
			right_font = font[3];
			break;
		case 20:
			right_font = font[4];
			break;	
		default: /* in case of trouble - use first one */
			right_font = font[0];
			error("font not found \n");
			break;
	}

	global.text=TTF_RenderUTF8_Blended(right_font,str,clr);
 	SDL_BlitSurface(global.text,NULL,global.screen,&rect);
	SDL_FreeSurface(global.text);
}

void render_background()
{
	Uint32 color = BG_COLOR;	

	if (global.state == STATE_MENU)
	{
		boxColor(global.screen,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,color);
	}
	else
	{
		boxColor(global.screen,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,color);
		SDL_BlitSurface(waves[global.wave].bg,NULL,global.screen,NULL);
	}
}

void render_menu()
{
	unsigned int color_green = 0x00FF00FF; 
	unsigned int color_white = 0xFFFFFFFF;
	unsigned int color;

	color = (global.focus==FOCUS_START) ? color_white : color_green;
	render_text(20,"START GAME",WINDOW_WIDTH/2-70,WINDOW_HEIGHT-150,color);
	color = (global.focus==FOCUS_OPTIONS) ? color_white : color_green;
	render_text(20,"OPTIONS",WINDOW_WIDTH/2-70,WINDOW_HEIGHT-125,color);
	color = (global.focus==FOCUS_EXIT) ? color_white : color_green;
	render_text(20,"EXIT",WINDOW_WIDTH/2-70,WINDOW_HEIGHT-100,color);
}

void render_fps()
{
	char str[20];

	render_text(14,"FPS : ",5,0,0xFFFF00FF);
	sprintf(str,"%u",global.fps);
	render_text(14,str,50,0,0xFFFF00FF);
}

void render_logo()
{
	SDL_Rect rect;
	rect.x = GAME_WIDTH/2 - 280;
	rect.y = GAME_HEIGHT/2 - 200;
 	SDL_BlitSurface(sur_logo,NULL,global.screen,&rect);
}

/* render label wave1, wave2 etc */
void render_label_wave()
{	
	SDL_Rect rect;
	SDL_Surface *sur_wave = NULL;

	rect.x = GAME_WIDTH/2 - 150;
	rect.y = GAME_HEIGHT/2 - 40;

	switch(global.wave)
	{
		case 0:
			sur_wave = sur_wave1;
			break;
		case 1:
			sur_wave = sur_wave2;
			break;
		case 2:
			sur_wave = sur_wave3;
			break;
		default:
			error("render_wave() no such wave! : %u \n", global.wave);
			break;
	}
 	SDL_BlitSurface(sur_wave,NULL,global.screen,&rect);
}

void render_label_wave_cleared()
{
	SDL_Rect dest;
	dest.x = 60;
	dest.y = GAME_HEIGHT/2 - 40;
 	SDL_BlitSurface(sur_wave_cleared,NULL,global.screen,&dest);
}

/* render score after the wave end */
void render_score()
{
	char str[20];
	render_text(20,"CURRENT SCORE : ",GAME_WIDTH/2-110,GAME_HEIGHT/2-200,0xFF4500FF);
	sprintf(str,"%u",global.score);
	render_text(20,str,GAME_WIDTH/2+80,GAME_HEIGHT/2-200,0xFF4500FF);
}

void render_hi_score()
{
	char str[20];
	render_text(14,"HI-SCORE : ",GAME_WIDTH-180,5,0xFF0000FF);
	sprintf(str,"%u",global.hi_score);
	render_text(14,str,GAME_WIDTH-90,5,0xFF0000FF);
	
}


/* word shield, bar, digits */
void render_shield_energy()
{
	char str[20];
	Uint32 color = 0x4169E1FF; /* royal blue */

	if (!bonus[BONUS_DEF_SHIELD].on_player)
		return;
	
	/* word SHIELD */
	render_text(12,"SHIELD",WINDOW_WIDTH/2 -70 ,WINDOW_HEIGHT-15,color);

	/* energy bar */
	boxColor(global.screen, SHIELD_BAR_X, WINDOW_HEIGHT-15,
		 SHIELD_BAR_X + bonus_defshield.energy, WINDOW_HEIGHT-5,color);
	rectangleColor(global.screen, SHIELD_BAR_X, WINDOW_HEIGHT-15,
		 SHIELD_BAR_X + SHIELD_BAR_W, WINDOW_HEIGHT-5, 0x0000FFFF);
	/* digits on the top of bar */
	sprintf(str,"%u",bonus_defshield.energy);
	render_text(12,str, WINDOW_WIDTH/2 + 25,WINDOW_HEIGHT-15,0xFFFFFFFF);
}

/* render time and bar remained till the end of the wave */
void render_wave_time()
{
	Uint32 time_passed, one_percent, percents_passed;
	Uint32 color = 0x00FF00FF; /* green, 0xRRGGBBAA */

	/* time passed from start of the wave */
	time_passed = SDL_GetTicks() - waves[global.wave].start_time;
	one_percent = waves[global.wave].duration/100; //600ms
	percents_passed = time_passed / one_percent;
	
	/* word TIME */
	render_text(12,"WAVE",WINDOW_WIDTH-155, WINDOW_HEIGHT-15,color);

	/* time bar. we assume TIME_BAR_W is 100 always ;) */
	if (TIME_BAR_W > percents_passed)
		boxColor(global.screen, TIME_BAR_X, WINDOW_HEIGHT-15,
			 TIME_BAR_X + TIME_BAR_W - percents_passed, WINDOW_HEIGHT-5, 0x32CD32FF);

	rectangleColor(global.screen, WINDOW_WIDTH - 120, WINDOW_HEIGHT-15,
			 WINDOW_WIDTH - 20, WINDOW_HEIGHT-5, 0x008000FF);

	/* digits on the top of bar */
	//sprintf(str,"%u",percents_passed);
	//render_text(12,str, WINDOW_WIDTH/2 + 25,WINDOW_HEIGHT-100,0xFFFFFFFF);
}

void render_interface()
{
	char str[20];
	Uint32 color = 0x00FF00FF;
 
	boxColor(global.screen,0,GAME_HEIGHT,GAME_WIDTH,GAME_HEIGHT+0,color);

	/* render hi_score */
	render_text(12,"Hi-score",20,WINDOW_HEIGHT-15,0xFF0000FF);
	sprintf(str,"%u",global.hi_score);
	render_text(12,str,85,WINDOW_HEIGHT-15,0xFF0000FF);

	/* render current score */
	render_text(12,"score",150,WINDOW_HEIGHT-15,0x00FF00FF);
	sprintf(str,"%u",global.score);
	render_text(12,str,190,WINDOW_HEIGHT-15,0x00FF00FF);

	render_shield_energy();
	render_wave_time();
}


/* draw text about player death for a few seconds */
void render_label_lose()
{
	Uint32 ms;
	static Uint32 old_ms = 0;
	static int cnt = 0;
	static Uint32 color = 0xFFFFFFFF; 
	/* when player is dead and after we've seen 100 frames of his agony */
	if (player_ship->dead_frame >= 100)
	{
		//save the time. then blink red and white every 0.5 secs
		ms = SDL_GetTicks();
		if (!old_ms)
			old_ms = ms;
		else
		{
			if ((ms - old_ms) > 300)
			{
			  color = (color==0xFFFFFFFF) ? 0xFF0000FF : 0xFFFFFFFF;
			  old_ms = ms;
			  cnt++;
			}
		}
		
		render_text(20,"DIE!!! DIE!!! DIE!!!",WINDOW_WIDTH/2-120,WINDOW_HEIGHT/2,color);

		if (cnt == 5)
		{
			old_ms = 0;
			cnt = 0;
			/* important - RESET this game and start a new one */
			stall_reset();
		}
	}	
}

void render_player()
{
	Uint32 color = CLR_RED;
	int x1,y1;
	SDL_Rect src, dest;
	static u8 even = 0;
	static u8 cnt = 0;
	
	// flame animation
	if (even)
		src.y = 89;
	else
		src.y = 43;

	//choose right sprite from the sprite list
	src.x = 41;
//	src.y = 88;
	src.w = PLAYER_WIDTH;
	src.h = PLAYER_HEIGHT;
	
	if (keys[KEY_LEFT])
	{
		src.x = 0;
//		src.y = 88;
		src.w = PLAYER_WIDTH;
		src.h = PLAYER_HEIGHT;
	}

	if (keys[KEY_RIGHT])
	{
		src.x = 82;
//		src.y = 88;
		src.w = PLAYER_WIDTH;
		src.h = PLAYER_HEIGHT;
	}


	//set destination
	dest.x = player_ship->crd.x1;
	dest.y = player_ship->crd.y1;
	src.w = PLAYER_WIDTH;
	src.h = PLAYER_HEIGHT;

	if (player_ship->alive)
	{
 		SDL_BlitSurface(sur_sprites,&src,global.screen,&dest);
	}
	else
	{
		/* draw player death animation (100 frames) */
		if (player_ship->dead_frame < 100)
		{
		x1 = player_ship->crd.x1 - player_ship->speed*player_ship->dead_frame;
		y1 = player_ship->crd.y1 - player_ship->speed*player_ship->dead_frame;
		boxColor(global.screen,x1,y1,x1+PLAYER_WIDTH/2,y1+PLAYER_HEIGHT/2,color);

		x1 = player_ship->crd.x1 + player_ship->speed*player_ship->dead_frame;
		y1 = player_ship->crd.y1 + player_ship->speed*player_ship->dead_frame;
		boxColor(global.screen,x1,y1,x1+PLAYER_WIDTH/2,y1+PLAYER_HEIGHT/2,color);

		x1 = player_ship->crd.x1 + player_ship->speed*player_ship->dead_frame;
		y1 = player_ship->crd.y1 - player_ship->speed*player_ship->dead_frame;
		boxColor(global.screen,x1,y1,x1+PLAYER_WIDTH/2,y1+PLAYER_HEIGHT/2,color);

		x1 = player_ship->crd.x1 - player_ship->speed*player_ship->dead_frame;
		y1 = player_ship->crd.y1 + player_ship->speed*player_ship->dead_frame;
		boxColor(global.screen,x1,y1,x1+PLAYER_WIDTH/2,y1+PLAYER_HEIGHT/2,color);

		player_ship->dead_frame++;
		}
	}

	cnt++;
	if (cnt == 12)
	{
		even = !even;
		cnt = 0;
	}
}

void render_dead_parts()
{
	int i;
	SDL_Rect src, dest;

	for (i = 0; i < TOTAL_DEAD_PARTS; i++)
	{
		if (dead_parts[i].alive)
		{
			src.x = dead_parts[i].pic.x;
			src.y = dead_parts[i].pic.y;
			src.w = dead_parts[i].pic.w;
			src.h = dead_parts[i].pic.h;
			dest.x = dead_parts[i].crd.x1;
			dest.y = dead_parts[i].crd.y1;
			//SDL_BlitSurface(sur_enemy, &src, global.screen, &dest);
			SDL_BlitSurface(waves[global.wave].enemy, &src, global.screen, &dest);

			//debug("render_dead_parts() #%d \n",i);
		}
	}
}

void render_enemies()
{
    int i;
    SDL_Rect src, dest;
    unsigned int diff; 

    for (i = 0; i < MAX_ENEMIES; i++)
    {
	if (enemy_ship[i].alive)
	{
		
		diff = SDL_GetTicks();	
		/* do not change the frame if enemy just created */
		if (!enemy_ship[i].time_anim)
		{
			enemy_ship[i].time_anim = diff;
			continue;
		}

		switch (global.wave)
		{
			case 0: case 1:
				src.x = enemy_ship[i].anim * waves[global.wave].enemy_width;
				src.y = 0;
				src.w = waves[global.wave].enemy_width;
				src.h = waves[global.wave].enemy_height;
				dest.x = enemy_ship[i].crd.x1;
				dest.y = enemy_ship[i].crd.y1;
				SDL_BlitSurface(waves[global.wave].enemy, &src, global.screen,&dest);
				if (diff - enemy_ship[i].time_anim > 1000/ENEMY_ANIM_SPEED)
				{
					enemy_ship[i].anim++;
					if (enemy_ship[i].anim >= ENEMY_ANIM_FRAMES)
						enemy_ship[i].anim = 0;
					enemy_ship[i].time_anim += 1000/ENEMY_ANIM_SPEED;
				}
			break;

/*
			case 1:
			break;
*/

			default:
			break;
		}
	}
    }
}

// 8 width, 16 height, 128x16
void render_bullets()
{
	int i;
	SDL_Rect src, dest;
	unsigned int diff; 

	/* render player bullets */
	Uint32 color = waves[global.wave].player_bullets_color;
	for (i = 0; i < PLAYER_BULLETS; i++)
	{
		if (player_bullet[i].exist)
		{
			boxColor(global.screen,player_bullet[i].crd.x1, player_bullet[i].crd.y1,
				 player_bullet[i].crd.x2, player_bullet[i].crd.y2,color);
		}
	}

	/* render enemy bullets */
	for (i = 0; i < ENEMY_BULLETS; i++)
	{
		if (enemy_bullet[i].exist)
		{
			/* choose correct sprite from spritelist */
			src.x = enemy_bullet[i].anim * ENEMY_BULLET_WIDTH;
			src.y = 0;
			src.w = ENEMY_BULLET_WIDTH;
			src.h = ENEMY_BULLET_HEIGHT;
			
			/* render chosen sprite to the correct place on the screen */
			dest.x = enemy_bullet[i].crd.x1;
			dest.y = enemy_bullet[i].crd.y1;
#if 0			
			if (i == 0)
				debug("anim %d src.x %d, src,y %d \n",
				 enemy_bullet[i].anim, src.x, src.y);
#endif 
 			SDL_BlitSurface(sur_enemy_bullet,&src,global.screen,&dest);

			/* check - time to render next animation sprite? */
			diff = SDL_GetTicks() - enemy_bullet[i].time_anim;
			if (diff > 1000/BULLET_ANIM_SPEED) 
			{
				//debug ("diff %u, anim %d \n", diff, enemy_bullet[i].anim);	

				enemy_bullet[i].anim++;
				if (enemy_bullet[i].anim >= BULLET_ANIM_FRAMES)
					enemy_bullet[i].anim = 0;
				/* correct. add +100, not +112 */
				enemy_bullet[i].time_anim += 1000/BULLET_ANIM_SPEED;
			}
		}
	}
}

void render_bonuses()
{
	int i;
	SDL_Rect dest;


	for (i = 0; i < BONUS_NUM; i++)
	{
		if (bonus[i].active)
		{
			dest.x = bonus[i].crd.x1;
			dest.y = bonus[i].crd.y1;
 			SDL_BlitSurface(sur_bonus[i],NULL,global.screen,&dest);
		}	
	}
}

void render_defshield()
{
	SDL_Rect dest;

	if (!bonus[BONUS_DEF_SHIELD].on_player)
		return;

	if (bonus_defshield.show)
	{
		dest.x = bonus_defshield.crd.x1;
		dest.y = bonus_defshield.crd.y1;
		SDL_BlitSurface(bonus_defshield.img,NULL,global.screen,&dest);
	}
}

/* render ram on_player */
void render_ram()
{
 	static Uint32 prev_time = 0, cur_time;
	static int frame_num = 0; /* number of animation frame */
	SDL_Rect dest;

	if (!bonus[BONUS_RAM].on_player || !player_ship->alive)
		return;

	dest.x = player_ship->crd.x1;
	dest.y = player_ship->crd.y1 - 20;
 	SDL_BlitSurface(sur_bonus_ram[frame_num],NULL,global.screen,&dest);

	cur_time = SDL_GetTicks();
	if (cur_time - prev_time > 1000/RAM_ANIM_SPEED)
	{
		prev_time = cur_time;
		frame_num++;
		if (frame_num >= RAM_ANIM_FRAMES)
			frame_num = 0;	
	}
}
