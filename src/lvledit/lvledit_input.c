/* 
 *
 *   Copyright (c) 2002, 2003 Johannes Prix
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

#define _leveleditor_input_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_widgets.h"

static void HandleLevelEditorCursorKeys()
{
	level *EditLevel;

	EditLevel = curShip.AllLevels[Me.pos.z];
	static int PressedSince[4] = { 0, 0, 0, 0 };
	int DoAct[4] = { 0, 0, 0, 0 };
	int i;
	int repeat = 1;

	// Keys that are released have to be marked as such
	if (!LeftPressed())
		PressedSince[0] = 0;
	if (!RightPressed())
		PressedSince[1] = 0;
	if (!DownPressed())
		PressedSince[2] = 0;
	if (!UpPressed())
		PressedSince[3] = 0;

	if (LeftPressed() && PressedSince[0] == 0) {
		PressedSince[0] = SDL_GetTicks();
		DoAct[0] = 1;
	}
	if (RightPressed() && PressedSince[1] == 0) {
		PressedSince[1] = SDL_GetTicks();
		DoAct[1] = 1;
	}
	if (DownPressed() && PressedSince[2] == 0) {
		PressedSince[2] = SDL_GetTicks();
		DoAct[2] = 1;
	}
	if (UpPressed() && PressedSince[3] == 0) {
		PressedSince[3] = SDL_GetTicks();
		DoAct[3] = 1;
	}

	for (i = 0; i < 4; i++) {
		if (PressedSince[i] && SDL_GetTicks() - PressedSince[i] > 500) {
			DoAct[i] = 1;
			PressedSince[i] = SDL_GetTicks() - 450;
		}
	}

	if (CtrlPressed())
		repeat = FLOOR_TILES_VISIBLE_AROUND_TUX;

	if (DoAct[0]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.x) > 0)
				Me.pos.x -= 1;
	}
	if (DoAct[1]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.x) < EditLevel->xlen - 1)
				Me.pos.x += 1;
	}
	if (DoAct[2]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.y) < EditLevel->ylen - 1)
				Me.pos.y += 1;
	}
	if (DoAct[3]) {
		for (i = 0; i < repeat; i++)
			if (rintf(Me.pos.y) > 0)
				Me.pos.y -= 1;
	}
};				// void HandleLevelEditorCursorKeys ( void )

/**
 * This function automatically scrolls the leveleditor window when the
 * mouse reaches an edge 
 */
static void level_editor_auto_scroll()
{
	float chx = 0, chy = 0;	/*Value of the change to player position */
	static int edgedate[4] = { 0, 0, 0, 0 };

#define AUTOSCROLL_DELAY 500

	if (GetMousePos_x() < 5) {
		// scroll to the left
		if (edgedate[0] == 0) {
			edgedate[0] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[0] > AUTOSCROLL_DELAY) {
			chx -= 0.05;
			chy += 0.05;
		}
	} else {
		edgedate[0] = 0;
	}

	if (GameConfig.screen_width - GetMousePos_x() < 5) {
		// scroll to the right
		if (edgedate[1] == 0) {
			edgedate[1] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[1] > AUTOSCROLL_DELAY) {
			chx += 0.05;
			chy -= 0.05;
		}
	} else {
		edgedate[1] = 0;
	}

	if (GameConfig.screen_height - GetMousePos_y() < 5) {
		// scroll down
		if (edgedate[2] == 0) {
			edgedate[2] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[2] > AUTOSCROLL_DELAY) {
			chx += 0.05;
			chy += 0.05;
		}
	} else {
		edgedate[2] = 0;
	}

	if (GetMousePos_y() < 5) {
		//scroll up
		if (edgedate[3] == 0) {
			edgedate[3] = SDL_GetTicks();
		} else if (SDL_GetTicks() - edgedate[3] > AUTOSCROLL_DELAY) {
			chx -= 0.05;
			chy -= 0.05;
		}
	} else {
		edgedate[3] = 0;
	}

	Me.pos.x += chx;
	Me.pos.y += chy;

	if (Me.pos.x >= curShip.AllLevels[Me.pos.z]->xlen - 1)
		Me.pos.x = curShip.AllLevels[Me.pos.z]->xlen - 1;
	if (Me.pos.x <= 0)
		Me.pos.x = 0;
	if (Me.pos.y >= curShip.AllLevels[Me.pos.z]->ylen - 1)
		Me.pos.y = curShip.AllLevels[Me.pos.z]->ylen - 1;
	if (Me.pos.y <= 0)
		Me.pos.y = 0;
}

