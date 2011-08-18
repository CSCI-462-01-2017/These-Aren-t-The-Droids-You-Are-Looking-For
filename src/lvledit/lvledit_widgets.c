/* 
 *
 *   Copyright (c) 2009 Arthur Huillet
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

#define _lvledit_widgets_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#include "lvledit/lvledit.h"
#include "lvledit/lvledit_actions.h"
#include "lvledit/lvledit_tools.h"
#include "lvledit/lvledit_object_lists.h"
#include "lvledit/lvledit_display.h"
#include "lvledit/lvledit_beautify_actions.h"
#include "lvledit/lvledit_map.h"
#include "lvledit/lvledit_menu.h"
#include "lvledit/lvledit_widgets.h"

static struct widget_group *level_editor_widget_group = NULL; 
static int *all_obstacles_list = NULL;

typedef struct {
	char *name;
	int **object_list;
	struct widget_lvledit_categoryselect *cs;
} object_category;

static object_category obstacle_category_list[] = {
	{ "WALL", &wall_tiles_list },
	{ "FURNITURE", &furniture_tiles_list },
	{ "MACHINERY", &machinery_tiles_list },
	{ "CONTAINER", &container_tiles_list },
	{ "PLANT", &plant_tiles_list },
	{ "OTHER", &misc_tiles_list },
	{ "ALL", &all_obstacles_list }
};

static object_category floor_category_list[] = {
	{ "SIDEWALK", &sidewalk_floor_list },
	{ "WATER", &water_floor_list },
	{ "GRASS", &grass_floor_list },
	{ "SQUARE", &square_floor_list },
	{ "OTHER", &other_floor_list },
	{ "ALL", &floor_tiles_list }
};

static object_category item_category_list[] = {
	{ "MELEE", &melee_items_list },
	{ "GUN", &gun_items_list },
	{ "DEFENSIVE", &defense_items_list },
	{ "USEABLE", &spell_items_list },
	{ "OTHER", &other_items_list },
	{ "ALL", &all_items_list }
};

static object_category waypoint_category_list[] = {
	{ "ALL", &waypoint_list }
};

static object_category map_label_category_list[] = {
	{ "ALL", &map_label_list }
};

static object_category enemy_category_list[] = {
	{ "DROIDS", &droid_enemies_list },
	{ "HUMANS", &human_enemies_list },
	{ "ALL", &all_enemies_list },
};

static struct {
	enum lvledit_object_type object_type;
	object_category *categories;
	size_t length;
} category_list[] = {
	{ OBJECT_OBSTACLE, obstacle_category_list, sizeof(obstacle_category_list) / sizeof(obstacle_category_list[0]) },
	{ OBJECT_FLOOR, floor_category_list, sizeof(floor_category_list) / sizeof(floor_category_list[0]) },
	{ OBJECT_ITEM, item_category_list, sizeof(item_category_list) / sizeof(item_category_list[0]) },
	{ OBJECT_ENEMY, enemy_category_list, sizeof(enemy_category_list) / sizeof(enemy_category_list[0]) },
	{ OBJECT_WAYPOINT, waypoint_category_list, sizeof(waypoint_category_list) / sizeof(waypoint_category_list[0]) },
	{ OBJECT_MAP_LABEL, map_label_category_list, sizeof(map_label_category_list) / sizeof(waypoint_category_list[0]) },
};

// Level editor buttons' primary actions
static void undo_button_click(struct widget_button *wb)
{
	level_editor_action_undo();
}

static void redo_button_click(struct widget_button *wb)
{
	level_editor_action_redo();
}

static void save_ship_button_click(struct widget_button *wb)
{
	if (game_root_mode == ROOT_IS_LVLEDIT) {	/*don't allow saving if root mode is GAME */
		char fp[2048];
		find_file("levels.dat", MAP_DIR, fp, 0);
		SaveShip(fp, TRUE, 0);
		find_file("ReturnOfTux.droids", MAP_DIR, fp, 0);
		save_special_forces(fp);
		alert_window("%s", _("M E S S A G E\n\nYour ship was saved to file 'levels.dat' in the map directory.\n\nIf you have set up something cool and you wish to contribute it to FreedroidRPG, please contact the FreedroidRPG dev team."));
	} else {
		alert_window("%s", _("M E S S A G E\n\nE R R O R ! Your ship was not saved.\n\nPlaying on a map leaves the world in an unclean state not suitable for saving. Enter the editor from the main menu to be able to save."));
	}
}

static void beautify_grass_button_click(struct widget_button *wb)
{
	level_editor_beautify_grass_tiles(EditLevel());
}

