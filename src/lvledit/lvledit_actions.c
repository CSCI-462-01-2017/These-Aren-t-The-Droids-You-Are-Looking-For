/* 
 *
 *   Copyright (c) 2004-2010 Arthur Huillet
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

/**
 * This file contains all the actions performed by the level editor, ie. the functions that act on the level.
 */

#define _leveleditor_actions_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "SDL_rotozoom.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_widgets.h"

/* Undo/redo action lists */
LIST_HEAD(to_undo);
LIST_HEAD(to_redo);

static int push_mode = NORMAL;

/**
 *  @fn void clear_action(action * pos)
 *
 *  @brief clears an action from its list, and pointers held within the action
 */
static void clear_action(action * action)
{
	if (action->type == ACT_SET_OBSTACLE_LABEL && action->d.change_obstacle_name.new_name != NULL)
		free(action->d.change_obstacle_name.new_name);
	else if (action->type == ACT_SET_MAP_LABEL && action->d.change_label_name.new_name != NULL)
		free(action->d.change_label_name.new_name);

	list_del(&action->node);	//< removes an action from a list
	free(action);		//< free's the action
}

/**
 *  @fn void clear_action_list(struct list_head *list)
 *
 *  @brief clears an action list, and all its data
 */
static void clear_action_list(struct list_head *list)
{
	// free actions individually
	action *pos, *next;
	list_for_each_entry_safe(pos, next, list, node)
	    clear_action(pos);
}

/**
 *  @fn static void action_freestack(void)
 *
 *  @brief clears to_undo and to_redo when LevelEditor() exits
 */
void action_freestack(void)
{
	clear_action_list(&to_redo);
	clear_action_list(&to_undo);
}

static action *action_create(int type, va_list args)
{
	action *act = malloc(sizeof(action));
	act->type = type;
	switch (type) {
		case ACT_CREATE_OBSTACLE:
			act->d.create_obstacle.x = va_arg(args, double);
			act->d.create_obstacle.y = va_arg(args, double);
			act->d.create_obstacle.new_obstacle_type = va_arg(args, int);
			break;
		case ACT_REMOVE_OBSTACLE:
			act->d.delete_obstacle = va_arg(args, obstacle *);
			break;
		case ACT_MOVE_OBSTACLE:
			act->d.move_obstacle.obstacle = va_arg(args, obstacle *);
			act->d.move_obstacle.newx = va_arg(args, double);
			act->d.move_obstacle.newy = va_arg(args, double);
			break;
		case ACT_CREATE_ITEM:
			act->d.create_item.x = (float)va_arg(args, double);
			act->d.create_item.y = (float)va_arg(args, double);
			act->d.create_item.type = va_arg(args, int);
			break;
		case ACT_REMOVE_ITEM:
			act->d.delete_item = va_arg(args, item *);
			break;
		case ACT_MOVE_ITEM:
			act->d.move_item.item = va_arg(args, item *);
			act->d.move_item.newx = (float)va_arg(args, double);
			act->d.move_item.newy = (float)va_arg(args, double);
			break;
		case ACT_CREATE_WAYPOINT:
			act->d.create_waypoint.x = va_arg(args, int);
			act->d.create_waypoint.y = va_arg(args, int);
			act->d.create_waypoint.suppress_random_spawn = va_arg(args, int);
			break;
		case ACT_REMOVE_WAYPOINT:
			act->d.delete_waypoint.x = va_arg(args, int);
			act->d.delete_waypoint.y = va_arg(args, int);
			break;
		case ACT_TOGGLE_WAYPOINT_RSPAWN:
			act->d.toggle_waypoint_rspawn.x = va_arg(args, int);
			act->d.toggle_waypoint_rspawn.y = va_arg(args, int);
			break;
		case ACT_TOGGLE_WAYPOINT_CONNECTION:
			act->d.toggle_waypoint_connection.x = va_arg(args, int);
			act->d.toggle_waypoint_connection.y = va_arg(args, int);
			break;
		case ACT_TILE_FLOOR_SET:
			act->d.change_floor.x = va_arg(args, int);
			act->d.change_floor.y = va_arg(args, int);
			act->d.change_floor.type = va_arg(args, int);
			break;
		case ACT_MULTIPLE_ACTIONS:
			act->d.number_actions = va_arg(args, int);
			break;
		case ACT_SET_OBSTACLE_LABEL:
			act->d.change_obstacle_name.obstacle = va_arg(args, obstacle *);
			act->d.change_obstacle_name.new_name = va_arg(args, char *);
			break;
		case ACT_SET_MAP_LABEL:
			act->d.change_label_name.id = va_arg(args, int);
			act->d.change_label_name.new_name = va_arg(args, char *);
			break;
		case ACT_JUMP_TO_LEVEL:
			act->d.jump_to_level.target_level = va_arg(args, int);
			act->d.jump_to_level.x = (float)va_arg(args, double);
			act->d.jump_to_level.y = (float)va_arg(args, double);
			break;
		default:
			ErrorMessage(__FUNCTION__, "Unknown action type %d\n", PLEASE_INFORM, IS_FATAL, type);
	}

	return act;
}

