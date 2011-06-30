/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2009 Arthur Huillet 
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
 * This file contains functions related to events and event triggers.
 */

#define _event_c
#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

struct event_trigger {

	luacode lua_code;

	char *name;
	int enabled;		//is the trigger enabled?
	int silent;		//do we have to advertise this trigger to the user? (teleporters..)


	enum {
		POSITION,
		ENTER_LEVEL,
		EXIT_LEVEL,
		ENEMY_DEATH,
	} trigger_type;

	union {
		struct {
			int level;
			int x;
			int y;
		} position;
		struct {
			int level;
		} change_level;
		struct {
			int level;
			int faction;
			char *dialog_name;
			int marker;
		} enemy_death;
	} trigger;
};

static struct dynarray event_triggers;

/**
 * Delete all events and event triggers
 */
static void clear_out_events(void)
{
	// Remove existing triggers
	int i;
	struct event_trigger *arr = event_triggers.arr;
	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].name)
			free(arr[i].name);

		if (arr[i].lua_code)
			free(arr[i].lua_code);
	}

	dynarray_free(&event_triggers);
	
	dynarray_init(&event_triggers, 16, sizeof(struct event_trigger));
}

#define EVENT_TRIGGER_BEGIN_STRING "* New event trigger *"
#define EVENT_TRIGGER_END_STRING "* End of trigger *"
#define EVENT_TRIGGER_NAME_STRING "Name=_\""
#define EVENT_TRIGGER_LUACODE_STRING "LuaCode={"
#define EVENT_TRIGGER_LUACODE_END_STRING "}"
#define EVENT_TRIGGER_ENABLED_STRING "Enable this trigger by default="

// For position-based events
#define EVENT_TRIGGER_LABEL_STRING "Trigger at label=\""
#define EVENT_TRIGGER_IS_SILENT_STRING "Silent="

// For level-enter events
#define LEVEL_EVENT_LVLNUM_VALUE "Trigger entering level="

// For level-exit events
#define LEVEL_EXIT_LVLNUM_VALUE "Trigger exiting level="

// For enemy-death events
#define ENEMY_DEATH_LVLNUM_VALUE "Trigger on enemy death in level="
#define ENEMY_DEATH_FACTION "Enemy faction=\""
#define ENEMY_DEATH_DIALOG_NAME "Enemy dialog name=\""
#define ENEMY_DEATH_MARKER "Enemy marker="

/** 
 *
 *
 */
static void load_events(char *EventSectionPointer)
{
	char *EventPointer;
	char *EndOfEvent;
	char *TempMapLabelName;
	location TempLocation;
	char *TempEnemyFaction;
	char s;
	struct event_trigger temp;

	EventPointer = EventSectionPointer;
	while ((EventPointer = strstr(EventPointer, EVENT_TRIGGER_BEGIN_STRING)) != NULL) {
		memset(&temp, 0, sizeof(struct event_trigger));

		EventPointer += strlen(EVENT_TRIGGER_BEGIN_STRING) + 1;

		EndOfEvent = LocateStringInData(EventPointer, EVENT_TRIGGER_END_STRING);
		s = EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1];
		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = 0;

		// Determine type of event condition
		if ((TempMapLabelName = strstr(EventPointer, EVENT_TRIGGER_LABEL_STRING))) {
			temp.trigger_type = POSITION;
			TempMapLabelName = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LABEL_STRING, "\"");
			ResolveMapLabelOnShip(TempMapLabelName, &TempLocation);
			temp.trigger.position.x = TempLocation.x;
			temp.trigger.position.y = TempLocation.y;
			temp.trigger.position.level = TempLocation.level;
			free(TempMapLabelName);
			ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_IS_SILENT_STRING, "%d", "1",
						&temp.silent, EndOfEvent);
		} else if (strstr(EventPointer, LEVEL_EVENT_LVLNUM_VALUE)) {
			temp.trigger_type = ENTER_LEVEL;
			ReadValueFromString(EventPointer, LEVEL_EVENT_LVLNUM_VALUE, "%d",
						&temp.trigger.change_level.level, EndOfEvent);
		} else if (strstr(EventPointer, LEVEL_EXIT_LVLNUM_VALUE)) {
			temp.trigger_type = EXIT_LEVEL;
			ReadValueFromString(EventPointer, LEVEL_EXIT_LVLNUM_VALUE, "%d",
						&temp.trigger.change_level.level, EndOfEvent);
		} else {
			temp.trigger_type = ENEMY_DEATH;
			ReadValueFromString(EventPointer, ENEMY_DEATH_LVLNUM_VALUE, "%d",
						&temp.trigger.enemy_death.level, EndOfEvent);
			if (strstr(EventPointer, ENEMY_DEATH_FACTION)) {
				TempEnemyFaction = ReadAndMallocStringFromData(EventPointer, ENEMY_DEATH_FACTION, "\"");
				temp.trigger.enemy_death.faction = get_faction_id(TempEnemyFaction);
				free(TempEnemyFaction);
			} else {
				temp.trigger.enemy_death.faction = -1;
			}
			if (strstr(EventPointer, ENEMY_DEATH_DIALOG_NAME)) {
				temp.trigger.enemy_death.dialog_name = ReadAndMallocStringFromData(EventPointer, ENEMY_DEATH_DIALOG_NAME, "\"");
			} else {
				temp.trigger.enemy_death.dialog_name = NULL;
			}
			ReadValueFromStringWithDefault(EventPointer, ENEMY_DEATH_MARKER, "%d", "-1",
						&temp.trigger.enemy_death.marker, EndOfEvent);
		}

		temp.name = ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_NAME_STRING, "\"");

		temp.lua_code =
			ReadAndMallocStringFromData(EventPointer, EVENT_TRIGGER_LUACODE_STRING, EVENT_TRIGGER_LUACODE_END_STRING);

		ReadValueFromStringWithDefault(EventPointer, EVENT_TRIGGER_ENABLED_STRING, "%d", "1",
						&temp.enabled, EndOfEvent);

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = '\0';

		EndOfEvent[strlen(EVENT_TRIGGER_END_STRING) - 1] = s;

		dynarray_add(&event_triggers, &temp, sizeof(struct event_trigger));
	}			// While Event trigger begin string found...
}