static void delete_obstacle_button_click(struct widget_button *wb)
{
	level_editor_cut_selection();
}

static void next_object_button_click(struct widget_button *wb)
{
	level_editor_cycle_marked_object();
}

static void new_obstacle_label_button_click(struct widget_button *wb)
{
	if (single_tile_selection(OBJECT_OBSTACLE))
		action_change_obstacle_label_user(EditLevel(), single_tile_selection(OBJECT_OBSTACLE));
}

static void new_map_label_button_click(struct widget_button *wb)
{
	level_editor_action_change_map_label_user(EditLevel());
}

static void edit_chest_button_click(struct widget_button *wb)
{
	level_editor_edit_chest(single_tile_selection(OBJECT_OBSTACLE));
}

static void toggle_enemies_button_click(struct widget_button *wb)
{
	GameConfig.omit_enemies_in_level_editor = !GameConfig.omit_enemies_in_level_editor;
}

static void toggle_tooltips_button_click(struct widget_button *wb)
{
	GameConfig.show_lvledit_tooltips = !GameConfig.show_lvledit_tooltips;
}

static void toggle_collisions_button_click(struct widget_button *wb)
{
	draw_collision_rectangles = !draw_collision_rectangles;
}

static void toggle_grid_button_click(struct widget_button *wb)
{
	GameConfig.show_grid = !GameConfig.show_grid;
}

static void toggle_grid_button_right_click(struct widget_button *wb)
{
	GameConfig.grid_mode = (GameConfig.grid_mode + 1) % 2;
}

static void toggle_obstacles_button_click(struct widget_button *wb)
{
	GameConfig.omit_obstacles_in_level_editor = !GameConfig.omit_obstacles_in_level_editor;
}

static void zoom_in_button_click(struct widget_button *wb)
{
	GameConfig.zoom_is_on = !GameConfig.zoom_is_on;
}

static void quit_button_click(struct widget_button *wb)
{
	TestMap();
	if (game_root_mode == ROOT_IS_GAME)
		level_editor_done = TRUE;
}

static void editor_right_button_click(struct widget_button *wb)
{
	widget_lvledit_toolbar_scroll_right();
}

static void editor_left_button_click(struct widget_button *wb)
{
	widget_lvledit_toolbar_scroll_left();
}

static void typeselect_obstacle_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_OBSTACLE);
}

static void typeselect_floor_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_FLOOR);
}

static void typeselect_item_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_ITEM);
}

static void typeselect_enemy_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_ENEMY);
}

static void typeselect_waypoint_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_WAYPOINT);
}

static void typeselect_map_label_button_click(struct widget_button *wb)
{
	lvledit_select_type(OBJECT_MAP_LABEL);
}

static void toggle_waypoints_button_click(struct widget_button *wb)
{
	GameConfig.show_wp_connections = !GameConfig.show_wp_connections;
}

static void zoom_in_button_right_click(struct widget_button *wb)
{
	float zf = lvledit_zoomfact();
	zf += 3.0;
	if (zf > 9.0)
		zf = 3.0;
	if (!lvledit_set_zoomfact(zf)) {
		sprintf(VanishingMessage, _("Zoom factor set to %f."), zf);
	} else {
		sprintf(VanishingMessage, _("Could not change zoom factor."));
	}
	VanishingMessageEndDate = SDL_GetTicks() + 1000;
}

// Level editor buttons' update callbacks
static void toggle_enemies_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.omit_enemies_in_level_editor;
}

static void toggle_tooltips_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !GameConfig.show_lvledit_tooltips;
}

static void toggle_collisions_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !draw_collision_rectangles;
}

static void toggle_waypoints_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !GameConfig.show_wp_connections;
}

static void zoom_in_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = !GameConfig.zoom_is_on;
}

static void toggle_obstacles_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.omit_obstacles_in_level_editor;
}

static void toggle_grid_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = GameConfig.show_grid;
}

static void next_object_button_update(struct widget *w)
{
	w->enabled = level_editor_can_cycle_marked_object();
}

static void delete_obstacle_button_update(struct widget *w)
{
	w->enabled = !selection_empty();
}

static void edit_chest_button_update(struct widget *w)
{
	obstacle *o = single_tile_selection(OBJECT_OBSTACLE);
	if (o) {
		switch (o->type) {
			case ISO_H_CHEST_CLOSED:
			case ISO_V_CHEST_CLOSED:
			case ISO_E_CHEST2_CLOSED:
			case ISO_S_CHEST2_CLOSED:
			case ISO_N_CHEST2_CLOSED:
			case ISO_W_CHEST2_CLOSED:
				w->enabled = 1;
				break;

			default:
				w->enabled = 0;
		} 
	}
	else {
		w->enabled = 0;
	}
}