void action_push(int type, ...)
{
	va_list args;
	va_start(args, type);
	
	action *act = action_create(type, args);

	switch (push_mode) {
		case UNDO:
			list_add(&act->node, &to_redo);
			break;
		case REDO:
		case NORMAL:
			list_add(&act->node, &to_undo);
			break;
	}

	va_end(args);
}

obstacle *action_create_obstacle(level * EditLevel, double x, double y, int new_obstacle_type)
{
	int i;

	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		if (EditLevel->obstacle_list[i].type == (-1)) {
			EditLevel->obstacle_list[i].type = new_obstacle_type;
			EditLevel->obstacle_list[i].pos.x = x;
			EditLevel->obstacle_list[i].pos.y = y;
			EditLevel->obstacle_list[i].pos.z = EditLevel->levelnum;
			glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);
			// Now that we have disturbed the order of the obstacles on this level, we need
			// to re-assemble the lists of pointers to obstacles, like the door list, the
			// teleporter list and the refreshes list.
			//
			dirty_animated_obstacle_lists(EditLevel->levelnum);
			
			return (&(EditLevel->obstacle_list[i]));
		}
	}

	ErrorMessage(__FUNCTION__, "\
	    Ran out of obstacle positions (%d) in level %d!", PLEASE_INFORM, IS_FATAL, MAX_OBSTACLES_ON_MAP, EditLevel->levelnum);
	return (NULL);
}

void action_move_obstacle(level * EditLevel, obstacle * obs, float newx, float newy)
{
	action_push(ACT_MOVE_OBSTACLE, obs, obs->pos.x, obs->pos.y);
	obs->pos.x = newx;
	obs->pos.y = newy;
	glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);
}

obstacle *action_create_obstacle_user(Level EditLevel, double x, double y, int new_obstacle)
{
	obstacle *o = action_create_obstacle(EditLevel, x, y, new_obstacle);
	if (o) {
		action_push(ACT_REMOVE_OBSTACLE, o);
	}
	return o;
}

/**
 * Change an obstacle label, possibly removing it.
 * @return the number of actions that were pushed on the stack.
 */
static int action_change_obstacle_label(level *EditLevel, obstacle *obstacle, char *name, int undoable)
{
	// If the obstacle already has a label, remove it
	char *old_label = get_obstacle_extension(EditLevel, obstacle, OBSTACLE_EXTENSION_LABEL);
	if (old_label) {
		del_obstacle_extension(EditLevel, obstacle, OBSTACLE_EXTENSION_LABEL);
	}

	// Create the undo action if appropriate
	if (undoable) {
		action_push(ACT_SET_OBSTACLE_LABEL, obstacle, old_label);
	} else {
		free(old_label);
	}

	// If the new label is empty, we are done
	if (!name || !strlen(name))
		return 0;

	name = strdup(name);

	// Assign the new label
	add_obstacle_extension(EditLevel, obstacle, OBSTACLE_EXTENSION_LABEL, name);

	return undoable;
}

void action_change_obstacle_label_user(level *EditLevel, obstacle *our_obstacle, char *predefined_name)
{
	char *name = NULL;

	if (!our_obstacle)
		return;

	if (predefined_name == NULL) {
		char *old_label = get_obstacle_extension(EditLevel, our_obstacle, OBSTACLE_EXTENSION_LABEL);
		name = GetEditableStringInPopupWindow(1000, _("\nPlease enter obstacle label: \n\n"),
						   old_label ? old_label : "");
	} else {
		name = strdup(predefined_name);
	}
	
	if (name) {
		action_change_obstacle_label(EditLevel, our_obstacle, name, 1);
		free(name);
	}
}

