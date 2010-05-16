/* 
 *
 *   Copyright (c) 2010 Arthur Huillet
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


#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

void *get_obstacle_extension(level *lvl, int index, enum obstacle_extension_type type)
{
	int i;

	for (i = 0; i < lvl->obstacle_extensions_size; i++) {
		if (lvl->obstacle_extensions[i].index == index) 
			if (lvl->obstacle_extensions[i].type == type)
				return lvl->obstacle_extensions[i].data;
	}

	return NULL;
}

void del_obstacle_extensions(level *lvl, int index)
{
}