static void new_obstacle_label_button_update(struct widget *w)
{
	w->enabled = single_tile_selection(OBJECT_OBSTACLE) ? 1 : 0;
}

static void undo_button_update(struct widget *w)
{
	w->enabled = !list_empty(&to_undo);
}

static void redo_button_update(struct widget *w)
{
	w->enabled = !list_empty(&to_redo);
}

static void typeselect_obstacle_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_OBSTACLE);
}

static void typeselect_floor_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_FLOOR);
}

static void typeselect_item_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_ITEM);
}

static void typeselect_enemy_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_ENEMY);
}

static void typeselect_waypoint_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_WAYPOINT);
}

static void typeselect_map_label_button_update(struct widget *w)
{
	WIDGET_BUTTON(w)->active = (selection_type() == OBJECT_MAP_LABEL);
}

// Leveleditor floor layers button callbacks
static void floor_layers_button_click(struct widget_button *wb)
{
	GameConfig.show_all_floor_layers = !GameConfig.show_all_floor_layers;
}

static void floor_layers_button_right_click(struct widget_button *wb)
{
	int next_layer = current_floor_layer + 1;
	if (next_layer >= EditLevel()->floor_layers)
		next_layer = 0;
	action_change_floor_layer(EditLevel(), next_layer);
}

static void floor_layers_button_update(struct widget *w)
{
	w->enabled = selection_type() == OBJECT_FLOOR && EditLevel()->floor_layers > 1;
	WIDGET_BUTTON(w)->active = !GameConfig.show_all_floor_layers;
}

/**
 * This function returns the editor top level widget and creates it if necessary.
 */
struct widget_group *get_lvledit_ui()
{
	if (level_editor_widget_group)
		// Editor UI already initialized.
		return level_editor_widget_group;

	level_editor_widget_group = widget_group_create();
	widget_set_rect(WIDGET(level_editor_widget_group), 0, 0, GameConfig.screen_width, GameConfig.screen_height);
	lvledit_widget_list = &level_editor_widget_group->list;
	
	struct widget *map;

	/* Build our interface */

	/* The map (has to be the first widget in the list) */
	map = widget_lvledit_map_create();
	widget_group_add(level_editor_widget_group, map);	

	/* The toolbar */
	struct widget *widget = widget_lvledit_toolbar_create();
	widget_group_add(level_editor_widget_group, widget);

