/* stall - shoot them all game */

#include <stdio.h>
#include <stdlib.h>  /* for malloc */
#include <string.h>  /* for memset */
#include <time.h>    /* for time */
#include <unistd.h>  /* for usleep */
#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_ttf.h>
#include "stall_types.h"
#include "stall_ini.h"
#include "stall_ai.h"
#include "stall_render.h"

void wave_inc();
void wave_end();
void stall_quit();
void stall_player_shot();
void stall_inc_score(unsigned int);
void stall_reset();
void dead_part_create(int, int);
void stall_objects_clear();

char *version = "0.82";

global_t global;
player_ship_t *player_ship; 
enemy_ship_t enemy_ship[MAX_ENEMIES];
bullet_t player_bullet[PLAYER_BULLETS] = {0}; /* all elements set to zero */
bullet_t enemy_bullet[ENEMY_BULLETS] = {0};
wave_t waves[WAVE_NUM] = {0};
bonus_t bonus[BONUS_NUM] = {0};
bonus_defshield_t bonus_defshield;
dead_parts_t dead_parts[TOTAL_DEAD_PARTS] = {0}; /* dead_parts v2.0 */

TTF_Font *font[5];

/* contains pressed keys. 1 - held, 0 - released */
enum keys_t keys[KEY_MAX] = {0};

/* --- menu functions --- */
void menu_down()
{
	if (global.focus == FOCUS_MAX - 1)
		global.focus = 0;
	else
		global.focus++;
}

void menu_up()
{
	if (global.focus > FOCUS_START)
		global.focus--;
	else
		global.focus = FOCUS_MAX - 1;
}

void menu_execute()
{
	switch (global.focus)
	{
		case FOCUS_START:
		{
			global.state = STATE_WAVESTART;
			global.wavestart_time = SDL_GetTicks();
			debug("switched to STATE_WAVESTART\n");
			break;	
		}
		case FOCUS_OPTIONS:
		{
			break;	
		}
		case FOCUS_EXIT:
		{
			stall_quit();
			break;	
		}
		default:
			break;
	}
}

/* --- keyboard functions --- */
void key_press(SDL_keysym *keysym)
{
	//debug("press\n");
	switch (keysym->sym)
	{
		case SDLK_ESCAPE:
		{
			stall_quit();
			break;
		}
		case SDLK_LEFT:
		{
			keys[KEY_LEFT] = 1;
			break;
		}	
		case SDLK_RIGHT:
		{
			keys[KEY_RIGHT] = 1;
			break;
		}		
		case SDLK_UP:
		{
			keys[KEY_UP] = 1;
			if (global.state == STATE_MENU)
				menu_up();
			break;
		}	
		case SDLK_DOWN:
		{
			keys[KEY_DOWN] = 1;
			if (global.state == STATE_MENU)
				menu_down();
			break;
		}		
		case SDLK_SPACE:
		{
			if (global.state == STATE_MENU)
			{
				menu_execute();
				break;
			}
			if (global.state == STATE_MAIN)
				stall_player_shot();
			break;
		}	
		case SDLK_PAGEDOWN:
		{
			if (global.state == STATE_MAIN)
			{
				wave_end();
				debug("level changed.\n");
			}
			break;
		}		

		default:
			break;
	}

	return;
}

void key_release(SDL_keysym *keysym)
{
	//debug("release\n");
	switch (keysym->sym)
	{
		case SDLK_ESCAPE:
		{
			stall_quit();
			break;
		}
		case SDLK_LEFT:
		{
			keys[KEY_LEFT] = 0;
			break;
		}	
		case SDLK_RIGHT:
		{
			keys[KEY_RIGHT] = 0;
			break;
		}		
		case SDLK_UP:
		{
			keys[KEY_UP] = 0;
			break;
		}	
		case SDLK_DOWN:
		{
			keys[KEY_DOWN] = 0;
			break;
		}	
		default:
			break;
	}

	return;
}

void stall_keyboard(void)
{
	while (SDL_PollEvent(&global.event))
	{
		switch (global.event.type)
		{
			case SDL_KEYDOWN:
				key_press(&global.event.key.keysym);
				break;
			case SDL_KEYUP:
				key_release(&global.event.key.keysym);
				break;
			default:
				break;
		}
	}
}

/* ---------------------------- destroying enemy functions ------------------ */

/* enemy gets killed by bullet - few dead parts created */
void stall_kill_enemy(int ship_num)
{
	int i;
	if (enemy_ship[ship_num].alive)
	{
		enemy_ship[ship_num].alive = 0;
		global.enemies_alive--;
		//debug("enemy #%d killed \n",ship_num);

		if (global.state == STATE_MAIN)
			stall_inc_score(ENEMY_COST);
		
		/* create dead_parts. quantity according to current wave */
		for (i = 0; i < waves[global.wave].dead_parts_num; i++)
		{
			dead_part_create(ship_num,i);
		}
	}
}

/* enemy gets crushed by player_ship - no dead parts created */
void stall_krash_enemy(int ship_num)
{
	int i;
	if (enemy_ship[ship_num].alive)
	{
		enemy_ship[ship_num].alive = 0;
		global.enemies_alive--;
		debug("enemy #%d krushed \n",ship_num);

		if (global.state == STATE_MAIN)
			stall_inc_score(ENEMY_COST);

		if (bonus[BONUS_RAM].on_player)
			return;
	}
}

/* --- BONUS_DEF_SHIELD functions --- */

void bonus_defshield_move()
{
	bonus_defshield.crd.x1 = player_ship->crd.x1 - BONUS_DEFSHIELD_SIZE;
	bonus_defshield.crd.y1 = player_ship->crd.y1 - BONUS_DEFSHIELD_SIZE;
	bonus_defshield.crd.x2 = player_ship->crd.x2 + BONUS_DEFSHIELD_SIZE;
	bonus_defshield.crd.y2 = player_ship->crd.y2 + BONUS_DEFSHIELD_SIZE;
}

void bonus_defshield_setenergy(int energy)
{
	bonus_defshield.energy = energy;
}