/**
 * Remove an obstacle from the map.
 * @param undoable indicates whether the action should be made undoable by pushing 
 * undo actions on the stack.
 */
void action_remove_obstacle(level *EditLevel, obstacle *our_obstacle)
{
	// The likely case that no obstacle was currently marked.
	//
	if (our_obstacle == NULL)
		return;

	our_obstacle->type = (-1);

	// Remove the obstacle label if we had one
	// with the current design this won't be undoable
	action_change_obstacle_label(EditLevel, our_obstacle, NULL, 0);

	// Now doing that must have shifted the glue!  That is a problem.  We need to
	// reglue everything to the map...
	//
	glue_obstacles_to_floor_tiles_for_level(EditLevel->levelnum);

	// Now that we have disturbed the order of the obstacles on this level, we need
	// to re-assemble the lists of pointers to obstacles, like the door list, the
	// teleporter list and the refreshes list.
	//
	dirty_animated_obstacle_lists(EditLevel->levelnum);
}

void action_remove_obstacle_user(Level EditLevel, obstacle * our_obstacle)
{
	/* Save obstacle information for the undo action */
	double posx, posy;
	int type;

	type = our_obstacle->type;
	posx = our_obstacle->pos.x;
	posy = our_obstacle->pos.y;

	// make an undoable removal
	action_remove_obstacle(EditLevel, our_obstacle);
	action_push(ACT_CREATE_OBSTACLE, posx, posy, type);
}

/**
 * Create an item on the map, add this action in the stack undo/redo
 * \param EditLevel Pointer towards the editing level where create the item
 * \param x The x position of the item
 * \param y The y position of the item
 * \param type The type of the item
 * \return The new item created
 */
item *action_create_item(level *EditLevel, float x, float y, int type)
{
	int multiplicity = 1;

	if (ItemMap[type].item_group_together_in_inventory) {
		// Display a popup window with a number selector in order to choose the
		// multiplicity of the item
		multiplicity =
		    do_graphical_number_selection_in_range(1, (!MatchItemWithName(type, "Valuable Circuits")) ? 100 : 1000, 1);
		
		// We always want create an item with a multiplicity higher than 0
		multiplicity = (multiplicity > 0) ? multiplicity : 1;
	}

	// Create an item on the map
	item *it = DropItemAt(type, EditLevel->levelnum, x, y, multiplicity);
	if (it) {
		action_push(ACT_REMOVE_ITEM, it);
	}
	return it;
}

/**
 * Remove an item on the map and add this action in the stack undo/redo
 * \param EditLevel Pointer towards the editing level where delete the item
 * \param it The item which we want deleted
 */
void action_remove_item(level *EditLevel, item *it)
{
	float x, y;
	int type;

	// Save item information for the undo action
	type = it->type;
	x = it->pos.x;
	y = it->pos.y;

	// Remove the item on the map
	DeleteItem(it);

	// Make an undoable removal
	action_push(ACT_CREATE_ITEM, x, y, type);
}

/**
 * Move an item on the map and add this action in stack undo/redo
 * \param EditLevel Pointer towards the editing level where move the item
 * \param it The item which we want moved
 * \param x The new x position of the item
 * \param y The new y position of the item
 */
void action_move_item(level *EditLevel, item *it, float x, float y)
{
	float oldx, oldy;

	// Save item position for the undo action
	oldx = it->pos.x;
	oldy = it->pos.y;

	// Define the new position of the item
	it->pos.x = x;
	it->pos.y = y;

	action_push(ACT_MOVE_ITEM, it, oldx, oldy);
}

/**
 * Create a waypoint on the map, add this action in the undo/redo stack
 * \param EditLevel Pointer towards the currently edited level where create the waypoint
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 * \param random_spawn TRUE if the waypoint can be used to place a random bot
 * \return The new waypoint created
 */