	struct {
		int btn_index;
		char *text;
		string tooltip_text;
		int  number_of_toggle_states;
		void (*activate_button) (struct widget_button *);
		void (*activate_button_secondary) (struct widget_button *);
		void (*update) (struct widget *);
	} b[] = {
		{LEVEL_EDITOR_UNDO_BUTTON, NULL,
			_("Undo\n\nUse this button to undo your last actions."),
			1, undo_button_click, NULL, undo_button_update},
		{LEVEL_EDITOR_REDO_BUTTON, NULL,
			_("Redo\n\nUse this button to redo an action."),
			1, redo_button_click, NULL, redo_button_update},
		{LEVEL_EDITOR_SAVE_SHIP_BUTTON, NULL,
			_("Save Map\n\nThis button will save your current map over the file '../map/levels.dat' from your current working directory."),
			2, save_ship_button_click, NULL, NULL},
		{LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON, NULL,
			_("Beautify grass button\n\nUse this button to automatically 'beautify' rough edges of the grass-sand tiles. It will apply to the selected floor or, if not applicable, to the entire level.\n\nYou can also use Ctrl-b for this."),
			1, beautify_grass_button_click, NULL, NULL},
		{LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON, NULL,
			_("Delete selected obstacle\n\nUse this button to delete the currently marked obstacle.\n\nYou can also use Ctrl-X for this."),
			1, delete_obstacle_button_click, NULL, delete_obstacle_button_update},
		{LEVEL_EDITOR_NEXT_OBJECT_BUTTON, NULL,
			_("Next object on currently selected tile\n\nUse this button to cycle the currently marked object on this tile.\n\nYou can also use the N key for this."),
			1, next_object_button_click, NULL, next_object_button_update},
		{LEVEL_EDITOR_TOGGLE_WAYPOINT_CONNECTIONS_BUTTON, NULL,
			_("Toggle display waypoint connections\n\nUse this button to toggle between waypoint connections displayed on and off."),
			2, toggle_waypoints_button_click, NULL, toggle_waypoints_button_update},
		{LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON, NULL,
			_("New obstacle label\n\nUse this button to attach a label to the currently marked obstacle.  These obstacle labels can be used to define obstacles to be modified by events.\n"),
			1, new_obstacle_label_button_click, NULL, new_obstacle_label_button_update},
		{LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON, NULL,
			_("New map label\n\nUse this button to attach a new map label to the current cursor position.  These map labels can be used to define starting points for bots and characters or also to define locations for events and triggers."),
			1, new_map_label_button_click, NULL, NULL},
		{LEVEL_EDITOR_EDIT_CHEST_BUTTON, NULL,
			_("Edit chests contents\n\nUse this button to change the contents of the chest. If it is empty the game may generate random items at run time."),
			1, edit_chest_button_click, NULL, edit_chest_button_update},
		{LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON, NULL,
			_("Toggle display enemies\n\nUse this button to toggle between enemies displayed in level editor or enemies hidden in level editor."),
			2, toggle_enemies_button_click, NULL, toggle_enemies_button_update},
		{LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON, NULL,
			_("Toggle display tooltips\n\nUse this button to toggle these annoying help windows on and off."),
			2, toggle_tooltips_button_click, NULL, toggle_tooltips_button_update},
		{LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON, NULL,
			_("Toggle display collision rectangles\n\nUse this button to toggle the visible collision rectangles on and off."),
			2, toggle_collisions_button_click, NULL, toggle_collisions_button_update},
		{LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF, NULL,
			_("Toggle grid mode.\n\nUse this button to toggle grid displaying on and off.\n\nUse right click to change the grid mode."),
			2, toggle_grid_button_click, toggle_grid_button_right_click, toggle_grid_button_update},
		{LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON, NULL,
			_("Toggle display obstacles\n\nUse this button to toggle between obstacles displayed in level editor or obstacles hidden in level editor."),
			2, toggle_obstacles_button_click, NULL, toggle_obstacles_button_update},
		{LEVEL_EDITOR_ZOOM_IN_BUTTON, NULL,
			_("Zoom in/out\n\nUse this button to zoom INTO or OUT of the level.\n\nUse right click to change the zoom ratio.\n"),
			2, zoom_in_button_click, zoom_in_button_right_click, zoom_in_button_update},
		{LEVEL_EDITOR_ALL_FLOOR_LAYERS_BUTTON, NULL,
			_("Toggle floor layers\n\nUse this button to toggle between all floor layers displayed or single floor layer displayed.\n\nUse right click to change the current floor layer.\n"),
			2, floor_layers_button_click, floor_layers_button_right_click, floor_layers_button_update},
		{LEVEL_EDITOR_QUIT_BUTTON, NULL,
			_("Test Map\n\nThis will save your map and reload it after you finish testing, avoiding saving an unclean world state."),
			1, quit_button_click, NULL, NULL},
		{RIGHT_LEVEL_EDITOR_BUTTON, NULL, NULL,
			1, editor_right_button_click, NULL},
		{LEFT_LEVEL_EDITOR_BUTTON, NULL, NULL,
			1, editor_left_button_click, NULL, NULL},
		{LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON, "OBSTACLE", NULL,
			2, typeselect_obstacle_button_click, NULL, typeselect_obstacle_button_update},
		{LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON, "FLOOR", NULL,
			2, typeselect_floor_button_click, NULL, typeselect_floor_button_update},
		{LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON, "ITEM", NULL,
			2, typeselect_item_button_click, NULL, typeselect_item_button_update},
		{LEVEL_EDITOR_TYPESELECT_ENEMY_BUTTON, "ENEMY", NULL,
			2, typeselect_enemy_button_click, NULL, typeselect_enemy_button_update},
		{LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON, "WP", NULL,
			2, typeselect_waypoint_button_click, NULL, typeselect_waypoint_button_update},
		{LEVEL_EDITOR_TYPESELECT_MAP_LABEL_BUTTON, "LABEL", NULL,
			2, typeselect_map_label_button_click, NULL, typeselect_map_label_button_update}
	};

	int i, j;

