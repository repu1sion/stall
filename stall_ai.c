/* there is everything ai-related */

#include <SDL.h>
#include "stall_types.h"
#include "stall_ini.h"

extern global_t global;
extern wave_t waves[WAVE_NUM];
extern enemy_ship_t enemy_ship[MAX_ENEMIES];
extern player_ship_t *player_ship; 

/* -------------------- AI API --------------------- */

/* distance. used as return values in ai_look_at_player() */
static int dist_x, dist_y;
static int goal_x, goal_y;

/* return destination between ai and player */
static void ai_look_at_player(int i)
{
	dist_x = player_ship->crd.x1 - enemy_ship[i].crd.x1;
	dist_y = player_ship->crd.y1 - enemy_ship[i].crd.y1;
}

/* move to the goal coords. check if goal reached */
static void ai_move(int i)
{	

	switch(enemy_ship[i].move)
	{
		case MOVE_RIGHT:
		{
			enemy_ship[i].crd.x1 += enemy_ship[i].speed;
			enemy_ship[i].crd.x2 += enemy_ship[i].speed;
			break;
		}
		case MOVE_LEFT:
		{
			enemy_ship[i].crd.x1 -= enemy_ship[i].speed;
			enemy_ship[i].crd.x2 -= enemy_ship[i].speed;
			break;
		}
		case MOVE_DOWN:
		{
			enemy_ship[i].crd.y1 += enemy_ship[i].speed;
			enemy_ship[i].crd.y2 += enemy_ship[i].speed;
			break;
		}
		case MOVE_KAMIKAZE: //move on both x and y axis at same time
		{
			if (enemy_ship[i].crd.x1 > player_ship->crd.x1)
			{
				enemy_ship[i].crd.x1 -= enemy_ship[i].speed;
				enemy_ship[i].crd.x2 -= enemy_ship[i].speed;
			}
			else
			{
				enemy_ship[i].crd.x1 += enemy_ship[i].speed;
				enemy_ship[i].crd.x2 += enemy_ship[i].speed;
			}
			enemy_ship[i].crd.y1 += enemy_ship[i].speed;
			enemy_ship[i].crd.y2 += enemy_ship[i].speed;
		}

		default:
			break;
	}



//	if (enemy_ship[i].x1 != enemy_ship[i].goal_x)
	
/*
	if (enemy_ship[i].y1 != enemy_ship[i].goal_y)
	{
		enemy_ship[i].y1 += enemy_ship[i].speed;
		enemy_ship[i].y2 += enemy_ship[i].speed;
	}
*/	
	

#if 0
	if (i==0) debug("x1 %d, y1 %d , goal_x %d, goal_y %d \n", 
		enemy_ship[i].crd.x1, enemy_ship[i].crd.y1, enemy_ship[i].goal_x, enemy_ship[i].goal_y);
#endif
				

	/* if goal reached - remove current action */
	if (enemy_ship[i].crd.x1 < enemy_ship[i].goal_x + 3 &&
	    enemy_ship[i].crd.x1 > enemy_ship[i].goal_x - 3 &&
	    enemy_ship[i].crd.y1 < enemy_ship[i].goal_y + 3 &&
	    enemy_ship[i].crd.y1 > enemy_ship[i].goal_y - 3 )
	{
		enemy_ship[i].in_action = 0;
		//if (i==0) debug ("stop action \n");
	}
}

static void ai_move_direction(int i)
{
	switch(enemy_ship[i].move)
	{
		case MOVE_DONT: // now same as MOVE_LEFT
		{
			enemy_ship[i].move = MOVE_RIGHT;
			goal_x = rand() % (GAME_WIDTH - waves[global.wave].enemy_width - enemy_ship[i].crd.x1);
			enemy_ship[i].goal_x = enemy_ship[i].crd.x1 + goal_x;
			enemy_ship[i].goal_y = enemy_ship[i].crd.y1;	
			break;
		}	
		case MOVE_RIGHT:
		{
			enemy_ship[i].move = MOVE_LEFT;
			goal_x = rand() % enemy_ship[i].crd.x1;
			enemy_ship[i].goal_x = goal_x;
			enemy_ship[i].goal_y = enemy_ship[i].crd.y1;	
			break;
		}
		case MOVE_LEFT:
		{
			enemy_ship[i].move = MOVE_RIGHT;
			goal_x = rand() % (GAME_WIDTH - waves[global.wave].enemy_width - enemy_ship[i].crd.x1);
			enemy_ship[i].goal_x = enemy_ship[i].crd.x1 + goal_x;
			enemy_ship[i].goal_y = enemy_ship[i].crd.y1;	
			break;
		}

		case MOVE_DOWN: /* MOVE_RIGHT or MOVE_LEFT will be set next time */
		{
			if (rand() % 2)
			{
				enemy_ship[i].move = MOVE_LEFT;  //TODO - same code as above
				goal_x = rand() % enemy_ship[i].crd.x1;
				enemy_ship[i].goal_x = goal_x;
				enemy_ship[i].goal_y = enemy_ship[i].crd.y1;			
			}
			else
			{	
				enemy_ship[i].move = MOVE_RIGHT;  //TODO - same code as above
				goal_x = rand() % (GAME_WIDTH - waves[global.wave].enemy_width - enemy_ship[i].crd.x1);
				enemy_ship[i].goal_x = enemy_ship[i].crd.x1 + goal_x;
				enemy_ship[i].goal_y = enemy_ship[i].crd.y1;	
			}
			break;
		}


		default:
			break;
	}
	//debug("ai_mov_direction. i %d ,goal_x %d, move %d \n",i, goal_x, enemy_ship[i].move);
}

static void ai_new_move(int i)
{
	switch (enemy_ship[i].mental) //XXX - add 2 more mentalities here
	{
		case MENTAL_AGGRESSIVE:
		{
			goal_x = player_ship->crd.x1;
			goal_y = player_ship->crd.y1;
			enemy_ship[i].goal_x = player_ship->crd.x1;
			enemy_ship[i].goal_y = player_ship->crd.y1;
			enemy_ship[i].move = MOVE_KAMIKAZE;
			break;
		}	
		case MENTAL_MIDDLE:
		{
			ai_move_direction(i);
			break;
		}
		default:
		{
			ai_move_direction(i);
			break;
		}
	}
	/* now ai has action to do */
	enemy_ship[i].in_action = 1;

	//debug("ai_new_move. i %d, goal_x %d \n", i, enemy_ship[i].goal_x);
}

/* this is the most important function.
   ai has to make a decision what to do next */
static void ai_decision(int i)
{
	if (enemy_ship[i].in_action) /* already have action to do */
	{
		ai_move(i);
		return;
	}
	if (!enemy_ship[i].in_action)
	{
		ai_look_at_player(i); //XXX - not used
		ai_new_move(i);
		ai_move(i);
	}
}

/* main ai cycle. every enemy must choose decision */
void ai_cycle()
{
	int i;
	for (i = 0; i < MAX_ENEMIES; i++)
	{
		if (enemy_ship[i].alive)
			ai_decision(i);
	}
}