waypoint *action_create_waypoint(level *EditLevel, int x, int y, int random_spawn)
{
	waypoint *wpts = EditLevel->waypoints.arr;
	int wpnum;

	wpnum = get_waypoint(EditLevel, x, y);
	if (wpnum < 0) {
		// When the waypoint doesn't exist on the map, create it
		wpnum = add_waypoint(EditLevel, x, y, random_spawn);

		// The waypoints array may have been moved by the add_waypoint call
		wpts = EditLevel->waypoints.arr;

		// Make an undoable action
		action_push(ACT_REMOVE_WAYPOINT, wpts[wpnum].x, wpts[wpnum].y);
	}

	return &wpts[wpnum];
}

/**
 * Remove a waypoint on the map and add this action in the undo/redo stack
 * \param EditLevel Pointer towards the currently edited level where delete the waypoint
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 */
void action_remove_waypoint(level *EditLevel, int x, int y)
{
	waypoint *wpts = EditLevel->waypoints.arr;

	int wpnum = get_waypoint(EditLevel, x, y);
	if (wpnum < 0) {
		// When the waypoint doesn't exist on the map, we are done
		return;
	}

	// Save waypoint information for the undo action
	int old_random_spawn = wpts[wpnum].suppress_random_spawn;

	// Remove the waypoint on the map
	del_waypoint(EditLevel, x, y);

	// Make an undoable action
	action_push(ACT_CREATE_WAYPOINT, x, y, old_random_spawn);
}

/**
 * Set/unset the flag for random bots and add this action in the undo/redo stack
 * \param EditLevel Pointer towards the currently edited level where toggle the waypoint
 * \param x The x position of the waypoint
 * \param y The y position of the waypoint
 */
void action_toggle_waypoint_randomspawn(level *EditLevel, int x, int y)
{
	waypoint *wpts = EditLevel->waypoints.arr;

	int wpnum = get_waypoint(EditLevel, x, y);
	if (wpnum < 0) {
		return;
	}

	// Toggle the flag for random bots
	wpts[wpnum].suppress_random_spawn = !wpts[wpnum].suppress_random_spawn;

	// Make an undoable action
	action_push(ACT_TOGGLE_WAYPOINT_RSPAWN, x, y);
}

int action_toggle_waypoint_connection(level *EditLevel, int id_origin, int id_target, int removeifpresent, int undoable)
{
	waypoint *wpts = EditLevel->waypoints.arr;
	int *connections;
	int i;

	// Get the connections of the waypoint;
	connections = wpts[id_origin].connections.arr;

	for (i = 0; i < wpts[id_origin].connections.size; i++) {
		if (connections[i] == id_target) {
			if (removeifpresent) {
				// Delete the connection of the waypoint
				dynarray_del(&wpts[id_origin].connections, i, sizeof(int));
			}
			if (undoable)
				action_push(ACT_TOGGLE_WAYPOINT_CONNECTION, id_origin, id_target);
			return -1;
		}
	}

	// Add the target connection of the waypoint
	dynarray_add(&wpts[id_origin].connections, &id_target, sizeof(int));

	if (undoable)
		action_push(ACT_TOGGLE_WAYPOINT_CONNECTION, id_origin, id_target);

	return 1;
}

void level_editor_action_toggle_waypoint_connection_user(level * EditLevel, int xpos, int ypos)
{
	waypoint *wpts;
	int i;

	// Determine which waypoint is currently targeted
	wpts = EditLevel->waypoints.arr;
	for (i = 0; i < EditLevel->waypoints.size; i++) {
		if ((wpts[i].x == xpos) && (wpts[i].y == ypos))
			break;
	}

	if (i == EditLevel->waypoints.size) {
		sprintf(VanishingMessage, _("Sorry, don't know which waypoint you mean."));
		VanishingMessageEndDate = SDL_GetTicks() + 7000;
	} else {
		sprintf(VanishingMessage, _("You specified waypoint nr. %d."), i);
		VanishingMessageEndDate = SDL_GetTicks() + 7000;
		if (OriginWaypoint == (-1)) {
			OriginWaypoint = i;
			strcat(VanishingMessage, _("\nIt has been marked as the origin of the next connection."));
			DebugPrintf(1, "\nWaypoint nr. %d. selected as origin\n", i);
		} else {
			if (OriginWaypoint == i) {
				strcat(VanishingMessage, _("\n\nOrigin==Target --> Connection Operation cancelled."));
				OriginWaypoint = (-1);
			} else {
				sprintf(VanishingMessage, _("\n\nOrigin: %d Target: %d. Operation makes sense."), OriginWaypoint, i);
				if (action_toggle_waypoint_connection(EditLevel, OriginWaypoint, i, 1, 1) < 0) {
					strcat(VanishingMessage, _("\nOperation done, connection removed."));
				} else {
					strcat(VanishingMessage, _("\nOperation done, connection added."));
				}
				OriginWaypoint = (-1);
			}
		}
	}

	return;

}

