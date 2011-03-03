/* 
 *
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 * This file contains miscellaeous helpful functions for Freedroid.
 */
#define _misc_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "savestruct.h"

//--------------------
// This header file is needed
#if HAVE_EXECINFO_H
#  include <execinfo.h>
#endif
#if HAVE_SIGNAL_H
#  include <signal.h>
#endif

long oneframedelay = 0;
long tenframedelay = 0;
long onehundredframedelay = 0;
float FPSover1 = 10;
float FPSover10 = 10;
float FPSover100 = 10;
Uint32 Now_SDL_Ticks;
Uint32 One_Frame_SDL_Ticks;
Uint32 Ten_Frame_SDL_Ticks;
Uint32 Onehundred_Frame_SDL_Ticks;
int framenr = 0;

char *our_homedir = NULL;
char *our_config_dir = NULL;

mouse_press_button AllMousePressButtons[MAX_MOUSE_PRESS_BUTTONS] = {
	[LOG_SCREEN_TOGGLE_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {239, 480 - 47, 46, 14}, TRUE},
	[CHA_SCREEN_TOGGLE_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {334, 480 - 47, 46, 14}, TRUE},
	[INV_SCREEN_TOGGLE_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {283, 480 - 47, 46, 14}, TRUE},
	[SKI_SCREEN_TOGGLE_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {378, 480 - 47, 46, 14}, TRUE},
	[CHA_SCREEN_TOGGLE_BUTTON_RED] = {EMPTY_IMAGE, "mouse_buttons/cha_button_red.png", {334, 480 - 47, 46, 14}, TRUE},
	[LOG_SCREEN_TOGGLE_BUTTON_YELLOW] =
	    {EMPTY_IMAGE, "mouse_buttons/log_button_yellow.png", {239, 480 - 47, 46, 14}, TRUE},
	[LOG_SCREEN_TOGGLE_BUTTON_RED] =
	    {EMPTY_IMAGE, "mouse_buttons/log_button_red.png", {239, 480 - 47, 46, 14}, TRUE},
	[CHA_SCREEN_TOGGLE_BUTTON_YELLOW] =
	    {EMPTY_IMAGE, "mouse_buttons/cha_button_yellow.png", {334, 480 - 47, 46, 14}, TRUE},
	[INV_SCREEN_TOGGLE_BUTTON_YELLOW] =
	    {EMPTY_IMAGE, "mouse_buttons/inv_button_yellow.png", {283, 480 - 47, 46, 14}, TRUE},
	[SKI_SCREEN_TOGGLE_BUTTON_YELLOW] =
	    {EMPTY_IMAGE, "mouse_buttons/ski_button_yellow.png", {378, 480 - 47, 46, 14}, TRUE},
	[UP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/UPButton.png", {600, 94, 40, 40}, TRUE},
	[DOWN_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/DOWNButton.png", {600, 316, 40, 40}, TRUE},

	[ITEM_BROWSER_LEFT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {280, 44, 37, 37}, TRUE},
	[ITEM_BROWSER_RIGHT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {536, 44, 37, 37}, TRUE},
	[ITEM_BROWSER_EXIT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {201, 340, 47, 47}, TRUE},

	[LEFT_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/LeftShopButton.png", {23, 446, 23, 23}, TRUE},
	[RIGHT_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/RightShopButton.png", {580, 447, 23, 23}, TRUE},
	[LEFT_TUX_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/LeftShopButton.png", {6, 15, 23, 23}, TRUE},
	[RIGHT_TUX_SHOP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/RightShopButton.png", {584, 15, 23, 23}, TRUE},
	[LEFT_LEVEL_EDITOR_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorLeft.png", {3, 8, 15, 60}, FALSE},
	[LEFT_LEVEL_EDITOR_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorLeft.png", {3, 8, 15, 60}, FALSE},
	[RIGHT_LEVEL_EDITOR_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorRight.png", {-16, 8, 15, 60}, FALSE},
	[RIGHT_LEVEL_EDITOR_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorObjectSelectorRight.png", {-16, 8, 15, 60}, FALSE},

	[NUMBER_SELECTOR_OK_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/number_selector_ok_button.png", {308, 288, 48, 48}, TRUE},
	[NUMBER_SELECTOR_LEFT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {148, 244, 35, 35}, TRUE},
	[NUMBER_SELECTOR_RIGHT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {404, 244, 35, 35}, TRUE},

	[BUY_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/buy_button.png", {199, 98, 47, 47}, TRUE},
	[SELL_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/sell_button.png", {199, 153, 47, 47}, TRUE},
	[REPAIR_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/repair_button.png", {199, 225, 47, 47}, TRUE},

	[OPEN_CLOSE_SKILL_EXPLANATION_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {0 + 17, 424, 33, 33}, FALSE},


/* lower right area next to the mini map*/
	[LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorDeleteObstacleButton.png", {-270, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_DELETE_OBSTACLE_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorDeleteObstacleButtonPushed.png", {-270, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNextObstacleButton.png", {-240, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEXT_OBSTACLE_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNextObstacleButtonPushed.png", {-240, -30, 0, 0}, FALSE},


/* upper right are directly under the object selector*/
	[LEVEL_EDITOR_ESC_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorESCButton.png", {-90, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_ESC_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorESCButtonPushed.png", {-90, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButton.png", {-60, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButtonPushed.png", {-60, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButtonOff.png", {-60, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_SAVE_SHIP_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorSaveShipButtonOffPushed.png", {-60, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_QUIT_BUTTON] =
		{EMPTY_IMAGE, "mouse_buttons/LevelEditorQuitButton.png", {-30, 80, 0, 0}, FALSE},
	[LEVEL_EDITOR_QUIT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorQuitButtonPushed.png", {-30, 80, 0, 0}, FALSE},


/* above the obstacle selectors, upper row */
	[LEVEL_EDITOR_EDIT_CHEST_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorEditChestButton.png", {-150, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_EDIT_CHEST_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorEditChestButtonPushed.png", {-150, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNewObstacleLabelButton.png", {-120, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEW_OBSTACLE_LABEL_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNewObstacleLabelButtonPushed.png", {-120, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNewMapLabelButton.png", {-90, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_NEW_MAP_LABEL_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNewMapLabelButtonPushed.png", {-90, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_IN_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomInButton.png", {-60, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_IN_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomInButtonPushed.png", {-60, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_OUT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomOutButton.png", {-60, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_ZOOM_OUT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorZoomOutButtonPushed.png", {-60, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorBeautifyGrassButton.png", {-30, -350, 0, 0}, FALSE},
	[LEVEL_EDITOR_BEAUTIFY_GRASS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorBeautifyGrassButtonPushed.png", {-30, -350, 0, 0}, FALSE},


/* above the obstacle selector, lower row*/
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonOff.png", {-150, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonOffPushed.png", {-150, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButton.png", {-150, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonPushed.png", {-150, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonFull.png", {-150, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_GRID_BUTTON_FULL_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleGridButtonFullPushed.png", {-150, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButton.png", {-120, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButtonPushed.png", {-120, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButtonOff.png", {-120, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_ENEMIES_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleEnemiesButtonOffPushed.png", {-120, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButton.png", {-90, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButtonPushed.png", {-90, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButtonOff.png", {-90, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_OBSTACLES_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleObstaclesButtonOffPushed.png", {-90, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButton.png", {-60, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButtonPushed.png", {-60, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButtonOff.png", {-60, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_TOOLTIPS_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleTooltipsButtonOffPushed.png", {-60, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButton.png", {-30, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButtonPushed.png", {-30, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButtonOff.png", {-30, -320, 0, 0}, FALSE},
	[LEVEL_EDITOR_TOGGLE_COLLISION_RECTS_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorToggleCollisionRectsButtonOffPushed.png", {-30, -320, 0, 0}, FALSE},


	[LEVEL_EDITOR_NEXT_ITEM_GROUP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorNextItemGroup.png", {55 + 64 * 8, 32 + 5 * 66, 0, 0}, TRUE},
	[LEVEL_EDITOR_PREV_ITEM_GROUP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorPrevItemGroup.png", {55, 32 + 5 * 66, 0, 0}, TRUE},

	[LEVEL_EDITOR_CANCEL_ITEM_DROP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorCancelItemDrop.png", {55 + 80, 32 + 5 * 66, 0, 0}, TRUE},
	[LEVEL_EDITOR_UNDO_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorUndoButton.png", {-330, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_UNDO_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorUndoButtonPushed.png", {-330, -30, 0, 0}, FALSE},

	[LEVEL_EDITOR_REDO_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorRedoButton.png", {-300, -30, 0, 0}, FALSE},
	[LEVEL_EDITOR_REDO_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorRedoButtonPushed.png", {-300, -30, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton.png", {-152, -290, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonPushed.png", {-152, -290, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOff.png", {-152, -290, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_OBSTACLE_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOffPushed.png", {-152, -290, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton.png", {-152, -252, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonPushed.png", {-152, -252, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOff.png", {-152, -252, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_FLOOR_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOffPushed.png", {-152, -252, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton.png", {-152, -214, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonPushed.png", {-152, -214, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOff.png", {-152, -214, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_ITEM_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOffPushed.png", {-152, -214, 0, 0}, FALSE},

	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButton.png", {-152, -176, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonPushed.png", {-152, -176, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON_OFF] =
		 {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOff.png", {-152, -176, 0, 0}, FALSE},
	[LEVEL_EDITOR_TYPESELECT_WAYPOINT_BUTTON_OFF_PUSHED] =
	    {EMPTY_IMAGE, "mouse_buttons/LevelEditorTypeSelectorButtonOffPushed.png", {-152, -176, 0, 0}, FALSE},

	[WEAPON_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {WEAPON_RECT_X, WEAPON_RECT_Y, WEAPON_RECT_WIDTH, WEAPON_RECT_HEIGHT}, FALSE},
	[DRIVE_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {DRIVE_RECT_X, DRIVE_RECT_Y, DRIVE_RECT_WIDTH, DRIVE_RECT_HEIGHT}, FALSE},
	[SHIELD_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {SHIELD_RECT_X, SHIELD_RECT_Y, SHIELD_RECT_WIDTH, SHIELD_RECT_HEIGHT}, FALSE},
	[HELMET_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {HELMET_RECT_X, HELMET_RECT_Y, HELMET_RECT_WIDTH, HELMET_RECT_HEIGHT}, FALSE},
	[ARMOUR_RECT_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {ARMOUR_RECT_X, ARMOUR_RECT_Y, ARMOUR_RECT_WIDTH, ARMOUR_RECT_HEIGHT}, FALSE},

	[SCROLL_DIALOG_MENU_UP_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuUp.png", {235, (480 - 20 - 130 - 20 - 2), 160, 20}, TRUE},
	[SCROLL_DIALOG_MENU_DOWN_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuDown.png", {235, (480 - 22), 160, 20}, TRUE},

	[MORE_STR_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, STR_Y - 5, 38, 22}, FALSE},
	[MORE_MAG_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, MAG_Y - 5, 38, 22}, FALSE},
	[MORE_DEX_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, DEX_Y - 5, 38, 22}, FALSE},
	[MORE_VIT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/AttributePlusButton.png", {0 + STR_X + 53, VIT_Y - 5, 38, 22}, FALSE},

	// These two buttons are for the scrolling text during the
	// title display, the credits menu and the level editor 
	// keyboard explanation...
	//
	[SCROLL_TEXT_UP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/arrow_up_for_scroll_text.png", {-65, 10, 73, 98}, FALSE},
	[SCROLL_TEXT_DOWN_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/arrow_down_for_scroll_text.png", {-65, -10 - 98, 73, 98}, FALSE},

	[DESCRIPTION_WINDOW_UP_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {607, 99, 26, 26}, TRUE},
	[DESCRIPTION_WINDOW_DOWN_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {607, 347, 26, 26}, TRUE},

	[DROID_SHOW_EXIT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {202, 311, 47, 47}, TRUE},

	// These are the scrollbuttons for the chat protocal inside the
	// chat window, like when talking to a character/bot.
	//
	[CHAT_LOG_SCROLL_UP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuUp.png", {342, 3, 160, 20}, TRUE},
	[CHAT_LOG_SCROLL_DOWN_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuDown.png", {342, 272, 160, 20}, TRUE},
	[CHAT_LOG_SCROLL_OFF_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuOff.png", {342, 3, 160, 20}, TRUE},
	[CHAT_LOG_SCROLL_OFF2_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuOff.png", {342, 272, 160, 20}, TRUE},

	[QUEST_BROWSER_EXIT_BUTTON] = {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {486, 324, 73, 73}, TRUE},
	[QUEST_BROWSER_OPEN_QUESTS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_open_quests.png", {473, 97, 153, 38}, TRUE},
	[QUEST_BROWSER_OPEN_QUESTS_OFF_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_open_quests_off.png", {473, 97, 153, 38}, TRUE},
	[QUEST_BROWSER_DONE_QUESTS_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_done_quests.png", {478, 149, 153, 38}, TRUE},
	[QUEST_BROWSER_DONE_QUESTS_OFF_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_done_quests_off.png", {478, 149, 153, 38}, TRUE},
	[QUEST_BROWSER_NOTES_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/quest_browser_notes.png", {478, 203, 153, 38}, TRUE},
	[QUEST_BROWSER_NOTES_OFF_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_notes_off.png", {478, 203, 153, 38}, TRUE},
	[QUEST_BROWSER_SCROLL_UP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuUp.png", {181, 12, 160, 20}, TRUE},
	[QUEST_BROWSER_SCROLL_DOWN_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/ScrollDialogMenuDown.png", {181, 452, 160, 20}, TRUE},
	[QUEST_BROWSER_ITEM_SHORT_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_item_short.png", {108, 86, 300, 26}, FALSE},
	[QUEST_BROWSER_ITEM_LONG_BUTTON] =
	    {EMPTY_IMAGE, "mouse_buttons/quest_browser_item_long.png", {108, 86, 300, 26}, FALSE},

	[TAKEOVER_HELP_BUTTON] = {EMPTY_IMAGE, "mouse_buttons/takeover_help_button.png", {78, 23, 153, 38}, FALSE},

	// This button is for changing the current weapon mode/reloading
	[WEAPON_MODE_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {CURRENT_WEAPON_RECT_X, 400, CURRENT_WEAPON_RECT_W, CURRENT_WEAPON_RECT_H}, TRUE},
	[SKI_ICON_BUTTON] =
	    {EMPTY_IMAGE, "THIS_DOESNT_NEED_BLITTING", {CURRENT_SKILL_RECT_X, 400, CURRENT_SKILL_RECT_W, CURRENT_SKILL_RECT_H}, TRUE},

	// Buttons of the item upgrade UI.
	[ITEM_UPGRADE_APPLY_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_apply.png",
	    { ITEM_UPGRADE_RECT_X + 250, ITEM_UPGRADE_RECT_Y + 389, 48, 48 }, FALSE},
	[ITEM_UPGRADE_APPLY_BUTTON_DISABLED] = { EMPTY_IMAGE, "item_upgrade/button_apply_disabled.png",
	    { ITEM_UPGRADE_RECT_X + 250, ITEM_UPGRADE_RECT_Y + 389, 48, 48 }, FALSE},
	[ITEM_UPGRADE_CLOSE_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_close.png",
	    { ITEM_UPGRADE_RECT_X + 215, ITEM_UPGRADE_RECT_Y + 406, 32, 32 }, FALSE},

	// Buttons of the add-on crafting UI.
	[ADDON_CRAFTING_APPLY_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_apply.png",
	    { ADDON_CRAFTING_RECT_X + 250, ADDON_CRAFTING_RECT_Y + 389, 48, 48 }, FALSE},
	[ADDON_CRAFTING_APPLY_BUTTON_DISABLED] = { EMPTY_IMAGE, "item_upgrade/button_apply_disabled.png",
	    { ADDON_CRAFTING_RECT_X + 250, ADDON_CRAFTING_RECT_Y + 389, 48, 48 }, FALSE},
	[ADDON_CRAFTING_CLOSE_BUTTON] = { EMPTY_IMAGE, "item_upgrade/button_close.png",
	    { ADDON_CRAFTING_RECT_X + 215, ADDON_CRAFTING_RECT_Y + 406, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_UP_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_up.png",
	    { ADDON_CRAFTING_RECT_X + 264, ADDON_CRAFTING_RECT_Y + 70, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_DOWN_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_down.png",
	    { ADDON_CRAFTING_RECT_X + 264, ADDON_CRAFTING_RECT_Y + 198, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_DESC_UP_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_up.png",
	    { ADDON_CRAFTING_RECT_X + 260, ADDON_CRAFTING_RECT_Y + 290, 32, 32 }, FALSE},
	[ADDON_CRAFTING_SCROLL_DESC_DOWN_BUTTON] = { EMPTY_IMAGE, "mouse_buttons/crafting_scroll_down.png",
	    { ADDON_CRAFTING_RECT_X + 260, ADDON_CRAFTING_RECT_Y + 350, 32, 32 }, FALSE},

};				// mouse_press_button AllMousePressButtons[ MAX_MOUSE_PRESS_BUTTONS ] 

//--------------------
// We make these global variables here, as we might want to use
// this function inside a signal handler and maybe also it's better
// not to mess too much around with the stack while trying to read
// out the stack...
//
#define MAX_CALLS_IN_BACKTRACE 200
void *backtrace_array[MAX_CALLS_IN_BACKTRACE];
size_t backtrace_size;
char **backtrace_strings;
size_t backtrace_counter;

#ifndef __WIN32__
struct sigaction new_action, old_action;
#endif

/** 
 * Obtain a backtrace and print it to stdout.
 * If signum != 0, call Terminate()
 */
void print_trace(int signum)
{

#if (!defined __WIN32__) && (!defined __APPLE_CC__) && (defined HAVE_BACKTRACE)

	// fprintf ( stderr , "print_trace:  Now attempting backtrace from within the code!\n" );
	// fprintf ( stderr , "print_trace:  Allowing a maximum of %d function calls on the stack!\n" , MAX_CALLS_IN_BACKTRACE );

	// We attempt to get a backtrace of all function calls so far, even
	// including the operating system (or rather libc) call to main() in 
	// the beginning of execution.
	//
	backtrace_size = backtrace(backtrace_array, MAX_CALLS_IN_BACKTRACE);

	fprintf(stderr, "print_trace:  Obtained %zd stack frames.\n", backtrace_size);

	// Now we attempt to translate the trace information we've got to the
	// symbol names that might still reside in the binary.
	//
	// NOTE: that in order for this to work, the -rdynamic switch must have
	//       been passed as on option to the LINKER!
	//       Also there might be a problem with non-ELF binaries, but let's
	//       hope that it still works...
	//
	backtrace_strings = backtrace_symbols(backtrace_array, backtrace_size);

	fprintf(stderr, "print_trace:  Obtaining symbols now done.\n");

	for (backtrace_counter = 0; backtrace_counter < backtrace_size; backtrace_counter++)
		fprintf(stderr, "%s\n", backtrace_strings[backtrace_counter]);

	// The strings generated in the backtrace_symbols function need to 
	// get freed.  Well, this isn't terribly important, but clean.
	//
	free(backtrace_strings);

#endif

	switch (signum) {
	case 0:
		return;
		break;
	case SIGSEGV:
		fprintf(stderr, "\n%s():  received SIGSEGV!\n", __FUNCTION__);
		break;
	case SIGFPE:
		fprintf(stderr, "\n%s():  received SIGFPE!\n", __FUNCTION__);
		break;
	default:
		fprintf(stderr, "\n%s():  received UNKNOWN SIGNAL %d!  ERROR! \n", __FUNCTION__, signum);
	}

	Terminate(EXIT_FAILURE, TRUE);

};				// void print_trace ( int sig_num )

/** 
 * In this function, we move the normal SIGSEGV handler (and other signal
 * handlers to our own handler stuff, which will print out a backtrace
 * of the preceeding function calls, so that we get some suitable debug
 * output, even if there was no debugger used when starting the game.
 *
 * This might not be completely portable to win32 systems.  Don't know if
 * it's our fault or not, but maybe we will have to disable this piece
 * of code via simple conditional compilation if the target is win32.
 *
 * For more documentation, see the GLIBC manual, Section 24.3.4 on signal
 * handling.
 *
 */
void implant_backtrace_into_signal_handlers(void)
{

#if (!defined __WIN32__) && (!defined __APPLE_CC__)

	DebugPrintf(-4, "\n-Signal Handling------------------------------------------------------\n\
Setting up signal handlers for internal backtrace:\n\
Now catching SIGSEGV: ");

	// We set up the structure for the new signal handling
	// to give to the operating system
	//
	new_action.sa_handler = print_trace;
	sigemptyset(&new_action.sa_mask);
	new_action.sa_flags = 0;

	// Now it's time to activate the new signal handling...
	//
	sigaction(SIGSEGV, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN) {
		DebugPrintf(-4, "YES");
		sigaction(SIGSEGV, &new_action, NULL);
	} else {
		DebugPrintf(-4, "NO");
	}

	// 
	DebugPrintf(-4, "\nNow catching FPE (if raised, that is!): ");
	sigaction(SIGFPE, NULL, &old_action);
	if (old_action.sa_handler != SIG_IGN) {
		DebugPrintf(-4, "YES");
		sigaction(SIGFPE, &new_action, NULL);
	} else {
		DebugPrintf(-4, "NO");
	}
	DebugPrintf(-4, "\n\n");

#endif

};				// void implant_backtrace_into_signal_handlers ( void )

/**
 * If we want the screen resolution to be a runtime option and not a 
 * compile time option any more, we must not use it as a constant.  That
 * means we must adapt the button positions to the current screen 
 * resolution at runtime to, so we do it in this function, which will be
 * involved at program startup.
 */
void adapt_button_positions_to_screen_resolution(void)
{
	int i;

	for (i = 0; i < MAX_MOUSE_PRESS_BUTTONS; i++) {
		if (AllMousePressButtons[i].button_rect.x < 0)
			AllMousePressButtons[i].button_rect.x += GameConfig.screen_width;
		if (AllMousePressButtons[i].button_rect.y < 0)
			AllMousePressButtons[i].button_rect.y += GameConfig.screen_height;
	}

	AllMousePressButtons[OPEN_CLOSE_SKILL_EXPLANATION_BUTTON].button_rect.x += CHARACTERRECT_X;

	AllMousePressButtons[MORE_STR_BUTTON].button_rect.x += CHARACTERRECT_X;
	AllMousePressButtons[MORE_MAG_BUTTON].button_rect.x += CHARACTERRECT_X;
	AllMousePressButtons[MORE_DEX_BUTTON].button_rect.x += CHARACTERRECT_X;
	AllMousePressButtons[MORE_VIT_BUTTON].button_rect.x += CHARACTERRECT_X;

	Droid_Image_Window.x = 48 * GameConfig.screen_width / 640;
	Droid_Image_Window.y = 44 * GameConfig.screen_height / 480;
	Droid_Image_Window.w = 130 * GameConfig.screen_width / 640;
	Droid_Image_Window.h = 172 * GameConfig.screen_height / 480;

	Full_User_Rect.x = 0;
	Full_User_Rect.y = 0;
	Full_User_Rect.w = GameConfig.screen_width;
	Full_User_Rect.h = GameConfig.screen_height;

	Cons_Text_Rect.x = 175;
	Cons_Text_Rect.y = 180;
	Cons_Text_Rect.w = GameConfig.screen_width - 175;
	Cons_Text_Rect.h = 305;

};				// void adapt_button_positions_to_screen_resolution( void )

/**
 * This is a useful utility sub-function for the checks whether the
 * mouse cursor is on an enemy, a closed chest or a barrel, or any other object.
 * The function detects if the current mouse cursor is over the graphics
 * mentioned in that iso image, using the given position from the
 * parameter list.
 *
 * TRUE or FALSE is returned, depending on whether the cursor IS or
 * IS NOT on that particular iso_image, if positioned on that given spot.
 */
int mouse_cursor_is_on_that_image(float pos_x, float pos_y, struct image *our_image)
{
	// our_iso_image = & ( enemy_iso_images [ RotationModel ] [ RotationIndex ] [ (int) this_bot -> animation_phase ] ) ;
	SDL_Rect screen_rectangle;

	screen_rectangle.x = translate_map_point_to_screen_pixel_x(pos_x, pos_y) + our_image->offset_x;
	screen_rectangle.y = translate_map_point_to_screen_pixel_y(pos_x, pos_y) + our_image->offset_y;
	screen_rectangle.w = our_image->w;
	screen_rectangle.h = our_image->h;

	if (MouseCursorIsInRect(&(screen_rectangle),
				input_axis.x + User_Rect.w / 2 + User_Rect.x, input_axis.y + User_Rect.h / 2 + User_Rect.y)) {
		return (TRUE);
	}

	return (FALSE);
}

/**
 * This function checks if a given screen position lies within the 
 * inventory screen toggle button or not.
 */
int MouseCursorIsInRect(const SDL_Rect *our_rect, int x, int y)
{
	// Now we can start to check if the mouse cursor really is on that
	// rectangle or not.
	//
	if (x > our_rect->x + our_rect->w)
		return (FALSE);
	if (x < our_rect->x)
		return (FALSE);
	if (y > our_rect->y + our_rect->h)
		return (FALSE);
	if (y < our_rect->y)
		return (FALSE);

	// So since the cursor is not outside of this rectangle, it must
	// we inside, and so we'll return this answer.
	//
	return (TRUE);

};				// int MouseCursorIsInRect( SDL_rect* our_rect , int x , int y )

/**
 * This function checks if a given screen position lies within the 
 * inventory screen toggle button or not.
 */
int MouseCursorIsOnButton(int ButtonIndex, int x, int y)
{
	SDL_Rect temp_rect;

	// First a sanity check if the button index given does make
	// some sense.
	//
	if ((ButtonIndex >= MAX_MOUSE_PRESS_BUTTONS) || (ButtonIndex < 0)) {
		ErrorMessage(__FUNCTION__, "\
A Button that should be checked for mouse contact was requested, but the\n\
button index given exceeds the number of buttons defined in freedroid.", PLEASE_INFORM, IS_FATAL);
	}

	Copy_Rect(AllMousePressButtons[ButtonIndex].button_rect, temp_rect);
	// If this button needs scaling still, then we do it now...
	//
	if (AllMousePressButtons[ButtonIndex].scale_this_button) {
		temp_rect.x *= ((float)GameConfig.screen_width) / 640.0;
		temp_rect.w *= ((float)GameConfig.screen_width) / 640.0;
		temp_rect.y *= ((float)GameConfig.screen_height) / 480.0;
		temp_rect.h *= ((float)GameConfig.screen_height) / 480.0;
	}

	if (y < AllMousePressButtons[ButtonIndex].button_rect.y)
		return (FALSE);

	// So since the cursor is not outside of this rectangle, it must
	// we inside, and so we'll return this answer.
	//
	return (MouseCursorIsInRect(&(temp_rect), x, y));

};				// int MouseCursorIsOnButton( int ButtonIndex , int x , int y )

/**
 * This function blits a button to the screen.  The button must have been
 * defined prior to this in the above button list.
 */
/**
 * This function blits a button to the screen.  The button must have been
 * defined prior to this in the above button list.
 */
void ShowGenericButtonFromList(int ButtonIndex)
{
	SDL_Surface *tmp;
	char fpath[2048];
	SDL_Rect Temp_Blitting_Rect;

	// First a sanity check if the button index given does make
	// some sense.
	//
	//
	if ((ButtonIndex >= MAX_MOUSE_PRESS_BUTTONS) || (ButtonIndex < 0)) {
		ErrorMessage(__FUNCTION__, "Request to display button index %d could not be fulfilled: the\n\
				button index given exceeds the number of buttons defined in freedroid.", PLEASE_INFORM, IS_FATAL, ButtonIndex);
	}
	// Now check if this button needs blitting, and if not, we do the scaling once
	// and disable the scaling ever afterwards...
	//
	if (!strcmp(AllMousePressButtons[ButtonIndex].button_image_file_name, "THIS_DOESNT_NEED_BLITTING")) {
		return;
	}
	// Now we check if we have to load the button image still
	// or if it is perhaps already loaded into memory.
	//
	if (!image_loaded(&AllMousePressButtons[ButtonIndex].button_image)) {
		find_file(AllMousePressButtons[ButtonIndex].button_image_file_name, GRAPHICS_DIR, fpath, 0);
		tmp = our_IMG_load_wrapper(fpath);
		if (tmp == NULL) {
			fprintf(stderr, "\nfpath: %s.\nButton Index: %d.\n", fpath, ButtonIndex);
			ErrorMessage(__FUNCTION__, "\
					An image file for a button that should be displayed on the screen couldn't\n\
					be successfully loaded into memory.\n\
					This is an indication of a severe bug/installation problem of freedroid.", PLEASE_INFORM, IS_WARNING_ONLY);
			return;
		}
		AllMousePressButtons[ButtonIndex].button_image.surface = our_SDL_display_format_wrapperAlpha(tmp);
		SDL_FreeSurface(tmp);

		if (AllMousePressButtons[ButtonIndex].scale_this_button) {
			tmp =
			    zoomSurface(AllMousePressButtons[ButtonIndex].button_image.surface, ((float)GameConfig.screen_width) / 640.0,
					((float)GameConfig.screen_height) / 480.0, TRUE);
			SDL_FreeSurface(AllMousePressButtons[ButtonIndex].button_image.surface);
			AllMousePressButtons[ButtonIndex].button_image.surface = our_SDL_display_format_wrapperAlpha(tmp);
			SDL_FreeSurface(tmp);

			AllMousePressButtons[ButtonIndex].button_rect.x *= ((float)GameConfig.screen_width) / 640.0;
			AllMousePressButtons[ButtonIndex].button_rect.w *= ((float)GameConfig.screen_width) / 640.0;
			AllMousePressButtons[ButtonIndex].button_rect.y *= ((float)GameConfig.screen_height) / 480.0;
			AllMousePressButtons[ButtonIndex].button_rect.h *= ((float)GameConfig.screen_height) / 480.0;
			AllMousePressButtons[ButtonIndex].scale_this_button = FALSE;
		}
		// Maybe we had '0' entries for the height or width of this button in the list.
		// This means that we will take the real width and the real height from the image
		// and overwrite the 0 entries with this.
		//
		if (AllMousePressButtons[ButtonIndex].button_rect.w == (0)) {
			AllMousePressButtons[ButtonIndex].button_rect.w = AllMousePressButtons[ButtonIndex].button_image.surface->w;
		}
		if (AllMousePressButtons[ButtonIndex].button_rect.h == (0)) {
			AllMousePressButtons[ButtonIndex].button_rect.h = AllMousePressButtons[ButtonIndex].button_image.surface->h;
		}
		// With OpenGL output method, we'll make a texture for faster and 
		// better blitting.
		//
		if (use_open_gl) {
			make_texture_out_of_surface(&(AllMousePressButtons[ButtonIndex].button_image));
		}
	}

	// Now that we know we have the button image loaded, we can start
	// to blit the button image to the screen.
	//
	// But in order not to damage the original rect data, we use the
	// temp value as parameter for the SDL_Blit thing..
	//
	Copy_Rect(AllMousePressButtons[ButtonIndex].button_rect, Temp_Blitting_Rect);

	display_image_on_screen(&AllMousePressButtons[ButtonIndex].button_image, Temp_Blitting_Rect.x,
						 Temp_Blitting_Rect.y, IMAGE_NO_TRANSFO);
}

/* -----------------------------------------------------------------
 * find a given filename in subdir relative to FD_DATADIR, 
 *
 * if you pass NULL as subdir, it will be ignored
 *
 * fills in the (ALLOC'd) string and returns 0 if okay, 1 on error
 *
 * ----------------------------------------------------------------- */
int find_file(const char *fname, const char *subdir, char *File_Path, int silent)
{
	int i;
	FILE *fp;		// this is the file we want to find?

	*File_Path = 0;
	if (!subdir)
		subdir = "";

	for (i = 0; i < 4; i++) {
		if (i == 0)
			sprintf((File_Path), ".");	/* first try local subdirs */
		if (i == 1)
			sprintf((File_Path), "..");	/* first try local subdirs */
		if (i == 2)
			sprintf((File_Path), "../..");	/* first try local subdirs */
		if (i == 3)
			sprintf((File_Path), "%s", FD_DATADIR);	/* then the DATADIR */

		strcat((File_Path), "/");
		strcat((File_Path), subdir);
		strcat((File_Path), "/");

		strcat((File_Path), fname);

		if ((fp = fopen((File_Path), "r")) != NULL) {	/* found it? */
			fclose(fp);
			break;
		} else {
			if (i != 3)
				DebugPrintf(1, "\nfind_file could not succeed with LOCAL path: %s.", File_Path);
			else {
				if (!silent) {
					DebugPrintf(-4, "The file name was: %s.\n", fname);
					ErrorMessage(__FUNCTION__, "File not found ", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
				}
				return 1;
			}
		}
	}			// for i 

	// DebugPrintf( 0 , "\nfind_file determined file path: %s." , File_Path );

	return 0;
};				// char * find_file ( ... )

/**
 * This function realizes the Pause-Mode: the game process is halted,
 * while the graphics and animations are not.  This mode 
 * can further be toggled from PAUSE to CHEESE, which is
 * a feature from the original program that should probably
 * allow for better screenshots.
 */
void Pause(void)
{
	int Pause = TRUE;
	int cheese = FALSE;	/* cheese mode: do not display GAME PAUSED  - nicer screenshots */
	SDL_Event event;
	SDLKey key;

	Activate_Conservative_Frame_Computation();

	AssembleCombatPicture(DO_SCREEN_UPDATE);

	input_get_keybind("pause", &key, NULL);

	while (Pause) {
		SDL_WaitEvent(&event);

		if (event.type == SDL_QUIT) {
			Terminate(EXIT_SUCCESS, TRUE);
		}

		AssembleCombatPicture(0);
		if (!cheese)
			CenteredPutStringFont(Screen, Menu_BFont, 200, _("GAME PAUSED"));
			CenteredPutStringFont(Screen, Menu_BFont, 230, _("press p to resume"));
		our_SDL_flip_wrapper();

		if (event.type == SDL_KEYDOWN) {
			if (event.key.keysym.sym == key) {
				Pause = FALSE;
			} else if (event.key.keysym.sym == SDLK_c) {
				cheese = !cheese;
			}
		}

		SDL_Delay(10);
	}

	return;
}

/**
 * This function starts the time-taking process.  Later the results
 * of this function will be used to calculate the current framerate
 */
void StartTakingTimeForFPSCalculation(void)
{
	/* This ensures, that 0 is never an encountered framenr,
	 * therefore count to 100 here
	 * Take the time now for calculating the frame rate
	 * (DO NOT MOVE THIS COMMAND PLEASE!) */
	framenr++;

	One_Frame_SDL_Ticks = SDL_GetTicks();
	if (framenr % 10 == 1)
		Ten_Frame_SDL_Ticks = SDL_GetTicks();
	if (framenr % 100 == 1) {
		Onehundred_Frame_SDL_Ticks = SDL_GetTicks();
	}
};				// void StartTakingTimeForFPSCalculation(void)

/**
 * This function computes the framerate that has been experienced
 * in this frame.  It will be used to correctly calibrate all 
 * movements of game objects.
 * 
 * NOTE:  To query the actual framerate a DIFFERENT function must
 *        be used, namely Frame_Time().
 */
void ComputeFPSForThisFrame(void)
{

	Now_SDL_Ticks = SDL_GetTicks();
	oneframedelay = Now_SDL_Ticks - One_Frame_SDL_Ticks;
	tenframedelay = Now_SDL_Ticks - Ten_Frame_SDL_Ticks;
	onehundredframedelay = Now_SDL_Ticks - Onehundred_Frame_SDL_Ticks;

	if (!oneframedelay)
		FPSover1 = 1000 * 1 / 0.5;
	else
		FPSover1 = 1000 * 1 / (float)oneframedelay;

	if (!tenframedelay)
		FPSover10 = 1000 * 10 / 0.5;
	else
		FPSover10 = 1000 * 10 / (float)tenframedelay;

	if (!onehundredframedelay)
		FPSover100 = 1000 * 100 / 0.5;
	else
		FPSover100 = 1000 * 100 / (float)onehundredframedelay;

};				// void ComputeFPSForThisFrame(void)

/**
 *
 * This function is the key to independence of the framerate for various game elements.
 * It returns the average time needed to draw one frame.
 * Other functions use this to calculate new positions of moving objects, etc..
 *
 * Also there is of course a serious problem when some interruption occurs, like e.g.
 * the options menu is called or the debug menu is called or the console or the elevator
 * is entered or a takeover game takes place.  This might cause HUGE framerates, that could
 * box the influencer out of the ship if used to calculate the new position.
 *
 * To counter unwanted effects after such events we have the SkipAFewFramerates counter,
 * which instructs Rate_To_Be_Returned to return only the overall default framerate since
 * no better substitute exists at this moment.  But on the other hand, this seems to
 * work REALLY well this way.
 *
 * This counter is most conveniently set via the function Activate_Conservative_Frame_Computation,
 * which can be conveniently called from eveywhere.
 *
 */
float Frame_Time(void)
{
	float Rate_To_Be_Returned;

	if (SkipAFewFrames) {
		Rate_To_Be_Returned = Overall_Average;
		return Rate_To_Be_Returned;
	}

	Rate_To_Be_Returned = (1.0 / FPSover1);

	return Rate_To_Be_Returned;
}

int Get_Average_FPS(void)
{
	return ((int)(1.0 / Overall_Average));
};				// int Get_Average_FPS( void )

/**
 * 
 * With framerate computation, there is a problem when some interruption occurs, like e.g.
 * the options menu is called or the debug menu is called or the console or the elevator
 * is entered or a takeover game takes place.  This might cause HUGE framerates, that could
 * box the influencer out of the ship if used to calculate the new position.
 *
 * To counter unwanted effects after such events we have the SkipAFewFramerates counter,
 * which instructs Rate_To_Be_Returned to return only the overall default framerate since
 * no better substitute exists at this moment.
 *
 * This counter is most conveniently set via the function Activate_Conservative_Frame_Computation,
 * which can be conveniently called from everywhere.
 *
 */
void Activate_Conservative_Frame_Computation(void)
{
	// SkipAFewFrames=212;
	// SkipAFewFrames=22;
	SkipAFewFrames = 3;

	DebugPrintf(1, "\nConservative_Frame_Computation activated!");

};				// void Activate_Conservative_Frame_Computation(void)

/**
 * This function is used to generate an integer in range of all
 * numbers from 0 to UpperBound.
 */
int MyRandom(int UpperBound)
{

	if (!UpperBound)
		return 0;

	float tmp;
	int PureRandom;
	int dice_val;		/* the result in [0, UpperBound] */

	PureRandom = rand();
	tmp = 1.0 * PureRandom / RAND_MAX;	/* random number in [0;1] */

	/* 
	 * we always round OFF for the resulting int, therefore
	 * we first add 0.99999 to make sure that UpperBound has
	 * roughly the same probablity as the other numbers 
	 */
	dice_val = (int)(tmp * (1.0 * UpperBound + 0.99999));

	return (dice_val);

};				// int MyRandom ( int UpperBound ) 

/**
 * This function teleports the influencer to a new position on the
 * ship.  THIS CAN BE A POSITION ON A DIFFERENT LEVEL.
 */
void Teleport(int LNum, float X, float Y, int with_sound_and_fading)
{
	// Check if we are in editor and not in game test mode then we store the level number
	//
	if(game_root_mode == ROOT_IS_LVLEDIT && game_status != INSIDE_GAME)
		GameConfig.last_edited_level = LNum;
	// Maybe the 'teleport' really comes from a teleportation device or
	// teleport spell or maybe even from accessing some sewer access way.
	// In that case we'll fade out the screen a bit using the gamma ramp
	// and then later back in again.  (Note that this is a blocking function
	// call, i.e. it will take a second or so each.)
	//
	if (with_sound_and_fading) {
		fade_out_screen();
	}

	if (LNum != Me.pos.z) {
		// In case a real level change has happened,
		// we need to do a lot of work.  Therefore we start by activating
		// the conservative frame time computation to avoid a 'jump'.
		//
		Activate_Conservative_Frame_Computation();

		Me.pos.x = X;
		Me.pos.y = Y;
		Me.pos.z = LNum;

		item_held_in_hand = NULL;

		// We add some sanity check against teleporting to non-allowed
		// locations (like outside of map that is)
		//
		if (!level_exists(LNum) || !pos_inside_level(Me.pos.x, Me.pos.y, curShip.AllLevels[LNum])) {
			fprintf(stderr, "\n\ntarget location was: lev=%d x=%f y=%f.\n", LNum, X, Y);
			fprintf(stderr, "source location was: lev=%d x=%f y=%f.", Me.pos.z, Me.pos.x, Me.pos.y);
			ErrorMessage(__FUNCTION__, "\
A Teleport was requested, but the location to teleport to lies outside\n\
the bounds of this 'ship' which means the current collection of levels.\n\
This indicates an error in the map system of Freedroid.", PLEASE_INFORM, IS_FATAL);
		}

		// Refresh some speed-up data structures
		get_visible_levels();
		
	} else {
		// If no real level change has occurred, everything
		// is simple and we just need to set the new coordinates, haha
		//
		Me.pos.x = X;
		Me.pos.y = Y;

		// Teleport could have been called by the leveleditor, due to
		// some changes in the current level (light values, for example),
		// so we refresh the speed-up data structures
		get_visible_levels();
	}

	// After the teleport, the mouse move target might be
	// completely out of date.  Therefore we simply delete it.  In cases
	// where the jump came from crossing a jump threshold (levels glued
	// together) we can still restore the move target in that (the calling!)
	// function.
	//
	Me.mouse_move_target.x = Me.pos.x;
	Me.mouse_move_target.y = Me.pos.y;
	Me.mouse_move_target.z = Me.pos.z;

	// Animate Tux as standing still
	Me.walk_cycle_phase = 0.0;
	Me.phase = tux_anim.standing_keyframe;

	if (with_sound_and_fading) {
		teleport_arrival_sound();
	}
	// Perhaps the player is visiting this level for the first time.  Then, the
	// tux should make it's initial statement about the location, if there is one.
	//
	if (!Me.HaveBeenToLevel[Me.pos.z]) {
		Me.HaveBeenToLevel[Me.pos.z] = TRUE;
	}

	SwitchBackgroundMusicTo(CURLEVEL()->Background_Song_Name);

	// Since we've mightily changed position now, we should clear the
	// position history, so that no one gets confused...
	//
	InitInfluPositionHistory();

	if (with_sound_and_fading) {
		append_new_game_message(_("Arrived at %s."), D_(curShip.AllLevels[Me.pos.z]->Levelname));
		fade_in_screen();
	}
}

/**
 * Teleport the influencer to the center of a level on the ship
 * \param level_num The number of the level where we want to be teleported
 */
void teleport_to_level_center(int level_num)
{
	// Calculate the center of the level
	float x = curShip.AllLevels[level_num]->xlen / 2;
	float y = curShip.AllLevels[level_num]->ylen / 2;

	// Teleporting to the center of the level
	Teleport(level_num, x, y, FALSE);
}

/**
 * Check if a level exists.
 * \param level_num The number of the level.
 * \return TRUE if the level exists.
*/
int level_exists(int level_num)
{
	if (level_num < 0 || level_num >= curShip.num_levels) {
		return FALSE;
	}

	if (curShip.AllLevels[level_num] == NULL) {
		return FALSE;
	}

	return TRUE;
}

/*----------------------------------------------------------------------
 * LoadGameConfig(): load saved options from config-file
 *
 * this should be the first of all load/save functions called
 * as here we read the $HOME-dir and create the config-subdir if necessary
 *
 *----------------------------------------------------------------------*/
int LoadGameConfig(void)
{
	char fname[5000];
	int original_width_of_screen = 800;
	int original_height_of_screen = 600;
	FILE *configfile;

	if (!our_config_dir) {
		DebugPrintf(1, "No useable config-dir. No config-loading possible\n");
		return (OK);
	}

	sprintf(fname, "%s/config", our_config_dir);
	if ((configfile = fopen(fname, "rb")) == NULL) {
		DebugPrintf(0, "WARNING: failed to open config-file: %s\n");
		return (ERR);
	}

	char *stuff = (char *)malloc(FS_filelength(configfile) + 1);
	fread(stuff, FS_filelength(configfile), 1, configfile);
	fclose(configfile);
	read_configuration_for_freedroid(stuff, "GameConfig", &GameConfig);
	configfile = NULL;
	free(stuff);

	if (!GameConfig.freedroid_version_string || strcmp(GameConfig.freedroid_version_string, VERSION)) {
		ErrorMessage(__FUNCTION__, "\
Settings file found in your ~/.freedroid_rpg dir does not\n\
seem to be from the same version a this installation of freedroid.\n\
This is perfectly normal if you have just upgraded your version of\n\
freedroid.  But the loading of your settings will be canceled now,\n\
cause the format of the settings file is no longer supported.  \n\
No need to panic.  The default settings will be used instead and a new\n\
settings file will be generated.\n", NO_NEED_TO_INFORM, IS_WARNING_ONLY);
		ResetGameConfigToDefaultValues();
		return (ERR);
	};

	// Now we will turn off the skills and inventory screen and that, cause
	// this should be off when the game starts...
	//
	GameConfig.CharacterScreen_Visible = FALSE;
	GameConfig.Inventory_Visible = FALSE;
	GameConfig.SkillScreen_Visible = FALSE;
	GameConfig.skill_explanation_screen_visible = FALSE;
	GameConfig.Automap_Visible = TRUE;

	if (command_line_override_for_screen_resolution) {
		GameConfig.screen_width = original_width_of_screen;
		GameConfig.screen_height = original_height_of_screen;
	}
	return (OK);

};				// int LoadGameConfig ( void )

/*----------------------------------------------------------------------
 * SaveGameConfig: do just that
 *
 *----------------------------------------------------------------------*/
int SaveGameConfig(void)
{
	char fname[5000];
	int current_width;
	int current_height;
	FILE *config_file;

	// Maybe the Terminate function was invoked BEFORE the startup process
	// was complete at all (like e.g. some illegal command line parameter).
	// Then the config dir is not initialized.  We catch this case and return
	// control to the operating system immediately if that happens...
	//
	if (our_config_dir == NULL) {
		printf("It seems that the game couldn't start up at all... therefore we need not save any config information.\n\n");
		SDL_Quit();
#if __WIN32__
		system("notepad stderr.txt");
		system("notepad stdout.txt");
#endif
		exit(ERR);
	}
	// Now we know, that the config dir has been initialized already.
	// That indicates, that the game did start up already.
	// Therefore we can do the normal save config stuff...
	//
	if (our_config_dir[0] == '\0')
		return (ERR);

	sprintf(fname, "%s/config", our_config_dir);
	if ((config_file = fopen(fname, "wb")) == NULL) {
		DebugPrintf(0, "WARNING: failed to create config-file: %s\n");
		return (ERR);
	}
	// We put the current version number of freedroid into the 
	// version number string.  This will be useful so that later
	// versions of freedroid can identify old config files and decide
	// not to use them in some cases.
	//
	if (GameConfig.freedroid_version_string) {
		free(GameConfig.freedroid_version_string);
	}
	GameConfig.freedroid_version_string = strdup(VERSION);

	// We preserve the current resolution, modify it a bit, such that
	// the preselected resolution will come to effect next time, save
	// it and then we restore the current settings again.
	//
	current_width = GameConfig.screen_width;
	current_height = GameConfig.screen_height;
	GameConfig.screen_width = GameConfig.next_time_width_of_screen;
	GameConfig.screen_height = GameConfig.next_time_height_of_screen;

	// Now write the actual data
	savestruct_autostr = alloc_autostr(4096);
	save_configuration_for_freedroid("GameConfig", &(GameConfig));
	fwrite(savestruct_autostr->value, 1, savestruct_autostr->length, config_file);

	free_autostr(savestruct_autostr);

	GameConfig.screen_width = current_width;
	GameConfig.screen_height = current_height;
	fclose(config_file);

	return (OK);

};				// int SaveGameConfig ( void )

/**
 * This function is used for terminating freedroid.  It will close
 * the SDL submodules and exit.
 */
void Terminate(int exit_code, int save_config)
{
	printf("\n----------------------------------------------------------------------");
	printf("\nTermination of freedroidRPG initiated...");

	// We save the config file in any case.

	if (save_config)
		SaveGameConfig();

	printf("Thank you for playing freedroidRPG.\n\n");
	SDL_Quit();

	// Finally, especially on win32 systems, we should open an editor with
	// the last debug output, since people in general won't know how and where
	// to find the material for proper reporting of bugs.

#if __WIN32__
	if (exit_code == EXIT_FAILURE) {
		system("notepad stderr.txt");
		system("notepad stdout.txt");
	}
#endif

	// Now we drop control back to the operating system.  The FreedroidRPG
	// program has finished.

	exit(exit_code);
}

/**
 * Return a pointer towards the obstacle designated by the given (unique) label.
 * \param level_number If not NULL, this is set to the levelnumber where the obstacle was found.
 */
obstacle *give_pointer_to_obstacle_with_label(const char *obstacle_label, int *level_number)
{
	int i, j;

	// On each level, browse the obstacle extensions until we find the label we are looking for
	for (i = 0; i < curShip.num_levels; i++) {
		level *l = curShip.AllLevels[i];

		if (l == NULL)
			continue;

		for (j = 0; j < l->obstacle_extensions.size; j++) {
			struct obstacle_extension *ext = &ACCESS_OBSTACLE_EXTENSION(l->obstacle_extensions, j);

			if (ext->type == OBSTACLE_EXTENSION_LABEL) {
				if (!strcmp(ext->data, obstacle_label)) {
					if (level_number) {
						*level_number = l->levelnum;
					}
					return ext->obs;
				}
			}

		}
	}

	ErrorMessage(__FUNCTION__, "Obstacle label \"%s\" was not found on the map.", PLEASE_INFORM, IS_FATAL, obstacle_label);
	return NULL;
}

/*----------------------------------------------------------------------
 * try getting round endian-differences with minimal intervention
 * to the code.. 
 *
 * read out a 2-byte short-int from give memory pointer, either using
 * the given byte-order (PCs) or SDL's 'network byte order' (Mac)
 *
 *----------------------------------------------------------------------*/
Sint16 ReadSint16(void *memory)
{
	Sint16 ret;

	memcpy(&ret, memory, sizeof(Sint16));
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	endian_swap((char *)&ret, sizeof(Sint16), 1);
#endif

	return (ret);

}				/* ReadSint16() */

/***********************************************************************
 * a little endian-swapper 
 */
void endian_swap(char *pdata, size_t dsize, size_t nelements)
{
	unsigned int i, j, indx;
	char tempbyte;

	if (dsize <= 1)
		return;

	for (i = 0; i < nelements; i++) {
		indx = dsize;
		for (j = 0; j < dsize / 2; j++) {
			tempbyte = pdata[j];
			indx = indx - 1;
			pdata[j] = pdata[indx];
			pdata[indx] = tempbyte;
		}

		pdata = pdata + dsize;
	}

	return;

}				/* endian swap */

/*----------------------------------------------------------------------
 * Get the power of 2 greater than of equal to the argument
 * http://www-graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
 *----------------------------------------------------------------------*/
uint32_t pot_gte(uint32_t v)
{
	uint32_t pot = v;
	--pot;
	pot |= pot >> 1;
	pot |= pot >> 2;
	pot |= pot >> 4;
	pot |= pot >> 8;
	pot |= pot >> 16;
	return (++pot);

}				// uint32_t pot_gte( uint32_t v)

#undef _misc_c
