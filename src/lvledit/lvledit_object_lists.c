/* 
 *
 *   Copyright (c) 2009-2011 Arthur Huillet
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

#define _leveleditor_tile_lists_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_object_lists.h"

static int floor_tiles_array[] = {

	ISO_SIDEWALK_0,

	ISO_SIDEWALK_7,
	ISO_SIDEWALK_5,
	ISO_SIDEWALK_6,
	ISO_SIDEWALK_8,

	ISO_SIDEWALK_11,
	ISO_SIDEWALK_9,
	ISO_SIDEWALK_10,
	ISO_SIDEWALK_12,

	ISO_SIDEWALK_3,
	ISO_SIDEWALK_1,
	ISO_SIDEWALK_2,
	ISO_SIDEWALK_4,

	ISO_SIDEWALK_18,
	ISO_SIDEWALK_19,
	ISO_SIDEWALK_20,
	ISO_SIDEWALK_17,

	ISO_SIDEWALK_15,
	ISO_SIDEWALK_16,
	ISO_SIDEWALK_13,
	ISO_SIDEWALK_14,

	ISO_SIDEWALK_21,
	ISO_SIDEWALK_22,
	ISO_SIDEWALK_23,
	ISO_SIDEWALK_24,
	ISO_WATER_SIDEWALK_01,
	ISO_WATER_SIDEWALK_02,
	ISO_WATER_SIDEWALK_03,
	ISO_WATER_SIDEWALK_04,

	ISO_FLOOR_STONE_FLOOR,
	ISO_FLOOR_STONE_FLOOR_WITH_DOT,
	ISO_FLOOR_STONE_FLOOR_WITH_GRATE,

	ISO_MISCELLANEOUS_FLOOR_14,
	ISO_MISCELLANEOUS_FLOOR_10,
	ISO_MISCELLANEOUS_FLOOR_11,
	ISO_MISCELLANEOUS_FLOOR_12,
	ISO_MISCELLANEOUS_FLOOR_13,

	ISO_MISCELLANEOUS_FLOOR_23,
	ISO_MISCELLANEOUS_FLOOR_17,
	ISO_MISCELLANEOUS_FLOOR_18,
	ISO_MISCELLANEOUS_FLOOR_15,
	ISO_MISCELLANEOUS_FLOOR_16,

	ISO_WATER,
	ISO_WATER_EDGE_1,
	ISO_WATER_EDGE_2,
	ISO_WATER_EDGE_3,
	ISO_WATER_EDGE_4,
	ISO_WATER_EDGE_5,
	ISO_WATER_EDGE_6,
	ISO_WATER_EDGE_7,
	ISO_WATER_EDGE_8,
	ISO_WATER_EDGE_9,
	ISO_WATER_EDGE_10,
	ISO_WATER_EDGE_11,
	ISO_WATER_EDGE_12,
	ISO_WATER_EDGE_13,
	ISO_WATER_EDGE_14,

	ISO_MISCELLANEOUS_FLOOR_9,
	ISO_COMPLETELY_DARK,
	ISO_FLOOR_ERROR_TILE,
	ISO_RED_WAREHOUSE_FLOOR,
	ISO_MISCELLANEOUS_FLOOR_21,
	ISO_MISCELLANEOUS_FLOOR_22,
	ISO_FLOOR_HOUSE_FLOOR,
	ISO_MISCELLANEOUS_FLOOR_19,
	ISO_MISCELLANEOUS_FLOOR_20,

	ISO_SAND_FLOOR_4,
	ISO_SAND_FLOOR_5,
	ISO_SAND_FLOOR_6,
	ISO_FLOOR_SAND,
	ISO_SAND_FLOOR_1,
	ISO_SAND_FLOOR_2,
	ISO_SAND_FLOOR_3,

	ISO_FLOOR_SAND_WITH_GRASS_14,
	ISO_FLOOR_SAND_WITH_GRASS_15,
	ISO_FLOOR_SAND_WITH_GRASS_16,
	ISO_FLOOR_SAND_WITH_GRASS_17,

	ISO_FLOOR_SAND_WITH_GRASS_1,
	ISO_FLOOR_SAND_WITH_GRASS_2,
	ISO_FLOOR_SAND_WITH_GRASS_3,
	ISO_FLOOR_SAND_WITH_GRASS_4,
	ISO_FLOOR_SAND_WITH_GRASS_5,
	ISO_FLOOR_SAND_WITH_GRASS_25,
	ISO_FLOOR_SAND_WITH_GRASS_26,
	ISO_FLOOR_SAND_WITH_GRASS_27,

	ISO_FLOOR_SAND_WITH_GRASS_20,
	ISO_FLOOR_SAND_WITH_GRASS_19,
	ISO_FLOOR_SAND_WITH_GRASS_18,
	ISO_FLOOR_SAND_WITH_GRASS_21,

	ISO_FLOOR_SAND_WITH_GRASS_6,
	ISO_FLOOR_SAND_WITH_GRASS_7,
	ISO_FLOOR_SAND_WITH_GRASS_8,
	ISO_FLOOR_SAND_WITH_GRASS_9,
	ISO_FLOOR_SAND_WITH_GRASS_10,
	ISO_FLOOR_SAND_WITH_GRASS_11,
	ISO_FLOOR_SAND_WITH_GRASS_12,
	ISO_FLOOR_SAND_WITH_GRASS_13,

	ISO_FLOOR_SAND_WITH_GRASS_22,
	ISO_FLOOR_SAND_WITH_GRASS_23,
	ISO_FLOOR_SAND_WITH_GRASS_24,
	ISO_FLOOR_SAND_WITH_GRASS_28,
	ISO_FLOOR_SAND_WITH_GRASS_29,

	ISO_CARPET_TILE_0001,
	ISO_CARPET_TILE_0002,
	ISO_CARPET_TILE_0003,
	ISO_CARPET_TILE_0004,
	ISO_LARGE_SQUARE_BBB,
	ISO_LARGE_SQUARE_BRB,
	ISO_LARGE_SQUARE_BRR,
	ISO_LARGE_SQUARE_GBB,
	ISO_LARGE_SQUARE_GRB,
	ISO_LARGE_SQUARE_GRR,
	ISO_LARGE_SQUARE_RBB,
	ISO_LARGE_SQUARE_RRB,
	ISO_LARGE_SQUARE_RRR,
	ISO_MINI_SQUARE_0001,
	ISO_MINI_SQUARE_0002,
	ISO_MINI_SQUARE_0003,
	ISO_MINI_SQUARE_0004,
	ISO_MINI_SQUARE_0005,
	ISO_MINI_SQUARE_0006,
	ISO_MINI_SQUARE_0007,
	ISO_MINI_SQUARE_0008,
	ISO_SQUARE_TILE_AAB,
	ISO_SQUARE_TILE_ACB2,
	ISO_SQUARE_TILE_ACB,
	ISO_SQUARE_TILE_ADB2,
	ISO_SQUARE_TILE_ADB,
	ISO_SQUARE_TILE_CAB2,
	ISO_SQUARE_TILE_CAB,
	ISO_SQUARE_TILE_CCB,
	ISO_SQUARE_TILE_DAB2,
	ISO_SQUARE_TILE_DAB,
	ISO_SQUARE_TILE_DDB,

	ISO_COMPLICATED_CMM,
	ISO_COMPLICATED_CMM2,
	ISO_COMPLICATED_P4,
	ISO_COMPLICATED_PMG,
	ISO_COMPLICATED_PMG2,
	ISO_COMPLICATED_PMM,

	ISO_TWOSQUARE_0001,
	ISO_TWOSQUARE_0002,
	ISO_TWOSQUARE_0003,
	-1
};

static int waypoint_array[] = {
	0,			//random spawn
	1,			//no random spawn 
	-1
};

int *floor_tiles_list = floor_tiles_array;
int *wall_tiles_list = NULL;
int *machinery_tiles_list = NULL;
int *furniture_tiles_list = NULL;
int *container_tiles_list = NULL;
int *plant_tiles_list = NULL;
int *misc_tiles_list = NULL;
int *waypoint_list = waypoint_array;

int *sidewalk_floor_list;
int *water_floor_list;
int *grass_floor_list;
int *square_floor_list;
int *other_floor_list;
	
int *melee_items_list;
int *gun_items_list;
int *defense_items_list;
int *spell_items_list;
int *other_items_list;
int *all_items_list;

void lvledit_set_obstacle_list_for_category(const char *category_name, struct dynarray *obstacles_filenames)
{
	const struct {
		const char *name;
		int **ptr;
	} obstacle_lists[] = {
		{ "WALL", &wall_tiles_list },
		{ "MACHINERY", &machinery_tiles_list },
		{ "FURNITURE", &furniture_tiles_list },
		{ "CONTAINER", &container_tiles_list },
		{ "PLANT", &plant_tiles_list },
		{ "OTHER", &misc_tiles_list }
	};

	int i, j;
	int *idx_list = MyMalloc(sizeof(int) * (obstacles_filenames->size + 1));

	// Translate obstacle filenames to obstacle indices
	int idx = 0;
	for (i = 0; i < obstacles_filenames->size; i++) {
		const char *filename = ((const char **)obstacles_filenames->arr)[i];
		for (j = 0; j < obstacle_map.size; j++) {
			if (!strcmp(filename, ((obstacle_spec *)obstacle_map.arr)[j].filename)) {
				idx_list[idx++] = j;
				break;
			}
		}
	}
	idx_list[idx] = -1;

	for (i = 0; i < sizeof(obstacle_lists) / sizeof(obstacle_lists[0]); i++) {
		if (!strcmp(category_name, obstacle_lists[i].name)) {
			*obstacle_lists[i].ptr = idx_list;
			return;
		}
	}

	ErrorMessage(__FUNCTION__, "Unknown obstacle category: %s.", PLEASE_INFORM, IS_WARNING_ONLY, category_name);
}

static void build_floor_tile_lists(void)
{
	int i;
	int sidewalk = 0;
	int water    = 0;
	int grass    = 0;
	int square   = 0;
	int other    = 0;

	free(sidewalk_floor_list); //Sidewalk Tiles
	free(water_floor_list);    //Water Tiles
	free(grass_floor_list);    //Grass Tiles
	free(square_floor_list);   //Square Tiles - Geometric Patterned
	free(other_floor_list);    //OTHER: Dirt, Sand, Carpet, Misc, etc.

	sidewalk_floor_list = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	water_floor_list    = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	grass_floor_list    = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	square_floor_list   = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));
	other_floor_list    = MyMalloc((ALL_ISOMETRIC_FLOOR_TILES) * sizeof(int));

	for (i = 0; i < ALL_ISOMETRIC_FLOOR_TILES; i++) {
		if (strstr(floor_tile_filenames[i], "sidewalk")) {
			sidewalk_floor_list[sidewalk] = i;
			sidewalk++;
			if (strstr(floor_tile_filenames[i], "water")) { //Water-Sidewalk tiles should be in both
				water_floor_list[water] = i;
				water++;
			}
		} else if (strstr(floor_tile_filenames[i], "water")) {
			water_floor_list[water] = i;
			water++;
		} else if (strstr(floor_tile_filenames[i], "grass")) {
			grass_floor_list[grass] = i;
			grass++;
		} else if (strstr(floor_tile_filenames[i], "square")) {
			square_floor_list[square] = i;
			square++;
		} else {
			other_floor_list[other] = i;
			other++;
		}
	}

	sidewalk_floor_list[sidewalk] = -1;
	water_floor_list[water]       = -1;
	grass_floor_list[grass]       = -1;
	square_floor_list[square]     = -1;
	other_floor_list[other]       = -1;

}

static void build_item_lists(void)
{
	int i;
	int melee   = 0;
	int guns    = 0;
	int defense = 0;
	int spell   = 0;
	int other       = 0;

	free(melee_items_list);  //MELEE WEAPONS
	free(gun_items_list);    //GUNS
	free(defense_items_list);//ARMOR, SHIELDS, & SHOES
	free(spell_items_list);  //SPELL-LIKE: Grenades, Spell Books, Pills, & Potions
	free(other_items_list);  //OTHER: Ammo, Circuts, Plot Items, etc.
	free(all_items_list);    //EVERYTHING

	melee_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	gun_items_list     = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	defense_items_list = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	spell_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	other_items_list   = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));
	all_items_list     = MyMalloc((Number_Of_Item_Types + 1) * sizeof(int));

	for (i = 1; i < Number_Of_Item_Types; i++) { //Start at 1 to skip the CTD item
		if (ItemMap[i].item_weapon_is_melee) {
			melee_items_list[melee] = i;
			melee++;
		} else if (ItemMap[i].item_can_be_installed_in_weapon_slot) {
			gun_items_list[guns] = i;
			guns++;
		} else if (ItemMap[i].item_can_be_installed_in_drive_slot || ItemMap[i].item_can_be_installed_in_armour_slot || 
				ItemMap[i].item_can_be_installed_in_shield_slot || ItemMap[i].item_can_be_installed_in_special_slot) {
			defense_items_list[defense] = i;
			defense++;
		} else if (ItemMap[i].item_combat_use_description) {
			spell_items_list[spell] = i;
			spell++;
		} else {
			other_items_list[other] = i;
			other++;
		}
		all_items_list[i] = i;
	}
	melee_items_list[melee]     = -1;
	gun_items_list[guns]        = -1;
	defense_items_list[defense] = -1;
	spell_items_list[spell]     = -1;
	other_items_list[other]     = -1;
	all_items_list[i]           = -1;

}

/**
 * This function builds all the lists of objects belonging to the various categories.
 */
void lvledit_build_tile_lists(void)
{
	build_floor_tile_lists();
	build_item_lists();
}