void action_set_floor(Level EditLevel, int x, int y, int type)
{
	int old = EditLevel->map[y][x].floor_value;
	EditLevel->map[y][x].floor_value = type;
	action_push(ACT_TILE_FLOOR_SET, x, y, old);
}

static int action_change_map_label(level *EditLevel, int i, char *name, int undoable)
{
	struct map_label_s *map_label;
	char *old_label = NULL;

	// If the map label exist, remove it
	if (i < EditLevel->map_labels.size) {
		// Get the map label
		map_label = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		// Get the old label for undoable actions
		old_label = map_label->label_name;

		// Delete the map label
		del_map_label(EditLevel, old_label);
	}

	// Create the undo action if appropriate
	if (undoable) {
		action_push(ACT_SET_MAP_LABEL, i, old_label);
	} else {
		free(old_label);
	}

	// If the new label is empty, we are done
	if (!name || !strlen(name))
		return 0;

	name = strdup(name);

	// Create a new map label at the position of cursor
	add_map_label(EditLevel, rintf(Me.pos.x - 0.5), rintf(Me.pos.y - 0.5), name);

	return undoable;
}

void level_editor_action_change_map_label_user(level *EditLevel)
{
	struct map_label_s *map_label = NULL;
	char *name;
	char *old_name = NULL;
	char suggested_label[200];
	int i;

	suggested_label[0] = 0;

	// We check if a map label already exists for this spot
	for (i = 0; i < EditLevel->map_labels.size; i++) {
		map_label = &ACCESS_MAP_LABEL(EditLevel->map_labels, i);

		if ((fabsf(map_label->pos.x + 0.5 - Me.pos.x) < 0.5) && 
			 (fabsf(map_label->pos.y + 0.5 - Me.pos.y) < 0.5)) {
			
			// Use the old label as a suggestion
			old_name = map_label->label_name;
			strcpy(suggested_label, old_name);
			break;
		}
	}
		
	// Check if the name entered already exists
	while (1) {
		// Show popup window to enter a new map label
		name = GetEditableStringInPopupWindow(sizeof(suggested_label) - 1, _("\nPlease enter map label: \n\n"), suggested_label);
		if (!name || (old_name && !strcmp(name, old_name))) {
			// Do not change label
			free(name);
			return;
		}

		map_label = get_map_label(EditLevel, name);
		if (!map_label) {
			// When the new name of the map label does not exist, we are done
			break;
		}

		// When the new name already exists, we must not create an other map
		// label with the same name, but we want to display an alert window,
		// and then go back to input box with the name still present
		alert_window(_("The new name of the map label already exists on this map, please choose an other name."));

		// Copy the name in order to have it in the input box
		strcpy(suggested_label, name);
		free(name);

		// Restore the menu background in order to correctly draw the next popup window
		RestoreMenuBackground(1);
	}

	// Change a map label when the name enter by the user is valid
	action_change_map_label(EditLevel, i, name, 1);
}

/**
 *  @fn void jump_to_level( int target_map, float x, float y)
 *
 *  @brief jumps to a target level, saving this level on the undo/redo stack
 */
void action_jump_to_level(int target_level, double x, double y)
{
	// When the user wants to change the current edited level, reset tools
	leveleditor_reset_tools();

	action_push(ACT_JUMP_TO_LEVEL, EditLevel()->levelnum, Me.pos.x, Me.pos.y);	//< sets undo or redo stack, depending on push_mode state
	reset_visible_levels();
	Teleport(target_level, (float)x, (float)y, FALSE);
}