void bonus_defshield_blink()
{
	static Uint32 cur_time, prev_time = 0;
	
	if (!bonus_defshield.blinking)
		return;

	cur_time = SDL_GetTicks();
	if (cur_time - prev_time > BONUS_DEFSHIELD_BLINK_FREQ)
	{
		bonus_defshield.show = !bonus_defshield.show;
		prev_time = cur_time;
	}
}

void bonus_defshield_create()
{
	bonus_defshield_move();
	bonus_defshield.show = 1;
	bonus_defshield_setenergy(100);
}

void bonus_defshield_destroy()
{
	bonus[BONUS_DEF_SHIELD].on_player = 0;
	bonus_defshield.show = 0;
	bonus_defshield.blinking = 0;
	debug("defshield destroyed \n");
}

void bonus_defshield_decenergy(int energy)
{
	bonus_defshield.energy -=energy;

	debug("decenergy %d , remain: %d \n", energy, bonus_defshield.energy);

	if (bonus_defshield.energy <= 0)
		bonus_defshield_destroy();
}

/* --- END BONUS_DEF_SHIELD functions --- */

/* --- BONUS_RANDOMBOMB functions --- */

void bonus_randombomb()
{
	int i, cnt = 0;
	for (i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemy_ship[i].alive)
		{
			stall_kill_enemy(i);
			cnt++;
		}
		if (cnt == BONUS_RANDOMBOMB_ENEMIES)
			break;
	}
}

/* --- BONUS_RAM functions --- */

void bonus_ram_start()
{
	player_ship->ram = 1;
}

void bonus_ram_stop()
{
	player_ship->ram = 0;
}

/* --- BONUS_AIM functions --- */


/* need to find the closest one */
void bonus_aim_target(int num)
{
	int i;
	/* player coords */
	int base_x = player_ship->crd.x1 + PLAYER_WIDTH/2;
	int base_y = player_ship->crd.y1;
	/* enemy coord */
	int cur_target_pts, cur_target_num;
	int target_pts = BONUS_AIM_ERRVALUE, target_num = BONUS_AIM_ERRVALUE;

	for (i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemy_ship[i].alive)
		{
			/* calculate distance (points) from base to target by x+y sum */
			cur_target_pts = base_x - (enemy_ship[i].crd.x1 + waves[global.wave].enemy_width/2); 
			cur_target_pts += base_y - enemy_ship[i].crd.y2;
			
			/* if we found the closer target - save its num and points */
			if (cur_target_pts < target_pts)
			{
				target_pts = cur_target_pts;
				target_num = i;
			}
		}
	}

	if (target_pts == BONUS_AIM_ERRVALUE || target_num == BONUS_AIM_ERRVALUE)
	{
		enemy_bullet[num].target_id = -1;
		debug("error: no aim target found! \n");
	}
	else
	{
		enemy_bullet[num].target_id = target_num;
		debug(" target found: #%d \n",target_num);
	}

}

void bonus_aim_move()
{
	//if (enemy_bullet[num].target_id > 0)
		/*TODO: change bullet coords here (move) */
	
}


/* yes we decrease coord first and then check.
   if value is negative - set it to 0.
   so we need signed int coords
*/
void stall_player_move()
{
	/* cant control dead ship */
	if (!player_ship->alive)
		return;

	if (keys[KEY_LEFT])
	{
		player_ship->crd.x1 -= player_ship->speed;
		player_ship->crd.x2 -= player_ship->speed;
		if (player_ship->crd.x1 < 0)
		{
			player_ship->crd.x1 = 0;
			player_ship->crd.x2 = PLAYER_WIDTH;
		} 
	}
	if (keys[KEY_RIGHT])
	{
		player_ship->crd.x1 += player_ship->speed;
		player_ship->crd.x2 += player_ship->speed;
		if (player_ship->crd.x2 > GAME_WIDTH)
		{
			player_ship->crd.x2 = GAME_WIDTH;
			player_ship->crd.x1 = GAME_WIDTH - PLAYER_WIDTH;
		} 
	}
	if (keys[KEY_UP])
	{
		player_ship->crd.y1 -= player_ship->speed;
		player_ship->crd.y2 -= player_ship->speed;
		if (player_ship->crd.y1 < 0)
		{
			player_ship->crd.y1 = 0;
			player_ship->crd.y2 = PLAYER_HEIGHT;
		} 
	}	
	if (keys[KEY_DOWN])
	{	
		player_ship->crd.y1 += player_ship->speed;
		player_ship->crd.y2 += player_ship->speed;
		if (player_ship->crd.y2 > GAME_HEIGHT)
		{
			player_ship->crd.y2 = GAME_HEIGHT;
			player_ship->crd.y1 = GAME_HEIGHT - PLAYER_HEIGHT;
		} 
	}

	bonus_defshield_move();
}

void stall_player_ship_creator()
{
	player_ship->crd.x1 = GAME_WIDTH/2;
	player_ship->crd.y1 = GAME_HEIGHT/2 + 100; 
	player_ship->crd.x2 = player_ship->crd.x1 + PLAYER_WIDTH;
	player_ship->crd.y2 = player_ship->crd.y1 + PLAYER_HEIGHT;
	player_ship->speed = PLAYER_SPEED;
	player_ship->bullet_speed = PLAYER_BULLET_SPEED;
	player_ship->alive = 1;
	player_ship->dead_frame = 0;
	player_ship->ram = 0;
}

