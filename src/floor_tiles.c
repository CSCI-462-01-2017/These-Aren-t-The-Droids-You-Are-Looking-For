/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
 *
 *
 *  This file is part of Freedroid
 *
 *  Freedroid is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  Freedroid is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Freedroid; see the file COPYING. If not, write to the 
 *  Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 *
 */

#define _floor_tiles_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "proto.h"
#include "global.h"

/**
 * This files contains floor tiles related functions.
 */

/**
 * This function creates the data structure used by the collision detection system and the rendering system.
 * Each obstacle covers a certain number of floor tiles: a reference to the obstacle from the 
 * floor tiles in question is created. This operation is similar to physically applying glue on the obstacles
 * to make them fit on the tiles.
 * An obstacle is glued to any tile it covers even partially. One obstacle is glued to 1 or more floor tiles.
 * A floor tile might have no obstacles glued onto it.
 */
void glue_obstacles_to_floor_tiles_for_level(int level_num)
{
	level *lvl = curShip.AllLevels[level_num];
	int obstacle_counter;
	int x_min, x_max, x;
	int y_min, y_max, y;

	// Clear out all the existing glue information
	free_glued_obstacles(lvl);

	// Browse obstacles on the level to glue them
	for (obstacle_counter = 0; obstacle_counter < MAX_OBSTACLES_ON_MAP; obstacle_counter++) {

		obstacle *o = &lvl->obstacle_list[obstacle_counter];

		if (o->type == -1)
			continue;

		obstacle_spec *spec = get_obstacle_spec(o->type);
		x_min = floor(o->pos.x + spec->left_border);
		x_max = floor(o->pos.x + spec->right_border);
		y_min = floor(o->pos.y + spec->upper_border);
		y_max = floor(o->pos.y + spec->lower_border);

		for (x = x_min; x <= x_max; x++) {
			for (y = y_min; y <= y_max; y++) {
				if (x < 0 || y < 0 || x >= lvl->xlen || y >= lvl->ylen)
					continue;

				dynarray_add(&lvl->map[y][x].glued_obstacles, &obstacle_counter, sizeof(obstacle_counter));
			}
		}
	}
}

/**
 * In order to make sure that an obstacle is only displayed/checked once, the collision detection and display code use a timestamp.
 * Every time a new collision detection is started, and every time a new frame is displayed, the timestamp is increased.
 * Obstacles with the same timestamp are obstacles that have already been checked.
 */
int next_glue_timestamp(void)
{
	static int glue_timestamp = 0;

	glue_timestamp++;

	if (glue_timestamp == 0) {
		ErrorMessage(__FUNCTION__, "The glue timestamp overflowed. This might not be properly handled.\n", PLEASE_INFORM, IS_WARNING_ONLY);
	}

	return glue_timestamp;
}

void free_glued_obstacles(level *lvl)
{
	int x, y;

	for (x = 0; x < lvl->xlen; x++) {
		for (y = 0; y < lvl->ylen; y++) {
			dynarray_free(&lvl->map[y][x].glued_obstacles);
		}
	}
}

#undef _floor_tiles_c