static void action_do(level * level, action * a)
{
	switch (a->type) {
	case ACT_CREATE_OBSTACLE:
		action_create_obstacle_user(level, a->d.create_obstacle.x, a->d.create_obstacle.y, a->d.create_obstacle.new_obstacle_type);
		break;
	case ACT_REMOVE_OBSTACLE:
		action_remove_obstacle_user(level, a->d.delete_obstacle);
		break;
	case ACT_MOVE_OBSTACLE:
		action_move_obstacle(level, a->d.move_obstacle.obstacle, a->d.move_obstacle.newx, a->d.move_obstacle.newy);
		break;
	case ACT_CREATE_ITEM:
		action_create_item(level, a->d.create_item.x, a->d.create_item.y, a->d.create_item.type);
		break;
	case ACT_REMOVE_ITEM:
		action_remove_item(level, a->d.delete_item);
		break;
	case ACT_MOVE_ITEM:
		action_move_item(level, a->d.move_item.item, a->d.move_item.newx, a->d.move_item.newy);
		break;
	case ACT_CREATE_WAYPOINT:
		action_create_waypoint(level, a->d.create_waypoint.x, a->d.create_waypoint.y, a->d.create_waypoint.suppress_random_spawn);
		break;
	case ACT_REMOVE_WAYPOINT:
		action_remove_waypoint(level, a->d.delete_waypoint.x, a->d.delete_waypoint.y);
		break;
	case ACT_TOGGLE_WAYPOINT_RSPAWN:
		action_toggle_waypoint_randomspawn(level, a->d.toggle_waypoint_rspawn.x, a->d.toggle_waypoint_rspawn.y);
		break;
	case ACT_TOGGLE_WAYPOINT_CONNECTION:
		action_toggle_waypoint_connection(level, a->d.toggle_waypoint_connection.x, a->d.toggle_waypoint_connection.y, 1, 1);
		break;
	case ACT_TILE_FLOOR_SET:
		action_set_floor(level, a->d.change_floor.x, a->d.change_floor.y, a->d.change_floor.type);
		break;
	case ACT_MULTIPLE_ACTIONS:
		ErrorMessage(__FUNCTION__, "Passed a multiple actions meta-action as parameter. A real action is needed.\n", PLEASE_INFORM, IS_WARNING_ONLY);
		break;
	case ACT_SET_OBSTACLE_LABEL:
		action_change_obstacle_label(level, a->d.change_obstacle_name.obstacle, a->d.change_obstacle_name.new_name, 1);
		break;
	case ACT_SET_MAP_LABEL:
		action_change_map_label(level, a->d.change_label_name.id, a->d.change_label_name.new_name, 1);
		break;
	case ACT_JUMP_TO_LEVEL:
		action_jump_to_level(a->d.jump_to_level.target_level, a->d.jump_to_level.x, a->d.jump_to_level.y);
		break;

	}
}

static void __level_editor_do_action_from_stack(struct list_head *stack)
{
	action *a;

	if (list_empty(stack))
		return;

	// Get the top action from the undo/redo stack
	a = list_entry(stack->next, action, node);

	if (a->type == ACT_MULTIPLE_ACTIONS) {
		// When the action is multiple, we must execute each action,
		// and push all actions in the stack in order to undo
		int i, max;

		max = a->d.number_actions;
		clear_action(a);

		// Execute all actions
		for (i = 0; i < max; i++) {
			__level_editor_do_action_from_stack(stack);
		}

		// Push all actions in the stack in order to undo
		action_push(ACT_MULTIPLE_ACTIONS, max);
	} else {
		action_do(EditLevel(), a);
		clear_action(a);
	}
}

void level_editor_action_undo()
{
	push_mode = UNDO;
	__level_editor_do_action_from_stack(&to_undo);
	push_mode = NORMAL;
}

void level_editor_action_redo()
{
	push_mode = REDO;
	__level_editor_do_action_from_stack(&to_redo);
	push_mode = NORMAL;
}

/**
 * Place an aligned object on the map with the keypad
 * \param positionid The position of the object on the purple grid
 */