void stall_enemy_ship_creator()
{
	if (global.enemies_alive < MAX_ENEMIES)
	{
		int i;
		for (i = 0; i < MAX_ENEMIES; i++)
		{
			if (!enemy_ship[i].alive)
			{
			    /* choose - appear from left or from right side */
		    	    int rnd = rand() % 3;
			    if (rnd == 0) /* from left side */
			    {
				enemy_ship[i].crd.x1 = - waves[global.wave].enemy_width + 5;
				enemy_ship[i].crd.y1 = rand() % (GAME_HEIGHT/2);
				enemy_ship[i].move = MOVE_RIGHT;
				enemy_ship[i].goal_x = rand() % (GAME_WIDTH - waves[global.wave].enemy_width);
				enemy_ship[i].goal_y = enemy_ship[i].crd.y1;	
			    }
			    if (rnd == 1) /* from right side */
			    {
				enemy_ship[i].crd.x1 = GAME_WIDTH - 5;
				enemy_ship[i].crd.y1 = rand() % (GAME_HEIGHT/2);
				enemy_ship[i].move = MOVE_LEFT;
				enemy_ship[i].goal_x = rand() % (GAME_WIDTH - waves[global.wave].enemy_width);
				enemy_ship[i].goal_y = enemy_ship[i].crd.y1;	
			    }

			    if (rnd == 2) /* from top side */
			    {
				enemy_ship[i].crd.x1 = rand() % (GAME_WIDTH - waves[global.wave].enemy_width);
				enemy_ship[i].crd.y1 = - waves[global.wave].enemy_height + 5;
				enemy_ship[i].move = MOVE_DOWN;
				enemy_ship[i].goal_x = enemy_ship[i].crd.x1;
				enemy_ship[i].goal_y = rand() % 400;
					
			    }

			/* enemy_ship[i].crd.x1 = rand() % (GAME_WIDTH - waves[global.wave].enemy_width); 
			enemy_ship[i].move = MOVE_DONT;  */

			    /* common */
			    enemy_ship[i].crd.x2 = enemy_ship[i].crd.x1 + waves[global.wave].enemy_width;
			    enemy_ship[i].crd.y2 = enemy_ship[i].crd.y1 + waves[global.wave].enemy_height;
			    enemy_ship[i].speed = waves[global.wave].enemy_speed;
			    enemy_ship[i].bullet_speed = ENEMY_BULLET_SPEED;
			    enemy_ship[i].mental = rand() % MENTAL_MAX;
			    enemy_ship[i].in_action = 1;
			    enemy_ship[i].alive = 1;
			    enemy_ship[i].anim = 0;
			    enemy_ship[i].time_anim = 0;
			    break;
			}
		}
		global.enemies_alive++;
		debug("enemy created. alive: %u, speed %u \n",
			 global.enemies_alive,enemy_ship[i].speed);
	}
}

void wave_inc()
{
	global.wave++;
}


/* called once when wave is ended */
void wave_end()
{
	int i;
	debug("wave end(). #%u \n", global.wave);

	/* this is STATE_WAVEEND from now and next 5 seconds */
	global.state = STATE_WAVEEND;
	global.waveend_time = SDL_GetTicks();

	/* kill all enemies */
	for (i = 0; i < MAX_ENEMIES; i++)
	{
		stall_kill_enemy(i);
	}

	for (i = 0; i < BONUS_NUM; i++)
	{
		bonus[i].active = 0;
	}
		
	global.phase = 0;
}

/* creating enemies, switching phases, switch state to STATE_WAVEEND */
void wave_live()
{
	static Uint32 prev_time = 0;
	static Uint32 phase_duration, phase_enemyfreq, phase_starttime;
	static Uint32 phase_start = 1; /* we have just switched to this phase */

	Uint32 cur_time = SDL_GetTicks();

	/* switch to STATE_WAVEEND or not */
	if (cur_time - waves[global.wave].start_time > waves[global.wave].duration)
	{
		wave_end();
		return;
	}

	/* phase init - do it just once per phase */
	if (phase_start)
	{
		phase_duration = waves[global.wave].phases[global.phase][0] * 1000;
		phase_enemyfreq = waves[global.wave].phases[global.phase][1];
		phase_starttime = cur_time;
		phase_start = 0;
	}

	/* check - is it time to create enemy? */
	if ((cur_time - prev_time) >= phase_enemyfreq)
	{
		stall_enemy_ship_creator();
		prev_time = cur_time;
	}

	/* check - is it time to switch to another phase? */
	if ((cur_time - phase_starttime) >= phase_duration)
	{
		/* additional check to avoid bug which can switch to phase6 */
		if (global.phase < WAVE_PHASES - 1)
		{
			phase_start = 1;
			global.phase++;
			debug("phase: %u \n", global.phase);
		}
	}
}


/* check every second - is it time to create new bonus */
void bonus_creator()
{
	int i, rnd;
	static Uint32 cur_time, prev_time = 0;

	if (global.state != STATE_MAIN)
		return;

	/* is it time to create bonus? */
	cur_time = SDL_GetTicks();
	if ((cur_time - prev_time) >= 1000)
	{
		for (i = 0; i < BONUS_NUM; i++)
		{
			rnd = rand() % 100;
			//debug("bonus_creator() rnd %d \n", rnd);

			/* do not create bonus if we already have it on screen */
			if (!bonus[i].active  && rnd < bonus[i].chance[global.wave])
			{
				/* creating bonus! */
				bonus[i].active = 1;
				bonus[i].scr_lifetime = bonus[i].scr_duration;
				bonus[i].crd.x1 = rand() % (GAME_WIDTH - BONUS_WIDTH);
				bonus[i].crd.x2 = bonus[i].crd.x1 + BONUS_WIDTH;
				bonus[i].crd.y1 = rand() % (GAME_HEIGHT - BONUS_HEIGHT);
				bonus[i].crd.y2 = bonus[i].crd.y1 + BONUS_HEIGHT;
				/*debug("bonus created! #%d ,x1 %d, y1 %d\n",
				i,bonus[i].crd.x1, bonus[i].crd.y1);*/
			}
		}
		prev_time = cur_time;
	}
}