/**
 * This function reads in the game events, i.e. the locations and conditions
 * under which some actions are triggered.
 */
void GetEventTriggers(const char *EventsAndEventTriggersFilename)
{
	char *EventSectionPointer;
	char fpath[2048];

	clear_out_events();

	find_file(EventsAndEventTriggersFilename, MAP_DIR, fpath, 0);
	EventSectionPointer =
		ReadAndMallocAndTerminateFile(fpath, "*** END OF EVENT ACTION AND EVENT TRIGGER FILE *** LEAVE THIS TERMINATOR IN HERE ***");

	load_events(EventSectionPointer);

	free(EventSectionPointer);
};

/**
 *
 * This function checks for triggered position-based events. Those events are
 * usually entered via the mission file and read into the appropriate
 * structures via the InitNewMission function.  Here we check, whether
 * the necessary conditions for an event are satisfied, and in case that
 * they are, we order the appropriate event to be executed.
 *
 */
void trigger_position_events(void)
{
	int i;
	int position_previous_x = (int)(GetInfluPositionHistoryX(0));
	int position_previous_y = (int)(GetInfluPositionHistoryY(0));
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {

		if (arr[i].trigger_type != POSITION)
			continue;

		if (arr[i].trigger.position.level != Me.pos.z)
 			continue;

		if (!arr[i].enabled)
			continue;

		if (((int)Me.pos.x == arr[i].trigger.position.x) 
		&& ((int)Me.pos.y == arr[i].trigger.position.y)) {

			if ((position_previous_x != arr[i].trigger.position.x) 
			|| (position_previous_y != arr[i].trigger.position.y))

				run_lua(arr[i].lua_code);
		}
	}
}

/**
 * Trigger level-enter and level-exit events for the given level
 */
void event_level_changed(int past_lvl, int cur_lvl)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type == ENTER_LEVEL) {
			if (arr[i].trigger.change_level.level != cur_lvl)
				continue;

		} else if (arr[i].trigger_type == EXIT_LEVEL) {
			if (arr[i].trigger.change_level.level != past_lvl)
				continue;

		} else
			continue;

		if (!arr[i].enabled)
			continue;

		run_lua(arr[i].lua_code);
	}
}

/**
 * Trigger level-enter events for the given level
 */
void event_enemy_died(enemy *dead)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type != ENEMY_DEATH)
			continue;

		if (!arr[i].enabled)
			continue;

		if (arr[i].trigger.enemy_death.level != dead->pos.z)
			continue;

		if (arr[i].trigger.enemy_death.faction != -1)
			if (arr[i].trigger.enemy_death.faction != dead->faction)
				continue;

		if (arr[i].trigger.enemy_death.dialog_name != NULL)
			if (strcmp(arr[i].trigger.enemy_death.dialog_name, dead->dialog_section_name))
				continue;

		if (arr[i].trigger.enemy_death.marker != -1)
			if (arr[i].trigger.enemy_death.marker != dead->marker)
				continue;

		run_lua(arr[i].lua_code);
	}
}

/**
 *
 *
 */
const char *teleporter_square_below_mouse_cursor(void)
{
	finepoint MapPositionOfMouse;
	struct event_trigger *arr;

	if (MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y())) {
		MapPositionOfMouse.x = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, TRUE);
		MapPositionOfMouse.y = translate_pixel_to_map_location((float)input_axis.x, (float)input_axis.y, FALSE);
 		arr = visible_event_at_location((int)MapPositionOfMouse.x, (int)MapPositionOfMouse.y, Me.pos.z);
		if (arr)
			return D_(arr->name);
	}
	return NULL;
}

/**
 * Enable or disable the trigger with the given name.
 */
void event_modify_trigger_state(const char *name, int state)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;

	for (i = 0; i < event_triggers.size; i++) {
		if (!arr[i].name)
			break;

		if (!strcmp(arr[i].name, name)) {
			arr[i].enabled = state;
			return;
		}
	}
}

/**
 * Visible event Trigger at a location
 */
struct event_trigger * visible_event_at_location(int x, int y, int z)
{
	int i;
	struct event_trigger *arr = event_triggers.arr;
	for (i = 0; i < event_triggers.size; i++) {
		if (arr[i].trigger_type != POSITION)
			continue;

		if (z != arr[i].trigger.position.level)
 			continue;

		if (x != arr[i].trigger.position.x)
			continue;

		if (y != arr[i].trigger.position.y)
			continue;

		if (arr[i].silent)
			continue;

		if (!arr[i].enabled)
			continue;

		return &arr[i];
	}
	return NULL;
}