void level_editor_place_aligned_object(int positionid)
{
	float position_offset_x[9] = { 0, 0.5, 1.0, 0, 0.5, 1.0, 0, 0.5, 1.0 };
	float position_offset_y[9] = { 1.0, 1.0, 1.0, 0.5, 0.5, 0.5, 0, 0, 0 };
	struct leveleditor_categoryselect *cs = get_current_object_type();
	int type = cs->indices[cs->selected_tile_nb];
	moderately_finepoint pos;

	positionid--;

	// Calculate the position of the object
	pos.x = (int)Me.pos.x + position_offset_x[positionid];
	pos.y = (int)Me.pos.y + position_offset_y[positionid];

	switch (cs->type) {
	case OBJECT_OBSTACLE:
			action_create_obstacle_user(EditLevel(), pos.x, pos.y, type);
			break;
	case OBJECT_FLOOR:
			action_set_floor(EditLevel(), (int)Me.pos.x, (int)Me.pos.y, type);
			break;
	case OBJECT_ITEM:
			action_create_item(EditLevel(), pos.x, pos.y, type);
			break;
	default:
			break;
	}
}

/**
 * This function should create a completely new level into the existing
 * ship structure that we already have.  The new level will be rather
 * small and simple.
 */
void CreateNewMapLevel(int level_num)
{
	level *NewLevel;
	int i, k, l;

	// Get the memory for one level 
	//
	NewLevel = (Level) MyMalloc(sizeof(level));

	DebugPrintf(0, "\n-----------------------------------------------------------------");
	DebugPrintf(0, "\nStarting to create and add a completely new level to the ship.");

	// Now we proceed in the order of the struct 'level' in the
	// struct.h file so that we can easily verify if we've handled
	// all the data structure or left something out which could
	// be terrible!
	//
	NewLevel->levelnum = level_num;
	NewLevel->xlen = 90;
	NewLevel->ylen = 90;
	NewLevel->light_bonus = 19;
	NewLevel->minimum_light_value = 19;
	NewLevel->infinite_running_on_this_level = FALSE;
	NewLevel->random_dungeon = 0;
	NewLevel->dungeon_generated = FALSE;
	NewLevel->Levelname = strdup("New level just created");
	NewLevel->Background_Song_Name = strdup("TheBeginning.ogg");

	// First we initialize the floor with 'empty' values
	//
	for (i = 0; i < NewLevel->ylen; i++) {
		NewLevel->map[i] = MyMalloc(NewLevel->xlen * sizeof(map_tile));
		for (k = 0; k < NewLevel->xlen; k++) {
			NewLevel->map[i][k].floor_value = ISO_FLOOR_SAND;
			for (l = 0; l < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE; l++) {
				NewLevel->map[i][k].obstacles_glued_to_here[l] = (-1);
			}
		}
	}
	// Now we initialize the level jump interface variables with 'empty' values
	//
	NewLevel->jump_target_north = (-1);
	NewLevel->jump_target_south = (-1);
	NewLevel->jump_target_east = (-1);
	NewLevel->jump_target_west = (-1);

	// Now we initialize the map obstacles with 'empty' information
	//
	for (i = 0; i < MAX_OBSTACLES_ON_MAP; i++) {
		NewLevel->obstacle_list[i].type = (-1);
		NewLevel->obstacle_list[i].pos.x = (-1);
		NewLevel->obstacle_list[i].pos.y = (-1);
		NewLevel->obstacle_list[i].pos.z = level_num;
	}

	// First we initialize the items arrays with 'empty' information
	//
	for (i = 0; i < MAX_ITEMS_PER_LEVEL; i++) {
		NewLevel->ItemList[i].pos.x = (-1);
		NewLevel->ItemList[i].pos.y = (-1);
		NewLevel->ItemList[i].pos.z = (-1);
		NewLevel->ItemList[i].type = (-1);
		NewLevel->ItemList[i].currently_held_in_hand = FALSE;

	}

	// Initialize obstacle extensions
	dynarray_init(&NewLevel->obstacle_extensions, 10, sizeof(struct obstacle_extension));

	// Initialize map labels
	dynarray_init(&NewLevel->map_labels, 10, sizeof(struct map_label_s));

	// Initialize waypoints
	dynarray_init(&NewLevel->waypoints, 10, sizeof(struct waypoint_s));
	
	curShip.AllLevels[level_num] = NewLevel;

	glue_obstacles_to_floor_tiles_for_level(level_num);

};				// void CreateNewMapLevel( int )