/* check every sec - is it time to decrease bonus display_time or stop bonus */
void bonus_disappear()
{
	int i;
	static Uint32 cur_time, prev_time = 0;

	/* is it time to disappear/stop bonus? */
	cur_time = SDL_GetTicks();
	if ((cur_time - prev_time) >= 1000)
	{
		for (i = 0; i < BONUS_NUM; i++)
		{
			if (bonus[i].active)
			{	
				if (bonus[i].scr_lifetime > 0)
				{
					bonus[i].scr_lifetime--;
				 	/* debug("#%d : dec scr_lifetime %d \n",
				  	i, bonus[i].scr_lifetime);*/
				}
				else
				{
					bonus[i].active = 0;
					debug("bonus_disappear #%d \n", i);
				}
			}
			if (bonus[i].on_player)
			{	
				if (bonus[i].plr_lifetime > 0)
				{
					bonus[i].plr_lifetime--;
					if (i == BONUS_DEF_SHIELD && 
					bonus[i].plr_lifetime == BONUS_DEFSHIELD_BLINKING_TIME)
						bonus_defshield.blinking = 1;
				  	/*	debug("#%d : dec plr_lifetime %d \n",
				  	i, bonus[i].plr_lifetime); */
				}
				else
				{
					bonus[i].on_player = 0;
					if (i == BONUS_RAM)
						bonus_ram_stop();
					else if (i == BONUS_ACCELERATION)
						player_ship->speed = PLAYER_SPEED;
				}
			}
		}
		prev_time = cur_time;
	}
}

void stall_bullets_move()
{
	int i;
	/* move player bullets */	
	for (i = 0; i < PLAYER_BULLETS; i++)
	{
		if (player_bullet[i].exist)
		{
			player_bullet[i].crd.y1 -= player_ship->bullet_speed;
			player_bullet[i].crd.y2 -= player_ship->bullet_speed;
			if (player_bullet[i].btype == BULLET_TYPE_RIGHT)
			{
				player_bullet[i].crd.x1 +=1;
				player_bullet[i].crd.x2 +=1;
			}	
			else if (player_bullet[i].btype == BULLET_TYPE_LEFT)
			{
				player_bullet[i].crd.x1 -=1;
				player_bullet[i].crd.x2 -=1;
			}

			if (player_bullet[i].crd.y2 < 1 || player_bullet[i].crd.x1 < 1 
			    || player_bullet[i].crd.x2 > GAME_WIDTH )
			{
				player_bullet[i].exist = 0;
			}
		}
	}
	/* move enemy bullets and destroy them if out of field */	
	for (i = 0; i < ENEMY_BULLETS; i++)
	{
		if (enemy_bullet[i].exist)
		{
			int id = enemy_bullet[i].ship_id;
			enemy_bullet[i].crd.y1 += enemy_ship[id].bullet_speed;
			enemy_bullet[i].crd.y2 += enemy_ship[id].bullet_speed;
			if (enemy_bullet[i].crd.y1 > GAME_HEIGHT)
			{
				enemy_bullet[i].exist = 0;
				enemy_bullet[i].anim = 0;
				enemy_bullet[i].time_anim = 0;
				global.enemy_bullets--;
				if (player_ship->alive)
					stall_inc_score(BULLET_COST);
			}
		}
	}
}

void stall_player_shot()
{
	int i;
	static int triple_shot_cnt = 0;

	if (!player_ship->alive)
		return;

	for (i = 0; i < PLAYER_BULLETS; i++)
	{
		if (!player_bullet[i].exist)
		{
			player_bullet[i].crd.x1 = player_ship->crd.x1 + PLAYER_WIDTH/2;
			player_bullet[i].crd.y1 = player_ship->crd.y1 - 1;
			player_bullet[i].crd.x2 = player_bullet[i].crd.x1 + PLAYER_BULLET_WIDTH;
			player_bullet[i].crd.y2 = player_bullet[i].crd.y1 + PLAYER_BULLET_HEIGHT;
			player_bullet[i].btype = triple_shot_cnt;
			player_bullet[i].exist = 1;
			
			if (bonus[BONUS_TRIPLE_SHOT].on_player)
			{
				/* we already launched 3 bullets, so break */
				if (triple_shot_cnt == 2)
				{
					triple_shot_cnt = 0;
					break;
				}
					
				triple_shot_cnt++;
				continue;
			}

			if (bonus[BONUS_AUTO_AIM_MISSILES].on_player)
			{
				player_bullet[i].btype = BULLET_TYPE_MISSILE;
				bonus_aim_target(i);
			}
			
			break;
		}
	}
}

void stall_enemy_shot()
{
	int i, j;
	for (i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemy_ship[i].alive)
		{
			int random = rand() % 200;
			if (random == 77)
			{
			    for (j = 0; j < ENEMY_BULLETS; j++)
			    {
				if (!enemy_bullet[j].exist)
				{
				    enemy_bullet[j].crd.x1 = enemy_ship[i].crd.x1 + waves[global.wave].enemy_width/2;
				    enemy_bullet[j].crd.y1 = enemy_ship[i].crd.y2 + 1;
				    enemy_bullet[j].crd.x2 = enemy_bullet[j].crd.x1 + ENEMY_BULLET_WIDTH;
				    enemy_bullet[j].crd.y2 = enemy_bullet[j].crd.y1 + ENEMY_BULLET_HEIGHT;
				    enemy_bullet[j].btype = BULLET_TYPE_CENTER;
				    enemy_bullet[j].exist = 1;
				    enemy_bullet[j].time_anim = SDL_GetTicks();
				    global.enemy_bullets++;
				    //    debug("enemy bullet: %u\n",global.enemy_bullets);
				    break;
				}
			    }
			}
		}
	}
}

/* --- Math --- */

/* inc when enemy killed or dodge from bullet */
void stall_inc_score(unsigned int score)
{
	//debug("inc_score() +%d \n", score);
	global.score += score;
}

#if 0
/* returns: 1 if we have a collision, 0 if not. 
   x1, y1, x2, y2 */
int stall_check_collision_old ( int left1, int top1, int right1, int bottom1,
			    int left2, int top2, int right2, int bottom2 )
{
	if (bottom1 < top2 || top1 > bottom2 || right1 < left2 || left1 > right2 )
		return 0;
	/* here we have collision */
	//debug("l1 %d, t1 %d, r1 %d, b1 %d , l2 %d, t2 %d, r2 %d, b2 %d \n",
	//      left1, top1, right1, bottom1, left2, top2, right2, bottom2);
	return 1;
}
#endif

/* returns: 1 if we have a collision, 0 if not */
int stall_check_collision (rect_t rect1, rect_t rect2)
{
	if (rect1.y2 < rect2.y1 || rect1.y1 > rect2.y2 || rect1.x2 < rect2.x1 || rect1.x1 > rect2.x2)
		return 0;
	/* here we have collision */
	return 1;
}

