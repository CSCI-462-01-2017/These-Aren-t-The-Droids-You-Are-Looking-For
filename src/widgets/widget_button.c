/* 
 *
 *   Copyright (c) 2009-2010 Arthur Huillet
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

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "widgets/widgets.h"

/**
 * Generic handle event function for buttons.
 * 
 * Buttons have 3 primary states: normal, hovered and pressed.
 * Pressed state is switched on on mouse press event and switched off
 * on mouse release or mouse leave event.
 * 
 * Buttons have one primary and one secondary action. The primary action is
 * triggered on left mouse button release while the secondary action is 
 * triggered on right mouse button release. Both actions require
 * the button to be in pressed state when receiving the release event
 * to be triggered.
 * @return 1 if the event was handled and no further handling is required.
 */
static int button_handle_event(struct widget *w, SDL_Event *event)
{
	struct widget_button *b = WIDGET_BUTTON(w);
	switch (event->type) {
		case SDL_MOUSEBUTTONDOWN:
			// Button has been pressed.
			b->state = PRESSED;
			return 1;

		case SDL_MOUSEBUTTONUP:
			// Trigger the button if it is currently pressed.
			if (b->state == PRESSED) {
				b->state = HOVERED;
				if (event->button.button == MOUSE_BUTTON_1)
					if (b->activate_button)
						b->activate_button(b);
				if (event->button.button == MOUSE_BUTTON_3)
					if (b->activate_button_secondary)
						b->activate_button_secondary(b);
			}
			break;

		case SDL_USEREVENT:
			// Set hovered state when the mouse is over the button.
			if (event->user.code == EVENT_MOUSE_ENTER) {
				b->state = HOVERED;
				// Set the button tooltip to be displayed.
				widget_set_tooltip(&b->tooltip, &w->rect);
			}
			// Mouse left the button's rectangle, switch to normal state.
			if (event->user.code == EVENT_MOUSE_LEAVE) {
				b->state = DEFAULT;
				widget_set_tooltip(NULL, NULL);
			}
			if (event->user.code == EVENT_UPDATE && w->update)
				w->update(w);
			break;

		default:
			break;
	}
	return 0;
}

/**
 * Display a button widget.
 */
static void button_display(struct widget *wb)
{
	struct widget_button *b = WIDGET_BUTTON(wb);

	int state = WIDGET_BUTTON(wb)->active;
	int offset = b->state;

	struct image *img = b->image[state][offset];
	
	// NULL image: fallback to the normal state image.
	if (!img)
		img = b->image[state][DEFAULT];
	
	// NULL image: fallback to default toggle state.
	if (!img)
		img = b->image[0][DEFAULT];

	// Don't display button if no image found.
	if (img)
		display_image_on_screen(img, wb->rect.x, wb->rect.y, set_image_transformation(wb->rect.w / (float)img->w, wb->rect.h / (float)img->h, 1.0, 1.0, 1.0, 1.0, 0));

	// Display button's text if any.
	if (b->text) {
		// Calculate the position of the text, we want to display the text at the
		// center of the button
		SDL_Rect rect = wb->rect;
		rect.x = rect.x + (rect.w - TextWidth(b->text)) / 2;
		rect.y = rect.y + 8;

		// Draw the text on the button
		display_text(b->text, rect.x, rect.y, &rect);
	}
}

/**
 * @brief Creates a widget_button.
 * This function creates a widget_button using the default callbacks.
 * @return A pointer to the newly created widget_button.
 */
struct widget_button *widget_button_create()
{
	struct widget_button *a = MyMalloc(sizeof(struct widget_button));
	widget_set_rect(WIDGET(a), 0, 0, 0, 0);
	WIDGET(a)->type = WIDGET_BUTTON;
	WIDGET(a)->display = button_display;
	WIDGET(a)->handle_event = button_handle_event;
	WIDGET(a)->update = NULL;
	WIDGET(a)->enabled = 1;

	a->state = DEFAULT;
	a->active = 0;
	a->text = NULL;
	a->tooltip.text = NULL;
	a->tooltip.get_text = NULL;
	a->activate_button = NULL;
	a->activate_button_secondary = NULL;
	
	int i;
	for (i = 0; i < 3; i++) {
		a->image[0][i] = NULL;
		a->image[1][i] = NULL;
	}

	return a;
}