void delete_map_level(int lnum)
{
	int i;
	level *l = curShip.AllLevels[lnum];

	// Delete floor tiles
	for (i = 0; i < l->ylen; i++)
		free(l->map[i]);

	// Remove references to this level from others
	for (i = 0; i < MAX_LEVELS; i++) {
		if (curShip.AllLevels[i] == NULL)
			continue;

		if (curShip.AllLevels[i]->jump_target_north == lnum)
			curShip.AllLevels[i]->jump_target_north = -1;
		if (curShip.AllLevels[i]->jump_target_south == lnum)
			curShip.AllLevels[i]->jump_target_south = -1;
		if (curShip.AllLevels[i]->jump_target_west == lnum)
			curShip.AllLevels[i]->jump_target_west = -1;
		if (curShip.AllLevels[i]->jump_target_east == lnum)
			curShip.AllLevels[i]->jump_target_east = -1;
	}

	// Free memory
	free(curShip.AllLevels[lnum]);

	curShip.AllLevels[lnum] = NULL;

	if (lnum == curShip.num_levels)
		curShip.num_levels--;

}

static int get_chest_contents(level *l, obstacle *o, item *items[MAX_ITEMS_IN_INVENTORY])
{
	struct dynarray *itemlist = get_obstacle_extension(l, o, OBSTACLE_EXTENSION_CHEST_ITEMS);

	memset(items, 0, MAX_ITEMS_IN_INVENTORY);

	if (!itemlist) {
		return 0;
	}

	int i;
	int curitem = 0;
	for (i = 0; i < itemlist->size; i++) {
		items[curitem++] = &((item *)itemlist->arr)[i];
		if (curitem == MAX_ITEMS_IN_INVENTORY - 1)
			break;
	}

	return curitem;
}

void level_editor_edit_chest(obstacle *o)
{
	item *chest_items[MAX_ITEMS_IN_INVENTORY];
	item *user_items[2];
	int chest_nb_items;
	int done = 0;
	shop_decision shop_order;
	item *tmp;

	item dummy_addtochest;
	init_item(&dummy_addtochest);
	dummy_addtochest.type = 1;
	FillInItemProperties(&dummy_addtochest, 2, 1);

	user_items[0] = &dummy_addtochest;
	user_items[1] = NULL;

	struct dynarray *itemlist = get_obstacle_extension(CURLEVEL(), o, OBSTACLE_EXTENSION_CHEST_ITEMS);

	// Safety check
	switch (o->type) {
	case ISO_H_CHEST_CLOSED:
	case ISO_H_CHEST_OPEN:
	case ISO_V_CHEST_CLOSED:
	case ISO_V_CHEST_OPEN:
	case ISO_E_CHEST2_CLOSED:
	case ISO_S_CHEST2_CLOSED:
	case ISO_N_CHEST2_CLOSED:
	case ISO_W_CHEST2_CLOSED:
		break;
	default:
		ErrorMessage(__FUNCTION__, "Tried to edit the contents of a chest, but the obstacle is not a chest.\n", PLEASE_INFORM,
			     IS_FATAL);
	}

	while (!done) {

		// Build the list of items in the chest
		chest_nb_items = get_chest_contents(CURLEVEL(), o, chest_items);

		// Display the shop interface
		done = GreatShopInterface(chest_nb_items, chest_items, 1, user_items, &shop_order);

		// BUY removes an item from the chest
		// SELL spaws the drop item interface
		switch (shop_order.shop_command) {
		case BUY_1_ITEM:
			DeleteItem(chest_items[shop_order.item_selected]);
			dynarray_del(itemlist, shop_order.item_selected, sizeof(item));
			break;
		case SELL_1_ITEM:
			tmp = ItemDropFromLevelEditor();
			if (tmp) {

				if (!itemlist) {
					itemlist = dynarray_alloc(10, sizeof(item));
					add_obstacle_extension(CURLEVEL(), o, OBSTACLE_EXTENSION_CHEST_ITEMS, itemlist);
				}

				dynarray_add(itemlist, tmp, sizeof(item)); 

				// delete the ground copy
				DeleteItem(tmp);
			}
			break;
		default:
			;
		}
	}
}

#undef _leveledijtor_action_c