/* if player took bonus - `on player` enabled */
void bonus_check()
{
	int i, j;
	for (i = 0; i < BONUS_NUM; i++)
	{
		if (bonus[i].active)
		{	
		    if (player_ship->alive)
		    {
			if (stall_check_collision(player_ship->crd, bonus[i].crd))
			{
				debug("bonus_check() : bonus was taken %d \n", i);
			 	bonus[i].active = 0;
			 	bonus[i].on_player = 1;
				bonus[i].plr_lifetime = bonus[i].plr_duration;
				if (i == BONUS_DEF_SHIELD)
					bonus_defshield_create();
				else if (i == BONUS_RANDOM_BOMB)
				{
					bonus_randombomb();
				}	
				else if (i == BONUS_RAM)
				{
					bonus_ram_start();
				}	
				else if (i == BONUS_ACCELERATION)
				{
					player_ship->speed = PLAYER_ACC_SPEED;
				}	
			}
		    }
		    /* enemies can eat the bonus! */
		    for (j = 0; j < MAX_ENEMIES; j++)
		    {
			if (enemy_ship[j].alive)
			{
			    if (stall_check_collision(enemy_ship[j].crd, bonus[i].crd))
				bonus[i].active = 0;
			}
			
		    }
		}
	}
}

/* --- dead_part v2.0 functions --- */

/* creates 1 dead part when enemy gets killed 
params: 
@ ship_number - number of parent ship (to get coords),
@ part_number - number of part (from 1 to 4)
*/

