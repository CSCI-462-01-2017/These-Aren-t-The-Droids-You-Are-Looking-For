/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
 *   Copyright (c) 2004-2007 Arthur Huillet 
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
 * This file contains (all) functions for keyboard and joystick handling
 */

#define _input_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "widgets/widgets.h"

#ifndef SDL_BUTTON_WHEELUP
#define SDL_BUTTON_WHEELUP 4
#endif
#ifndef SDL_BUTTON_WHEELDOWN
#define SDL_BUTTON_WHEELDOWN 5
#endif

Uint8 *key_state_array;		/* SDL keyboard state internal arrey */
int key_state_array_size;

Uint8 mouse_state_last_frame;	/* mouse states reference */
Uint8 mouse_state_this_frame;	/* current mouse state */

int MouseWheelUpMovesRecorded;
int MouseWheelDownMovesRecorded;

void init_keyboard_input_array()
{
	key_state_array = SDL_GetKeyState(&key_state_array_size);
}

int GetMousePos_x(void)
{
	int x;
	SDL_GetMouseState(&x, NULL);
	return x;
};

int GetMousePos_y(void)
{
	int y;
	SDL_GetMouseState(NULL, &y);
	return y;
};

/**
 * Save the current mouse state to mouse_state_last_frame, so
 * the *Clicked functions have a point of comparison.
 */
void save_mouse_state()
{
	mouse_state_last_frame = mouse_state_this_frame;
	mouse_state_this_frame = SDL_GetMouseState(NULL, NULL);
}

static void input_mouse_motion(SDL_Event * event)
{
	if (game_status != INSIDE_LVLEDITOR) {
		input_axis.x = event->motion.x - UserCenter_x;
		input_axis.y = event->motion.y - UserCenter_y;
	} else {
		handle_widget_event(event);
	}
}

static void input_mouse_button(SDL_Event * event)
{
	if (game_status != INSIDE_LVLEDITOR) {
		if (event->type == SDL_MOUSEBUTTONUP)
			return;
		input_axis.x = event->button.x - UserCenter_x;
		input_axis.y = event->button.y - UserCenter_y;

		if (event->button.button == SDL_BUTTON_WHEELUP)
			MouseWheelUpMovesRecorded++;

		if (event->button.button == SDL_BUTTON_WHEELDOWN)
			MouseWheelDownMovesRecorded++;
	} else {
		handle_widget_event(event);
	}
}

int input_handle(void)
{
	SDL_Event event;

	while (SDL_PollEvent(&event)) {

		switch (event.type) {
		case SDL_QUIT:
			printf("\n\nUser requested termination...\n\nTerminating...");
			Terminate(EXIT_SUCCESS, TRUE);
			break;

		case SDL_KEYDOWN:
			input_key_press(&event);
			break;

		case SDL_MOUSEMOTION:
			input_mouse_motion(&event);
			break;

		case SDL_MOUSEBUTTONDOWN:
		case SDL_MOUSEBUTTONUP:
			input_mouse_button(&event);
			break;

		default:
			break;
		}
	}
	return 0;
}

int MouseWheelUpPressed(void)
{
	if (MouseWheelUpMovesRecorded) {
		MouseWheelUpMovesRecorded--;
		return (TRUE);
	} else
		return (FALSE);
}

int MouseWheelDownPressed(void)
{
	if (MouseWheelDownMovesRecorded) {
		MouseWheelDownMovesRecorded--;
		return (TRUE);
	} else
		return (FALSE);
}

/* *Pressed functions return the current state of the keyboard/mouse button in question */
int LeftPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_LEFT]));
}

int RightPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_RIGHT]));
}

int UpPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_UP]));
}

int DownPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_DOWN]));
}

int SpacePressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_SPACE]));
}

int EnterPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_RETURN]));
}

int EscapePressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_ESCAPE]));
}

int LeftCtrlPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_LCTRL]));
}

int CtrlPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_LCTRL]) || key_state_array[SDLK_RCTRL]);
}

int ShiftPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_RSHIFT] || key_state_array[SDLK_LSHIFT]));
}

int AltPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_RALT] || key_state_array[SDLK_LALT]));
}

int APressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_a]));
}

int HPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_h]));
}

int MPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_m]));
}

int QPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_q]));
}

int SPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_s]));
}

int WPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_w]));
}

int XPressed()
{
	SDL_PumpEvents();
	return ((key_state_array[SDLK_x]));
}

int MouseRightPressed()
{
	SDL_PumpEvents();
	return ((SDL_GetMouseState(NULL, NULL)) & (SDL_BUTTON(3)));
}

int MouseLeftPressed()
{
	SDL_PumpEvents();
	return (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(1));
}

/* *Clicked functions return non zero if the mouse button is question has just been clicked, i.e. it is pressed now and wasn't at last frame.
 * "last frame" is whenever you called save_mouse_state
 */
int MouseRightClicked()
{
	return (!(mouse_state_last_frame & SDL_BUTTON(3)) && (mouse_state_this_frame & SDL_BUTTON(3)));
}

int MouseLeftClicked()
{
	return (!(mouse_state_last_frame & SDL_BUTTON(1)) && (mouse_state_this_frame & SDL_BUTTON(1)));
}

int MouseRightUnclicked()
{
	return ((mouse_state_last_frame & SDL_BUTTON(3)) && !(mouse_state_this_frame & SDL_BUTTON(3)));
}

int MouseLeftUnclicked()
{
	return ((mouse_state_last_frame & SDL_BUTTON(1)) && !(mouse_state_this_frame & SDL_BUTTON(1)));
}

#undef _input_c