void leveleditor_input_mouse_motion(SDL_Event * event)
{
	struct leveleditor_widget *w;

	w = get_active_widget(event->motion.x, event->motion.y);

	if (previously_active_widget != w) {
		if (w)
			w->mouseenter(event, w);
		if (previously_active_widget)
			previously_active_widget->mouseleave(event, previously_active_widget);
		previously_active_widget = w;
	}

	if (w && w->mousemove)
		w->mousemove(event, w);
}

void leveleditor_input_mouse_button(SDL_Event * event)
{
	struct leveleditor_widget *w;

	w = get_active_widget(event->button.x, event->button.y);

	if (w) {
		switch (event->type) {
		case SDL_MOUSEBUTTONUP:
			switch (event->button.button) {
			case 1:
				w->mouserelease(event, w);
				break;
			case 3:
				w->mouserightrelease(event, w);
				break;
			case 4:
			case 5:
				break;
			default:
				break;
				//ErrorMessage(__FUNCTION__, "Mouse button index %hd  not handled by leveleditor widgets.\n", PLEASE_INFORM, IS_WARNING_ONLY, event->button.button);
			}
			break;
		case SDL_MOUSEBUTTONDOWN:
			switch (event->button.button) {
			case 1:
				w->mousepress(event, w);
				break;
			case 3:
				w->mouserightpress(event, w);
				break;
			case 4:
				w->mousewheelup(event, w);
				break;
			case 5:
				w->mousewheeldown(event, w);
				break;
			default:
				break;
				//ErrorMessage(__FUNCTION__, "Mouse button index %hd not handled by leveleditor widgets.\n", PLEASE_INFORM, IS_WARNING_ONLY, event->button.button);
			}
			break;
		default:
			ErrorMessage(__FUNCTION__, "Event type %d sent to leveleditor as a mouse button event is not recognized.\n",
				     PLEASE_INFORM, IS_FATAL, event->type);
		}
	}
}

void leveleditor_input_keybevent(SDL_Event * event)
{
	struct leveleditor_widget *w, *n;

	w = get_active_widget(GetMousePos_x(), GetMousePos_y());

	// When we get a keyboard event that wasn't handled by the "general" keybinding system,
	// it means we have something widget- and state- dependent.
	// We will forward the event to the currently active widget, and if it did not handle it,
	// forward it to each widget in the list in order.

	if (w && w->keybevent && !w->keybevent(event, w))
		return;

	list_for_each_entry_safe(w, n, &leveleditor_widget_list, node) {
		if (w && w->keybevent && !w->keybevent(event, w))
			return;
	}

}

void leveleditor_process_input()
{
	leveleditor_update_button_states();
	save_mouse_state();
	input_handle();

	HandleLevelEditorCursorKeys();

	// If the person using the level editor pressed w, the waypoint is
	// toggled on the current square.  That means either removed or added.
	// And in case of removal, also the connections must be removed.
	//
	if (WPressed()) {
		int wpnum = get_waypoint(EditLevel(), EditX(), EditY());
		if (wpnum < 0) {
			// If the waypoint doesn't exist at the map position, create it
			action_create_waypoint(EditLevel(), EditX(), EditY(), ShiftPressed());
		} else {
			// An existing waypoint will be removed or have its
			// randomspawn flag toggled
			waypoint *wpts = EditLevel()->waypoints.arr;
			if (ShiftPressed()) {
				action_toggle_waypoint_randomspawn(EditLevel(), wpts[wpnum].x, wpts[wpnum].y);
			} else {
				action_remove_waypoint(EditLevel(), wpts[wpnum].x, wpts[wpnum].y);
			}
		}
		while (WPressed())
			SDL_Delay(1);
	}

	level_editor_auto_scroll();

	if (EscapePressed()) {
		level_editor_done = DoLevelEditorMainMenu();
	}
	while (EscapePressed())
		SDL_Delay(1);

}