void dead_part_create(int ship_number,int part_number) 
{
  int i;

  for (i = 0; i < TOTAL_DEAD_PARTS; i++)
  {
    if (!dead_parts[i].alive)
    {
    
//    debug("dp create()ship %d, part_n %d ,num %d\n",ship_number, part_number, i);
    dead_parts[i].alive = 1;
    dead_parts[i].speed = waves[global.wave].enemy_speed;

    switch (global.wave)
    {
	case 0:
	{   
	    if (part_number == 0) /* top  */
	    {
	        /* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = 0;
	    	dead_parts[i].pic.y = 0;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] =  0; //x
	        dead_parts[i].way[1] = -1; //y
	    }
	    else if (part_number == 1)  /* bottom right */
	    {
		/* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1 + waves[global.wave].enemy_width/2;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.y = waves[global.wave].enemy_height/2;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] = 1; //x
	        dead_parts[i].way[1] = 0; //y
	    }

	    else if (part_number == 2)
	    {
		/* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x1 + waves[global.wave].enemy_width/2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = 0;
	    	dead_parts[i].pic.y = waves[global.wave].enemy_height/2;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] = -1; //x
	        dead_parts[i].way[1] = 0; //y
	    }
	    break;
	}
	case 1:
	{   
	    if (part_number == 0) /* top left */
	    {
	        /* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1;
	    	dead_parts[i].crd.x2 = dead_parts[i].crd.x1 + waves[global.wave].enemy_width/2;
	    	dead_parts[i].crd.y2 = dead_parts[i].crd.y1 + waves[global.wave].enemy_height/2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = 0;
	    	dead_parts[i].pic.y = 0;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] = -1; //x
	        dead_parts[i].way[1] = -1; //y
	    }
	    else if (part_number == 1) /* top right */
	    {
		/* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1 + waves[global.wave].enemy_width/2;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.y = 0;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] = 1; //x
	        dead_parts[i].way[1] = -1; //y
	    }
	    else if (part_number == 2)  /* bottom right */
	    {
		/* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1 + waves[global.wave].enemy_width/2;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.y = waves[global.wave].enemy_height/2;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] = 1; //x
	        dead_parts[i].way[1] = 1; //y
	    }

	    else if (part_number == 3)
	    {
		/* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x1 + waves[global.wave].enemy_width/2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = 0;
	    	dead_parts[i].pic.y = waves[global.wave].enemy_height/2;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width/2;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] = -1; //x
	        dead_parts[i].way[1] = 1; //y
	    }

	    break;
	}
	case 2:
	{   
	    if (part_number == 0) /* top  */
	    {
	        /* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = 0;
	    	dead_parts[i].pic.y = 0;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] =  0; //x
	        dead_parts[i].way[1] = -1; //y
	    }
	    else if (part_number == 1) /* bottom */
	    {
	        /* set coords of new part based on parent ship coords */
	    	dead_parts[i].crd.x1 = enemy_ship[ship_number].crd.x1;
	    	dead_parts[i].crd.y1 = enemy_ship[ship_number].crd.y1 + waves[global.wave].enemy_height/2;
	    	dead_parts[i].crd.x2 = enemy_ship[ship_number].crd.x2;
	    	dead_parts[i].crd.y2 = enemy_ship[ship_number].crd.y2;

		/* set part of the sprite for the current part */
	    	dead_parts[i].pic.x = 0;
	    	dead_parts[i].pic.y = waves[global.wave].enemy_height/2;
	    	dead_parts[i].pic.w = waves[global.wave].enemy_width;
	    	dead_parts[i].pic.h = waves[global.wave].enemy_height/2;

		/* set way to move */
	        dead_parts[i].way[0] =  0; //x
	        dead_parts[i].way[1] =  1; //y
	    }
	    break;
	}

        default:
	    break;
    }
#if 0    
    debug("dp created %d, x1 %d, y1 %d, x2 %d, y2 %d \n",
           i, dead_parts[i].crd.x1,dead_parts[i].crd.y1,
          dead_parts[i].crd.x2,dead_parts[i].crd.y2);
#endif
   break;	
   }
  }
}

void dead_part_move()
{
    int i;

    for (i = 0; i < TOTAL_DEAD_PARTS; i++)
    {
	if (dead_parts[i].alive)
	{
		dead_parts[i].crd.x1 += dead_parts[i].way[0] * dead_parts[i].speed;
		dead_parts[i].crd.y1 += dead_parts[i].way[1] * dead_parts[i].speed;
		dead_parts[i].crd.x2 += dead_parts[i].way[0] * dead_parts[i].speed;
		dead_parts[i].crd.y2 += dead_parts[i].way[1] * dead_parts[i].speed;
	}
    }
}

void dead_part_push(int num)
{
	if (dead_parts[num].way[0] == -1)
	{
		dead_parts[num].way[0] = 1;
		dead_parts[num].crd.x1 +=30;
		dead_parts[num].crd.x2 +=30;
	}
	else if (dead_parts[num].way[0] == 1)
	{
		dead_parts[num].way[0] = -1;
		dead_parts[num].crd.x1 -=30;
		dead_parts[num].crd.x2 -=30;
	}

	if (dead_parts[num].way[1] == -1)
	{
		dead_parts[num].way[1] = 1;
		dead_parts[num].crd.y1 +=30;
		dead_parts[num].crd.y2 +=30;
	}
	else if (dead_parts[num].way[1] == 1)
	{
		dead_parts[num].way[1] = -1;
		dead_parts[num].crd.y1 -=30;
		dead_parts[num].crd.y2 -=30;
	}
	
	debug("push \n");
}

/* check collision between player ship and dead parts */
void dead_part_collision()
{
	int i, j;
	for (i = 0; i < TOTAL_DEAD_PARTS; i++)
	{
		if (dead_parts[i].alive)
		{
			/* check collision between player_ship and dead_part */
		    	if (player_ship->alive)
			{
				if (stall_check_collision(player_ship->crd, dead_parts[i].crd))
				{	
					/* if RAM - just destroy dead_part and go to the next */
					if (bonus[BONUS_RAM].on_player)
					{
				    		dead_parts[i].alive = 0;
						continue;
					}
					else if (bonus[BONUS_DEF_SHIELD].on_player)
					{
						bonus_defshield_decenergy(BONUS_DEFSHIELD_DEATHPART);
						dead_part_push(i);
					}
					else if (!bonus[BONUS_DEF_SHIELD].on_player &&  
					    !bonus[BONUS_RAM].on_player)
					{
						player_ship->alive = 0;
						debug("player killed by dead_part \n");
					}
				}
			}
			/* check collision between enemy_ship and dead_part */
			for (j = 0; j < MAX_ENEMIES; j++)
			{
			    if (enemy_ship[j].alive)
			    {
				if (stall_check_collision(enemy_ship[j].crd, dead_parts[i].crd))
				{
				    dead_parts[i].alive = 0;
				    stall_kill_enemy(j);
				}
			    }
			}
		}
	}
}

/* destroy when move out of the screen or killed by player bullet */
void dead_part_destroy()
{	
	int i, j;
	for (i = 0; i < TOTAL_DEAD_PARTS; i++)
	{
		/* out of screen */
		if (dead_parts[i].alive)
		{
			if (dead_parts[i].crd.x2 < 0 || dead_parts[i].crd.x1 > GAME_WIDTH ||
			    dead_parts[i].crd.y2 < 0 || dead_parts[i].crd.y1 > GAME_HEIGHT)
			{
				dead_parts[i].alive = 0; 
				continue; 
			}
		}
	
		/* killed by player's bullet */
		for (j = 0; j < PLAYER_BULLETS; j++)
		{
			if (player_bullet[j].exist && dead_parts[i].alive)
				if (stall_check_collision(player_bullet[j].crd, dead_parts[i].crd))
				{
					dead_parts[i].alive = 0;
					player_bullet[j].exist = 0;
					stall_inc_score(DEAD_PART_COST);
				}
		}
	}
}

void stall_push_player()
{
	player_ship->crd.y1 += 100;
	player_ship->crd.y2 += 100;
	debug("player pushed back! \n");
}

void stall_check_crash()
{
	int i;
	if (!player_ship->alive)
		return;

	/* check collision between enemy and player ship */
	for (i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemy_ship[i].alive)
		{
			if (stall_check_collision(player_ship->crd, enemy_ship[i].crd))
			{
				/* if RAM - kill enemy without penalty to the shield */
				if (bonus[BONUS_RAM].on_player)
				{
					//stall_krash_enemy(i);
					stall_kill_enemy(i);
					continue;
				}
				else if (bonus[BONUS_DEF_SHIELD].on_player)
				{
					bonus_defshield_decenergy(BONUS_DEFSHIELD_ENEMY);
					stall_push_player();
				}

				else if (!bonus[BONUS_DEF_SHIELD].on_player && 
				    !bonus[BONUS_RAM].on_player)
				{
					player_ship->alive = 0;
					debug("player killed by krash \n");
				}
			}
		}
	}
}

/* check if enemy or player get killed */
void stall_check_kill()
{
	int i, j;
	for (i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemy_ship[i].alive)
		{
			for (j = 0; j < PLAYER_BULLETS; j++)
			{
				if (player_bullet[j].exist)
				{
										
				if (stall_check_collision(player_bullet[j].crd, enemy_ship[i].crd))
				{
					stall_kill_enemy(i);
					if (!bonus[BONUS_ARMOR_BREAKER].on_player)
						player_bullet[j].exist = 0;
					break;
				}
				}
			}
		}
	}
/* in GOD MODE player becomes invulnerable. he-he */
#ifndef GOD_MODE
    if (player_ship->alive)
    {
	for (i = 0; i < ENEMY_BULLETS; i++)
	{
		if (enemy_bullet[i].exist)
		{
			if (stall_check_collision(enemy_bullet[i].crd, player_ship->crd))
			{
				if (!bonus[BONUS_DEF_SHIELD].on_player)
					player_ship->alive = 0;
				else
				{	
					enemy_bullet[i].exist = 0;
					bonus_defshield_decenergy(BONUS_DEFSHIELD_BULLET);
				}
			}
		}
	}
    }
#endif 
}

/* --- End of Math --- */

void stall_fps_counter()
{
	static Uint32 fps_cnt = 0;
	static Uint32 cur_time;
 	static Uint32 prev_time = 0;
	
	cur_time = SDL_GetTicks();
	if (cur_time - prev_time > 1000)
	{
		prev_time = cur_time;
		global.fps = fps_cnt;
		/* debug("fps: %u \n", fps_cnt); */
		fps_cnt = 0;
	}
	else
	{
		fps_cnt++;
	}
}

/*
  1.calculate how much time frame is rendering
  2.check is it lower than TIME_PER_FRAME
  3.sleep the rest of the time if its lower.
*/
void stall_fps_control(unsigned int frame_render_time)
{
	if (frame_render_time < TIME_PER_FRAME)
	{
		SDL_Delay( TIME_PER_FRAME - frame_render_time);	
	}
/*
	else if (frame_render_time > TIME_PER_FRAME)
		debug ("error: frame_render_time %u \n", frame_render_time);
*/
}

void stall_engine()
{
	Uint32 cur_time = SDL_GetTicks();
	global.frame_start = cur_time;

	switch (global.state)
	{
		case STATE_MENU:
		{
			render_background();
			render_logo();
			render_menu();
			render_hi_score();
			break;	
		}

		case STATE_WAVESTART:
		{	
			render_background();
			render_label_wave();
			if ((cur_time - global.wavestart_time) > TIME_SHOW_WAVE_LABEL)
			{
				/* clear all enemies, bullets and dead parts */
				stall_objects_clear();
				/* save start_time of the wave */
				waves[global.wave].start_time = cur_time;
				global.state = STATE_MAIN;
				debug("switched to STATE_MAIN \n");
			}
			break;	
		}

		case STATE_MAIN:
		{	
			stall_player_move();
			ai_cycle();
			stall_bullets_move();
			dead_part_move();
			wave_live();
			bonus_creator();
			bonus_disappear();
			bonus_defshield_blink();
			bonus_check();
			render_background();
			render_interface();
			render_bonuses();
			render_defshield();
			render_ram();
			render_player();
			render_enemies();
			render_dead_parts();
			render_bullets();
			stall_enemy_shot();
			stall_check_crash();
			dead_part_collision();
			stall_check_kill();
			dead_part_destroy();
			if (!player_ship->alive)
				render_label_lose();
			break;	
		}

		case STATE_WAVEEND:
		{	
			stall_player_move();
			stall_bullets_move();
			dead_part_move();
			render_background();
			render_interface();
			render_bonuses();
			render_defshield();
			render_ram();
			render_player();
			render_enemies();
			render_dead_parts();
			render_bullets();
			dead_part_destroy();

			if (cur_time - global.waveend_time >= TIME_WAVE_END)
			{
				/* remove all bullets */
				memset(player_bullet, 0x0, sizeof(bullet_t)*PLAYER_BULLETS);
				memset(enemy_bullet, 0x0, sizeof(bullet_t)*ENEMY_BULLETS);
				global.enemy_bullets = 0;
				global.state = STATE_WAVECLEARED;
				global.wavecleared_time = SDL_GetTicks(); 
			}
			break;	
		}

		case STATE_WAVECLEARED:
		{
			render_background();
			render_label_wave_cleared();
			if ((cur_time - global.wavecleared_time) > TIME_SHOW_WAVE_LABEL)
			{
				global.state = STATE_SCORE;
				global.wavescore_time = SDL_GetTicks();
				debug("switched to the STATE_SCORE \n");
			}			
			break;
		}

		case STATE_SCORE:
		{
			render_background();
			render_score();
			if ((cur_time - global.wavescore_time) > TIME_SHOW_SCORE)
			{
				wave_inc();
				global.state = STATE_WAVESTART;
				global.wavestart_time = SDL_GetTicks();
				debug("switched to the STATE_WAVESTART \n");
			}		
		}

		default:
			break;
	}

	stall_fps_counter();

#ifdef SHOW_FPS
	render_fps();
#endif

	SDL_Flip(global.screen);

	global.frame_end = SDL_GetTicks();
	stall_fps_control(global.frame_end - global.frame_start);
}

/* clear enemies, bullets, dead parts */
void stall_objects_clear()
{
	memset(enemy_ship, 0x0, sizeof(enemy_ship_t)*MAX_ENEMIES);
	memset(player_bullet, 0x0, sizeof(bullet_t)*PLAYER_BULLETS);
	memset(enemy_bullet, 0x0, sizeof(bullet_t)*ENEMY_BULLETS);
	memset(dead_parts, 0x0, sizeof(bullet_t)*TOTAL_DEAD_PARTS);
}

/* objects like player_ship, enemy ships, bullets etc. reused in reset */
void stall_objects_init()
{
	stall_objects_clear();
	stall_player_ship_creator();

	global.state = STATE_MENU;
	global.focus = FOCUS_START;
	global.wave = 0; 
	global.phase = 0;
	global.enemies_alive = 0;
	global.enemy_bullets = 0;
	global.score = 0;
	global.fps = 0;
	global.prev_time = time(0);
	global.wavestart_time = 0;
	global.waveend_time = 0;
	global.wavecleared_time = 0;
	global.wavescore_time = 0;
}

void bonus_init()
{
	bonus[0].scr_duration = 15;
	bonus[0].plr_duration = 20;
	bonus[0].name = "armor breaker";
	bonus[0].active = 0;
	bonus[0].on_player = 0;
	bonus[0].chance[0] = 5; // 5% every sec for first wave 
	bonus[0].chance[1] = 3; // 3% every sec for second wave
		
	bonus[1].scr_duration = 20;
	bonus[1].plr_duration = 20;
	bonus[1].name = "triple shot";
	bonus[1].active = 0;
	bonus[1].on_player = 0;
	bonus[1].chance[0] = 5; // 5% every sec for first wave 
	bonus[1].chance[1] = 3; // 3% every sec for second wave

	bonus[2].scr_duration = 20;
	bonus[2].plr_duration = 15;
	bonus[2].name = "def shield";
	bonus[2].active = 0;
	bonus[2].on_player = 1;
	bonus[2].chance[0] = 5; // 5% every sec for first wave 
	bonus[2].chance[1] = 3; // 3% every sec for second wave

	//XXX - hack for test purpose ;)
	bonus_defshield_create(); 
	bonus[2].plr_lifetime = bonus[2].plr_duration;

	bonus[3].scr_duration = 15;
	bonus[3].plr_duration = 0;
	bonus[3].name = "random bomb";
	bonus[3].active = 0;
	bonus[3].on_player = 0;
	bonus[3].chance[0] = 5; // 5% every sec for first wave 
	bonus[3].chance[1] = 3; // 3% every sec for second wave
		
	bonus[4].scr_duration = 20;
	bonus[4].plr_duration = 20;
	bonus[4].name = "ram";
	bonus[4].active = 0;
	bonus[4].on_player = 0;
	bonus[4].chance[0] = 5; // 5% every sec for first wave 
	bonus[4].chance[1] = 3; // 3% every sec for second wave

	bonus[5].scr_duration = 20;
	bonus[5].plr_duration = 20;
	bonus[5].name = "auto aim missiles";
	bonus[5].active = 0;
	bonus[5].on_player = 0;
	bonus[5].chance[0] = 5; // 5% every sec for first wave 
	bonus[5].chance[1] = 3; // 3% every sec for second wave

	bonus[6].scr_duration = 20;
	bonus[6].plr_duration = 20;
	bonus[6].name = "acceleration";
	bonus[6].active = 0;
	bonus[6].on_player = 0;
	bonus[6].chance[0] = 5; // 5% every sec for first wave 
	bonus[6].chance[1] = 3; // 3% every sec for second wave
}

void waves_init()
{
	int i,j;
	
	waves[0].enemy_speed = 2;
	waves[0].enemy_width = 52;
	waves[0].enemy_height = 53;
	waves[0].dead_parts_num = 3;
	waves[0].phases[0][0] = 10;
	waves[0].phases[0][1] = 2000;
	waves[0].phases[1][0] = 10;
	waves[0].phases[1][1] = 1000;
	waves[0].phases[2][0] = 10;
	waves[0].phases[2][1] = 700;
	waves[0].phases[3][0] = 10;
	waves[0].phases[3][1] = 500;
	waves[0].phases[4][0] = 10;
	waves[0].phases[4][1] = 200;
	waves[0].phases[5][0] = 10;
	waves[0].phases[5][1] = 100;
	waves[0].player_bullets_color = 0xFF0000FF;

	waves[1].enemy_speed = 3;
	waves[1].enemy_width = 48;
	waves[1].enemy_height = 32;
	waves[1].dead_parts_num = 4;	
	waves[1].phases[0][0] = 10;
	waves[1].phases[0][1] = 3000;
	waves[1].phases[1][0] = 10;
	waves[1].phases[1][1] = 2500;
	waves[1].phases[2][0] = 10;
	waves[1].phases[2][1] = 2300;
	waves[1].phases[3][0] = 10;
	waves[1].phases[3][1] = 2200;
	waves[1].phases[4][0] = 10;
	waves[1].phases[4][1] = 2100;
	waves[1].phases[5][0] = 10;
	waves[1].phases[5][1] = 1500;
	waves[1].player_bullets_color = 0x00FF00FF;

	waves[2].enemy_speed = 5;
	waves[2].enemy_width = 48;		//XXX - change accordingly
	waves[2].enemy_height = 32;		//XXX - change accordingly
	waves[2].dead_parts_num = 2;	
	waves[2].phases[0][0] = 10;
	waves[2].phases[0][1] = 3000;
	waves[2].phases[1][0] = 10;
	waves[2].phases[1][1] = 2500;
	waves[2].phases[2][0] = 10;
	waves[2].phases[2][1] = 2300;
	waves[2].phases[3][0] = 10;
	waves[2].phases[3][1] = 2200;
	waves[2].phases[4][0] = 10;
	waves[2].phases[4][1] = 2100;
	waves[2].phases[5][0] = 10;
	waves[2].phases[5][1] = 1500;
	waves[2].player_bullets_color = 0xFF0000FF;

	/* duration of wave is sum of all it phases */
	for (i = 0; i < WAVE_NUM; i++)
		for (j = 0; j < WAVE_PHASES; j++)
			waves[i].duration += waves[i].phases[j][0] * 1000;
}

int stall_init()
{
	int rv = 0;
	srand(time(0));
	player_ship = malloc(sizeof(player_ship_t));
	if (!player_ship)
	{	
		rv = 1;
		goto err;
	}

	stall_objects_init();
	bonus_init();
	waves_init();
	
	/* we should keep hi_score during whole game */
	global.hi_score = 0;

	rv = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER);
	if (rv)
		goto err;
	global.screen = SDL_SetVideoMode(WINDOW_WIDTH,WINDOW_HEIGHT,16,SDL_SWSURFACE | SDL_DOUBLEBUF);
	if (!global.screen)
		goto err;
 	SDL_WM_SetCaption("stall", "stall");
	SDL_ShowCursor(SDL_DISABLE);

 	if(TTF_Init()) 
	{
		rv = -1;
		goto err;
	}
	font[0] = TTF_OpenFont("courbd.ttf",12);
	font[1] = TTF_OpenFont("courbd.ttf",14);
	font[2] = TTF_OpenFont("courbd.ttf",16);
	font[3] = TTF_OpenFont("courbd.ttf",18);
	font[4] = TTF_OpenFont("courbd.ttf",20);
	if (!font)
	{
		rv = 2;
		goto err;
	}

	load_pictures();

	return rv;
err:
	error("stall_init(): %d \n", rv);
	stall_quit();	
}

/* save hi-score, reset this game and start a new one */
void stall_reset()
{
	if (global.score > global.hi_score)
		global.hi_score = global.score;
	global.state = STATE_MENU;
	stall_objects_init();
	bonus_init();
}


int main(int argc, char *argv[])
{
	debug("stall started. version %s \n", version);
	stall_init();

	while (1)
	{
		stall_keyboard();
		stall_engine();
	}

	return 0;
}

void stall_quit()
{
	free(player_ship);
	debug("stall_quit()\n");
	TTF_CloseFont(font[0]);
	TTF_CloseFont(font[1]);
	TTF_Quit();
	SDL_Quit();
	exit(0);
}