	// Build buttons using the array above.
	struct widget_button *button;
	for (i = 0; i < sizeof(b) / sizeof(b[0]); i++) {
		button = widget_button_create();

		// Set text and tooltip.
		button->text = b[i].text; 
		button->tooltip.text = b[i].tooltip_text; 
		
		// Set callbacks.
		button->activate_button = b[i].activate_button;
		button->activate_button_secondary = b[i].activate_button_secondary;
		WIDGET(button)->update = b[i].update;

		// Load images using AllMousePressButtons.
		// Buttons use 3 images for each toggle state. Radio buttons have two toggle states.
		for (j = 0; j < b[i].number_of_toggle_states; j++) {
			// AllMousePressButtons have only two images: normal and pressed. Hovered state will be ignored.
			button->image[j][0] = widget_load_image_resource(AllMousePressButtons[b[i].btn_index + j * 2].button_image_file_name, 0);		// Normal state
			button->image[j][1] = NULL;												// Hovered state
			button->image[j][2] = widget_load_image_resource(AllMousePressButtons[b[i].btn_index + j * 2 + 1].button_image_file_name, 0);	// Pressed state
		}
		
		// Set button size.
		SDL_Rect rect = AllMousePressButtons[b[i].btn_index].button_rect;	

		// Size can be specified in AllMousePressButtons or left to 0, meaning the actual image size must be used.
		rect.w = (rect.w) ? rect.w : button->image[0][0]->w;
		rect.h = (rect.h) ? rect.h : button->image[0][0]->h;
		widget_set_rect(WIDGET(button), rect.x, rect.y, rect.w, rect.h);
		
		//Add the button to the level editor top level group.
		widget_group_add(level_editor_widget_group, WIDGET(button));
	}

	lvledit_build_tile_lists();

	// Load categories for obstacles
	char fpath[4096];
	find_file("leveleditor_obstacle_categories.lua", MAP_DIR, fpath, 0);
	run_lua_file(LUA_CONFIG, fpath);

	// Build list of all obstacles
	all_obstacles_list = MyMalloc(sizeof(int) * (obstacle_map.size + 1));
	for (i = 0, j = 0; i < obstacle_map.size; i++) {
		// Dummy obstacles (obstacles without image) should not be displayed in the
		// level editor obstacle list.
		if (!image_loaded(get_obstacle_image(i, 0)))
			continue;
		all_obstacles_list[j++] = i;
	}
	all_obstacles_list[j] = -1;

	// Create category selectors 
	for (i = 0; i < sizeof(category_list) / sizeof(category_list[0]); i++) {
		enum lvledit_object_type type = category_list[i].object_type;
		object_category *categories = category_list[i].categories;
		for (j = 0; j < category_list[i].length; j++) {
			struct widget *widget = widget_lvledit_categoryselector_create(j, _(categories[j].name), type, *categories[j].object_list);
			categories[j].cs = widget->ext;
			widget_group_add(level_editor_widget_group, widget);
		}
	}

	// Create the minimap
	widget = widget_lvledit_minimap_create();
	widget_group_add(level_editor_widget_group, widget);
	
	// Activate the obstacle type selector
	lvledit_select_type(OBJECT_OBSTACLE);
	
	widget_lvledit_map_init();

	return level_editor_widget_group;
}

struct widget *get_active_widget(int x, int y)
{
	struct widget *w;
	list_for_each_entry_reverse(w, lvledit_widget_list, node) {
		if (!w->enabled)
			continue;
		if (MouseCursorIsInRect(&w->rect, x, y)) {
			return w;
		}
	}

	return NULL;
}

void lvledit_select_type(enum lvledit_object_type type)
{
	struct widget *cs_widget;

	if (selection_type() != type) {
		// When the user changes the current object type selected, reset tools
		lvledit_reset_tools();
	}

	// Find the categories which must be enabled
 	list_for_each_entry_reverse(cs_widget, lvledit_widget_list, node) {
 		if (cs_widget->type != WIDGET_CATEGORY_SELECTOR)
 			continue;

		// By default, desactivate all categories
		cs_widget->enabled = 0;		
		
		struct widget_lvledit_categoryselect *cs = cs_widget->ext;

		// Activate a category if it is of the right type
		if (cs->type == type) {
			cs_widget->enabled = 1;

			widget_lvledit_categoryselect_activate(cs);
		}
	}
}

void lvledit_categoryselect_switch(int direction)
{
	// Find a category list for the current selection type
	object_category *categories = NULL;
	int i, length = 0;
	for (i = 0; i < sizeof(category_list) / sizeof(category_list[0]); i++) {
		if (selection_type() == category_list[i].object_type) {
			categories = category_list[i].categories;
			length = category_list[i].length;
			break;
		}
	}

	// Find the current category
	for (i = 0; i < length; i++) {
		if (get_current_object_type() == categories[i].cs)
			break;
	}

	// Switch the current category according to the direction
	int index = i + direction;
	if (index < 0)
		index = length - 1;
	index %= length;
	widget_lvledit_categoryselect_activate(categories[index].cs);
}
