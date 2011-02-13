/* 
 *
 *   Copyright (c) 2004-2010 Arthur Huillet 
 *   Copyright (c) 1994, 2002, 2003 Johannes Prix
 *   Copyright (c) 1994, 2002 Reinhard Prix
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
 * This file does everything that has to do with the takeover game from
 * the original paradroid game.
 */

#define _takeover_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"
#include "takeover.h"
#include "map.h"
#include "SDL_rotozoom.h"

enemy *cDroid;
Uint32 cur_time;		// current time in ms 

// Takeover images
static struct image FillBlocks[NUM_FILL_BLOCKS];
static struct image CapsuleBlocks[NUM_CAPS_BLOCKS];
static struct image ToGameBlocks[NUM_TO_BLOCKS];
static struct image ToGroundBlocks[NUM_GROUND_BLOCKS];
static struct image ToColumnBlock;
static struct image ToLeaderBlock;

//--------------------
// Class seperation of the blocks 
//
int BlockClass[TO_BLOCKS] = {
	CONNECTOR,		// cable
	NON_CONNECTOR,		// end of cable
	CONNECTOR,		// re-enforcer
	CONNECTOR,		// color-change
	CONNECTOR,		// bridge above
	NON_CONNECTOR,		// bridge middle
	CONNECTOR,		// bridge below
	NON_CONNECTOR,		// uniter above
	CONNECTOR,		// uniter middle
	NON_CONNECTOR,		// uniter below
	NON_CONNECTOR		// empty
};

//--------------------
// Probability of the various elements 
//
#define MAX_PROB		100
int ElementProb[TO_ELEMENTS] = {
	100,			// EL_CABLE 
	2,			// EL_CABLE_END 
	5,			// EL_AMPLIFIER 
	5,			// EL_COLOR_EXCHANGER: only on last layer 
	5,			// EL_SEPARATOR 
	5			// EL_GATE 
};

int max_opponent_capsules;
int NumCapsules[TO_COLORS] = {
	0, 0
};

point LeftCapsulesStart[TO_COLORS] = {
	{YELLOW_LEFT_CAPSULES_X, YELLOW_LEFT_CAPSULES_Y},
	{PURPLE_LEFT_CAPSULES_X, PURPLE_LEFT_CAPSULES_Y}
};

point CurCapsuleStart[TO_COLORS] = {
	{YELLOW_CUR_CAPSULE_X, YELLOW_CUR_CAPSULE_Y},
	{PURPLE_CUR_CAPSULE_X, PURPLE_CUR_CAPSULE_Y}
};

point PlaygroundStart[TO_COLORS] = {
	{YELLOW_PLAYGROUND_X, YELLOW_PLAYGROUND_Y},
	{PURPLE_PLAYGROUND_X, PURPLE_PLAYGROUND_Y}
};

point DruidStart[TO_COLORS] = {
	{YELLOW_DROID_X, YELLOW_DROID_Y},
	{PURPLE_DROID_X, PURPLE_DROID_Y}
};

int CapsuleCurRow[TO_COLORS] = { 0, 0 };

int LeaderColor = YELLOW;		/* momentary leading color */
int YourColor = YELLOW;
int OpponentColor = PURPLE;
int OpponentType;		/* The druid-type of your opponent */
enemy *cdroid;

/* the display  column */
int DisplayColumn[NUM_LINES] = {
	YELLOW, PURPLE, YELLOW, PURPLE, YELLOW, PURPLE, YELLOW, PURPLE, YELLOW, PURPLE,
	YELLOW, PURPLE
};

SDL_Color to_bg_color = { 199, 199, 199 };

playground_t ToPlayground;
playground_t ActivationMap;
playground_t CapsuleCountdown;

void EvaluatePlayground(void);
float EvaluatePosition(const int color, const int row, const int layer, const int endgame);
float EvaluateCenterPosition(const int color, const int row, const int layer, const int endgame);
void AdvancedEnemyTakeoverMovements(const int countdown);

static void ShowPlayground(void);

static void display_takeover_help()
{
	PlayATitleFile("TakeoverInstructions.title");
}

/** 
 * Display the picture of a droid
 */
static void show_droid_picture(int PosX, int PosY, int type)
{
	SDL_Surface *tmp;
	SDL_Rect target;
	char ConstructedFileName[5000];
	char fpath[2048];
	static char LastImageSeriesPrefix[1000] = "NONE_AT_ALL";
#define NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION 32
	static SDL_Surface *DroidRotationSurfaces[NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION] = { NULL };
	SDL_Surface *Whole_Image;
	int i;
	int RotationIndex;

	if (!strcmp(Druidmap[type].droid_portrait_rotation_series_prefix, "NONE_AVAILABLE_YET"))
		return;		// later this should be a default-correction instead

	// Maybe we have to reload the whole image series
	//
	if (strcmp(LastImageSeriesPrefix, Druidmap[type].droid_portrait_rotation_series_prefix)) {
		// Maybe we have to free the series from an old item display first
		//
		if (DroidRotationSurfaces[0] != NULL) {
			for (i = 1; i < NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION; i++) {
				SDL_FreeSurface(DroidRotationSurfaces[i]);
			}
		}
		// Now we can start to load the whole series into memory
		//
		for (i = 0; i < NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION; i++) {
			if (!strcmp(Druidmap[type].droid_portrait_rotation_series_prefix, "NONE_AVAILABLE_YET")) {
				Terminate(ERR);
			} else {
				sprintf(ConstructedFileName, "droids/%s/portrait_%04d.jpg",
					Druidmap[type].droid_portrait_rotation_series_prefix, i + 1);
			}

			// We must remember, that his is already loaded of course
			strcpy(LastImageSeriesPrefix, Druidmap[type].droid_portrait_rotation_series_prefix);

			find_file(ConstructedFileName, GRAPHICS_DIR, fpath, 0);

			Whole_Image = our_IMG_load_wrapper(fpath);	// This is a surface with alpha channel, since the picture is one of this type
			if (Whole_Image == NULL) {
				fprintf(stderr, "\n\nfpath: %s. \n", fpath);
				ErrorMessage(__FUNCTION__, "\
Freedroid was unable to load an image of a rotated droid into memory.\n\
This error indicates some installation problem with freedroid.", PLEASE_INFORM, IS_FATAL);
			}

			SDL_SetAlpha(Whole_Image, 0, SDL_ALPHA_OPAQUE);

			DroidRotationSurfaces[i] = our_SDL_display_format_wrapperAlpha(Whole_Image);	// now we have an alpha-surf of right size
			SDL_SetColorKey(DroidRotationSurfaces[i], 0, 0);	// this should clear any color key in the dest surface

			if (use_open_gl) {
				flip_image_vertically(DroidRotationSurfaces[i]);
			}

			SDL_FreeSurface(Whole_Image);
		}
	}

	RotationIndex = (SDL_GetTicks() / 50);

	RotationIndex =
	    RotationIndex - (RotationIndex / NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION) * NUMBER_OF_IMAGES_IN_DROID_PORTRAIT_ROTATION;

	tmp = DroidRotationSurfaces[RotationIndex];

	SDL_Surface *scaled = zoomSurface(tmp,
									  (float)Droid_Image_Window.w / (float)tmp->w,
									  (float)Droid_Image_Window.w / (float)tmp->w, 0);

	SDL_SetClipRect(Screen, NULL);
	Set_Rect(target, PosX, PosY, GameConfig.screen_width, GameConfig.screen_height);
	our_SDL_blit_surface_wrapper(scaled, NULL, Screen, &target);
	SDL_FreeSurface(scaled);
}

/**
 * Display infopage page of droidtype.
 */
static void show_droid_info(int droidtype)
{
	SDL_Rect clip;

	SDL_SetClipRect(Screen, NULL);

	// Show background
	blit_special_background(ITEM_BROWSER_BG_PIC_BACKGROUND_CODE);

	// Show droid portrait
	show_droid_picture(UNIVERSAL_COORD_W(45), UNIVERSAL_COORD_H(190), droidtype);

	// Show the droid name
	SetCurrentFont(Menu_BFont);
	clip.x = UNIVERSAL_COORD_W(330);
	clip.y = UNIVERSAL_COORD_H(35);
	clip.w = UNIVERSAL_COORD_W(200);
	clip.h = UNIVERSAL_COORD_H(30);
	display_text_using_line_height(Druidmap[droidtype].default_short_description, clip.x, clip.y, &clip, 1.0);
}

/**
 * Initialize text widget with a description of the specified droidtype.
 */
static void init_droid_description(text_widget *w, int droidtype)
{
	char *item_name;
	int weapon_type;

	autostr_append(w->text, _("Unit Type %s\n"), Druidmap[droidtype].druidname);
	autostr_append(w->text, _("Entry : %d\n"), droidtype + 1);

	if ((weapon_type = Druidmap[droidtype].weapon_item.type) >= 0)	// make sure item != -1 
		item_name = D_(ItemMap[weapon_type].item_name);	// does not segfault
	else
		item_name = _("none");

	autostr_append(w->text, _("\nArmament : %s\n"), item_name);
	autostr_append(w->text, _("\nCores : %d\n"), 2 + Druidmap[droidtype].class);


	if (Me.TakeoverSuccesses[droidtype]+Me.TakeoverFailures[droidtype]) {
		int success_ratio =
			((100*Me.TakeoverSuccesses[droidtype])/
			 (Me.TakeoverSuccesses[droidtype]+Me.TakeoverFailures[droidtype]));

		autostr_append(w->text, _("\nTakeover Success : %2d%%\n"), success_ratio);
	}

	autostr_append(w->text, _("\nNotes: %s\n"), D_(Druidmap[droidtype].notes));
}

/**
 * This function does the countdown where you still can changes your
 * color.
 */
static void ChooseColor(void)
{
	int countdown = 100;	// duration in 1/10 seconds given for color choosing 
	int ColorChosen = FALSE;
	char count_text[80];
	SDL_Event event;

	Uint32 prev_count_tick, count_tick_len;

	count_tick_len = 100;	// countdown in 1/10 second steps 

	prev_count_tick = SDL_GetTicks();

	while (!ColorChosen) {

		// wait for next countdown tick 
		while (SDL_GetTicks() < prev_count_tick + count_tick_len) ;

		prev_count_tick += count_tick_len;	// set for next tick 

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(0);
			}

			if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch (event.button.button) {
					//(clever?) hack : mouse wheel up and down behave
					//exactly like LEFT and RIGHT arrow, so we mangle the event
				case SDL_BUTTON_WHEELUP:
					event.type = SDL_KEYDOWN;
					event.key.keysym.sym = SDLK_LEFT;
					break;
				case SDL_BUTTON_WHEELDOWN:
					event.type = SDL_KEYDOWN;
					event.key.keysym.sym = SDLK_RIGHT;
					break;
				case SDL_BUTTON_LEFT:
					ColorChosen = TRUE;
					break;

				default:
					break;
				}
			}

			/* no else there! (mouse wheel) */
			if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case SDLK_RIGHT:
					YourColor = PURPLE;
					OpponentColor = YELLOW;
					break;
				case SDLK_LEFT:
					YourColor = YELLOW;
					OpponentColor = PURPLE;
					break;
				case SDLK_SPACE:
					ColorChosen = TRUE;
					break;
				default:
					break;
				}
			}

		}

		countdown--;	// Count down 
		sprintf(count_text, _("Color-%d.%d"), countdown/10, countdown%10);

		ShowPlayground();
		to_show_banner(count_text, NULL);
		our_SDL_flip_wrapper();

		if (countdown == 0)
			ColorChosen = TRUE;

	}			// while(!ColorChosen) 

	while (MouseLeftPressed())
		SDL_Delay(1);
};				// void ChooseColor ( void ) 

static void PlayGame(int countdown)
{
	char count_text[80];
	int FinishTakeover = FALSE;
	int row;

	Uint32 prev_count_tick, count_tick_len;	/* tick vars for count-down */
	Uint32 prev_move_tick, move_tick_len;	/* tick vars for motion */
	int wait_move_ticks;	/* number of move-ticks to wait before "key-repeat" */

	int up, down, set;
	int up_counter, down_counter;

	SDL_Event event;

	sprintf(count_text, _("Subliminal"));	/* Make sure a value gets assigned to count_text */
	count_tick_len = 100;	/* countdown in 1/10 second steps */
	move_tick_len = 60;	/* allow motion at this tick-speed in ms */

	up = down = set = FALSE;
	up_counter = down_counter = 0;

	wait_move_ticks = 2;

	prev_count_tick = prev_move_tick = SDL_GetTicks();	/* start tick clock */

	while (!FinishTakeover) {
		cur_time = SDL_GetTicks();

		/* 
		 * here we register if there have been key-press events in the
		 * "waiting period" between move-ticks :
		 */
		up = (up | UpPressed());
		down = (down | DownPressed());
		set = set | SpacePressed() | MouseLeftPressed();

		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_MOUSEBUTTONDOWN) {
				switch (event.button.button) {
				case SDL_BUTTON_WHEELUP:
					up++;
					break;
				case SDL_BUTTON_WHEELDOWN:
					down++;
					break;
				default:
					break;
				}
			} else if (event.type == SDL_KEYDOWN) {
				/* allow for a WIN-key that gives immediate victory */
				event.key.keysym.mod &= ~(KMOD_CAPS | KMOD_NUM | KMOD_MODE);	/* We want to ignore "global" modifiers. */
				if (event.key.keysym.sym == SDLK_w && (event.key.keysym.mod == (KMOD_LCTRL | KMOD_LALT))) {
					LeaderColor = YourColor;	/* simple as that */
					return;	/* leave now, to avoid changing of LeaderColor! */
				}
			} else if (event.type == SDL_QUIT) {
				Terminate(0);
			}
		}

		if (!up)
			up_counter = 0;	/* reset counters for released keys */
		if (!down)
			down_counter = 0;

		if (cur_time > prev_count_tick + count_tick_len) {	/* time to count 1 down */
			prev_count_tick += count_tick_len;	/* set for next countdown tick */
			countdown--;
			sprintf(count_text, _("Finish-%d.%d"), countdown/10, countdown%10);

			if (countdown == 0)
				FinishTakeover = TRUE;

			AnimateCurrents();	/* do some animation on the active cables */

		}

		/* if (countdown_tick has occurred) */
		/* time for movement */
		if (cur_time > prev_move_tick + move_tick_len) {
			prev_move_tick += move_tick_len;	/* set for next motion tick */
			AdvancedEnemyTakeoverMovements(countdown);

			if (up) {
				if (!up_counter || (up_counter > wait_move_ticks)) {
					// Here I have to change some things in order to make
					// mouse movement work properly with the wheel...
					//
					// CapsuleCurRow[YourColor]--;
					//
					CapsuleCurRow[YourColor] -= up;

					if (CapsuleCurRow[YourColor] < 1)
						CapsuleCurRow[YourColor] = NUM_LINES;
				}
				up = FALSE;
				up_counter++;
			}
			if (down) {
				if (!down_counter || (down_counter > wait_move_ticks)) {
					// Here I have to change some things in order to make
					// mouse movement work properly with the wheel...
					//
					// CapsuleCurRow[YourColor]++;
					//
					CapsuleCurRow[YourColor] += down;

					if (CapsuleCurRow[YourColor] > NUM_LINES)
						CapsuleCurRow[YourColor] = 1;
				}
				down = FALSE;
				down_counter++;
			}

			if (set && (NumCapsules[YOU] > 0)) {
				set = FALSE;
				row = CapsuleCurRow[YourColor] - 1;
				if ((row >= 0) &&
				    (ToPlayground[YourColor][0][row] != CABLE_END) && (ActivationMap[YourColor][0][row] == INACTIVE)) {
					NumCapsules[YOU]--;
					CapsuleCurRow[YourColor] = 0;
					ToPlayground[YourColor][0][row] = AMPLIFIER;
					ActivationMap[YourColor][0][row] = ACTIVE1;
					CapsuleCountdown[YourColor][0][row] = CAPSULE_COUNTDOWN * 2;

					Takeover_Set_Capsule_Sound();

					if (!NumCapsules[YOU]) {
						/* You placed your last capsule ? let's speed up the end */
						count_tick_len *= 0.75;
					}
				}	/* if (row > 0 && ... ) */
			}
			/* if ( set ) */
			ProcessCapsules();	/* count down the lifetime of the capsules */

			ProcessPlayground();
			ProcessPlayground();
			ProcessPlayground();
			ProcessPlayground();	/* this has to be done several times to be sure */

			ProcessDisplayColumn();

		}
		/* if (motion_tick has occurred) */
		ShowPlayground();
		to_show_banner(count_text, NULL);
		our_SDL_flip_wrapper();
		SDL_Delay(10);
	}			/* while !FinishTakeover */

	/* Final countdown */
	countdown = CAPSULE_COUNTDOWN + 10;

	while (countdown--) {
		// speed this up a little, some people get bored here...
		//      while ( SDL_GetTicks() < prev_count_tick + count_tick_len ) ;
		//      prev_count_tick += count_tick_len;
		ProcessCapsules();	/* count down the lifetime of the capsules */
		ProcessCapsules();	/* do it twice this time to be faster */
		//      AnimateCurrents ();
		ProcessPlayground();
		ProcessPlayground();
		ProcessPlayground();
		ProcessPlayground();	/* this has to be done several times to be sure */
		ProcessDisplayColumn();
		ShowPlayground();
		our_SDL_flip_wrapper();
	}			/* while (countdown) */

	return;

}

static void show_info_up_button() {
	ShowGenericButtonFromList(UP_BUTTON);
}

static void show_info_down_button() {
	ShowGenericButtonFromList(DOWN_BUTTON);
}

/**
 * This function manages the whole takeover game of Tux against 
 * some bot.
 *
 * The return value is TRUE/FALSE depending on whether the game was
 * finally won/lost.
 */
int droid_takeover(enemy *target)
{
	int menu_finished = FALSE;
	int reward = 0;
	SDL_Event event;
	static text_widget droid_info;

	// Set up the droid description widget
	init_text_widget(&droid_info, "");
	init_droid_description(&droid_info, target->type);
	droid_info.rect.x = 258 * GameConfig.screen_width / 640;
	droid_info.rect.y = 89 * GameConfig.screen_height / 480;
	droid_info.rect.w = 346 * GameConfig.screen_width / 640;
	droid_info.rect.h = 282 * GameConfig.screen_height / 480;
	droid_info.font = FPS_Display_BFont;
	droid_info.content_above_func = show_info_up_button;
	droid_info.content_below_func = show_info_down_button;

	// Prevent distortion of framerate by the delay coming from 
	// the time spent in the menu.
	Activate_Conservative_Frame_Computation();

	// We set the UserRect to full again, no matter what other windows might
	// be open right now...
	User_Rect.x = 0;
	User_Rect.y = 0;
	User_Rect.w = GameConfig.screen_width;
	User_Rect.h = GameConfig.screen_height;

	while (SpacePressed() || MouseLeftPressed()) ;	// make sure space is release before proceed 

	SwitchBackgroundMusicTo(TAKEOVER_BACKGROUND_MUSIC_SOUND);

	while (!menu_finished) {
		show_droid_info(target->type);
		show_text_widget(&droid_info);
		ShowGenericButtonFromList(TAKEOVER_HELP_BUTTON);
		SetCurrentFont(Para_BFont);
		CenteredPutString(Screen, GameConfig.screen_height - 30, "For more information, click the help button.");
		blit_mouse_cursor();
		our_SDL_flip_wrapper();

		while (SDL_PollEvent(&event)) {

			if (event.type == SDL_QUIT) {
				Terminate(0);
			}

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
				if (MouseCursorIsOnButton(UP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					droid_info.scroll_offset--;
				} else if (MouseCursorIsOnButton(DOWN_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					droid_info.scroll_offset++;
				} else if (MouseCursorIsOnButton(DROID_SHOW_EXIT_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					menu_finished = TRUE;
				} else if (MouseCursorIsOnButton(TAKEOVER_HELP_BUTTON, GetMousePos_x(), GetMousePos_y())) {
					display_takeover_help();
				}
			} else if (event.type == SDL_KEYDOWN
				   && ((event.key.keysym.sym == SDLK_SPACE) || (event.key.keysym.sym == SDLK_ESCAPE))) {
				menu_finished = TRUE;
			}
		}
		SDL_Delay(1);
	}

	while (!(!SpacePressed() && !EscapePressed() && !MouseLeftPressed())) ;

	cDroid = target;
	int player_capsules = 2 + Me.skill_level[get_program_index_with_name("Hacking")];
	max_opponent_capsules = 2 + Druidmap[target->type].class;

	if (do_takeover(player_capsules, max_opponent_capsules, 100)) {
		/* Won takeover */
		Me.marker = target->marker;

		reward = Druidmap[target->type].experience_reward * Me.experience_factor;
		Me.Experience += reward;
		append_new_game_message(_("For taking control of your enemy, \4%s\5, you receive %d experience."), target->short_description_text, reward);

		// Maybe the enemy in question was a kind of 'boss monster' or it had
		// some special item, that is relevant to a mission or quest.  In that
		// case (like also when the bot is finally destroyed, the quest item
		// should be dropped after the successful takeover process, even if the
		// enemy isn't completely dead yet...
		//
		if (target->on_death_drop_item_code != (-1)) {
			DropItemAt(target->on_death_drop_item_code, target->pos.z, target->pos.x, target->pos.y, 1);
			target->on_death_drop_item_code = -1;
		}

		target->faction = FACTION_SELF;
		target->has_been_taken_over = TRUE;

		target->combat_state = WAYPOINTLESS_WANDERING;

		// Always use the AfterTakeover dialog
		free(target->dialog_section_name);
		target->dialog_section_name = strdup("AfterTakeover");

		// When the bot is taken over, it should not turn hostile when
		// the rest of his former combat group (identified by having the
		// same marker) is attacked by the Tux.
		//
		target->marker = 0;
                Me.TakeoverSuccesses[target->type]++;
	} else {
		Me.energy *= 0.5;
                Me.TakeoverFailures[target->type]++;
	}

	cDroid = NULL;

	ClearGraphMem();

	SwitchBackgroundMusicTo(CURLEVEL()->Background_Song_Name);

	if (LeaderColor == YourColor)
		return TRUE;
	else
		return FALSE;

};				// int Takeover( int enemynum ) 

/*-----------------------------------------------------------------
 * This function performs the enemy movements in the takeover game,
 * but it does this in an advanced way, that has not been there in
 * the classic freedroid game.
 *-----------------------------------------------------------------*/
void AdvancedEnemyTakeoverMovements(const int countdown)
{
	// static int Actions = 3;
	static int MoveProbability = 100;
	static int TurnProbability = 10;
	static int SetProbability = 80;

	int action;
	static int direction = 1;	/* start with this direction */
	int row = CapsuleCurRow[OpponentColor] - 1;
	int test_row;
	int test_value;

	int BestTarget = -1;
	float BestValue = (-10000);	// less than any capsule can have

        int endgame = 0;
        if (countdown < NumCapsules[ENEMY]*7)
                endgame = 1;
        
#define TAKEOVER_MOVEMENT_DEBUG 1

	if (NumCapsules[ENEMY] == 0)
		return;


        if (GameConfig.difficulty_level!= DIFFICULTY_EASY){  //disable AI waiting on easy
                // Wait for the player to move
          if ((LeaderColor!=YourColor) && ((NumCapsules[YOU]-NumCapsules[ENEMY])>=0) && (!endgame) && (NumCapsules[ENEMY]<max_opponent_capsules))
                        return;
        }
        
	// First we're going to find out which target place is
	// best choice for the next capsule setting.
	//
	for (test_row = 0; test_row < NUM_LINES; test_row++) {
                test_value = EvaluatePosition(OpponentColor, test_row, 1, endgame) + 0.01*test_row;
                if (test_value > BestValue) {
                        BestTarget = test_row;
                        BestValue = test_value;
                }
	}
	DebugPrintf(TAKEOVER_MOVEMENT_DEBUG, "\nBest target row found : %d.", BestTarget);

        if ((BestValue < 0.5) && (!endgame) && (LeaderColor==OpponentColor)) //it isn't worth it
                return;
        
	// Now we can start to move into the right direction.
	// Previously this was a pure random choice like
	//
	// action = MyRandom (Actions);
	//
	// but now we do it differently :)
	//
	if (row < BestTarget) {
		direction = 1;
		action = 0;
	} else if (row > BestTarget) {
		direction = -1;
		action = 0;
	} else {
		action = 2;
	}

	switch (action) {
	case 0:		/* Move along */
		if (MyRandom(100) <= MoveProbability) {
			row += direction;
			if (row > NUM_LINES - 1)
				row = 0;
			if (row < 0)
				row = NUM_LINES - 1;
		}
		break;

	case 1:		/* Turn around */
		if (MyRandom(100) <= TurnProbability) {
			direction *= -1;
		}
		break;
	case 2:		/* Try to set  capsule */
                if (MyRandom(100) <= SetProbability) {
			if ((row >= 0) && 
			    (ToPlayground[OpponentColor][0][row] != CABLE_END) && (ActivationMap[OpponentColor][0][row] == INACTIVE)) {
				NumCapsules[ENEMY]--;
				Takeover_Set_Capsule_Sound();
				ToPlayground[OpponentColor][0][row] = AMPLIFIER;
				ActivationMap[OpponentColor][0][row] = ACTIVE1;
				CapsuleCountdown[OpponentColor][0][row] = CAPSULE_COUNTDOWN;
				row = -1;	/* For the next capsule: startpos */
			}
			else
			{
				row += direction;
                                return;
			}
		}
		/* if MyRandom */
		break;

	default:
		break;

	}			/* switch action */

	CapsuleCurRow[OpponentColor] = row + 1;

	return;
};				// AdvancedEnemyTakeoverMovements 

static void GetTakeoverGraphics(void)
{
	static int TakeoverGraphicsAreAlreadyLoaded = FALSE;
	struct image img = EMPTY_IMAGE;
	int i, j;
	int curx = 0, cury = 0;
	SDL_Rect tmp;

	if (TakeoverGraphicsAreAlreadyLoaded)
		return;

	load_image(&img, TO_BLOCK_FILE, FALSE);

	// Get the fill-blocks 
	for (i = 0; i < NUM_FILL_BLOCKS; i++, curx += FILL_BLOCK_LEN + 2) {
		Set_Rect(tmp, curx, cury, FILL_BLOCK_LEN, FILL_BLOCK_HEIGHT);
		create_subimage(&img, &FillBlocks[i], &tmp);
	}

	// Get the capsule blocks 
	for (i = 0; i < NUM_CAPS_BLOCKS; i++, curx += CAPSULE_LEN + 2) {
		Set_Rect(tmp, curx, cury, CAPSULE_LEN, CAPSULE_HEIGHT);
		create_subimage(&img, &CapsuleBlocks[i], &tmp);
	}

	// Get the default background color, to be used when no background picture found! 
	curx += CAPSULE_LEN + 2;
	curx = 0;
	cury += FILL_BLOCK_HEIGHT + 2;

	// get the game-blocks 
	for (j = 0; j < 2 * NUM_PHASES; j++) {
		for (i = 0; i < TO_BLOCKS; i++) {
			Set_Rect(tmp, curx, cury, TO_BLOCKLEN, TO_BLOCKHEIGHT);
			create_subimage(&img, &ToGameBlocks[j * TO_BLOCKS + i], &tmp);
			curx += TO_BLOCKLEN + 2;
		}
		curx = 0;
		cury += TO_BLOCKHEIGHT + 2;
	}

	// Get the ground, column and leader blocks 
	for (i = 0; i < NUM_GROUND_BLOCKS; i++) {
		Set_Rect(tmp, curx, cury, GROUNDBLOCKLEN, GROUNDBLOCKHEIGHT);
		create_subimage(&img, &ToGroundBlocks[i], &tmp);
		curx += GROUNDBLOCKLEN + 2;
	}
	cury += GROUNDBLOCKHEIGHT + 2;
	curx = 0;

	// Now the rectangle for the column blocks will be set and after
	// that we can create the new surface for blitting.
	//
	Set_Rect(tmp, curx, cury, COLUMNBLOCKLEN, COLUMNBLOCKHEIGHT);
	create_subimage(&img, &ToColumnBlock, &tmp);

	curx += COLUMNBLOCKLEN + 2;

	// Now the rectangle for the leader block will be set and after
	// that we can create the new surface for blitting.
	//
	Set_Rect(tmp, curx, cury, LEADERBLOCKLEN, LEADERBLOCKHEIGHT);
	create_subimage(&img, &ToLeaderBlock, &tmp);

	free_image_surface(&img);
	TakeoverGraphicsAreAlreadyLoaded = TRUE;
}

int do_takeover(int player_capsules, int opponent_capsules, int game_length)
{
	char *message;
	int player_won = 0;
	int FinishTakeover = FALSE;
	int row;
	int old_status;

	old_status = game_status;

	Activate_Conservative_Frame_Computation();

	// Maybe takeover graphics haven't been loaded yet.  Then we do this
	// here now and for once.  Later calls will be ignored inside the function.
	GetTakeoverGraphics();

	// eat pending events
	input_handle();

	while (!FinishTakeover) {
		// Init Color-column and Capsule-Number for each opponent and your color 
		//
		for (row = 0; row < NUM_LINES; row++) {
			DisplayColumn[row] = (row % 2);
			CapsuleCountdown[YELLOW][0][row] = -1;
			CapsuleCountdown[PURPLE][0][row] = -1;
		}		// for row 

		YourColor = YELLOW;
		OpponentColor = PURPLE;

		CapsuleCurRow[YELLOW] = 0;
		CapsuleCurRow[PURPLE] = 0;

		NumCapsules[YOU] = player_capsules;
		NumCapsules[ENEMY] = opponent_capsules;
		InventPlayground();

		EvaluatePlayground();

		ShowPlayground();
		our_SDL_flip_wrapper();

		ChooseColor();

		// This following function plays the takeover game, until one
		// of THREE states is reached, i.e. until YOU WON, YOU LOST
		// or until DEADLOCK is reached.  Well, so maybe after that
		// the takeover game is finished, but if it's a deadlock, then
		// the game must be played again in the next loop...
		//
		PlayGame(game_length);

		// We we evaluate the final score of the game.  Maybe we're done
		// already, maybe not...
		//
		if (LeaderColor == YourColor) {
			Takeover_Game_Won_Sound();
			message = _("Complete");
			FinishTakeover = TRUE;
			player_won = 1;
		} else if (LeaderColor == OpponentColor) {
			Takeover_Game_Lost_Sound();
			message = _("Rejected");
			FinishTakeover = TRUE;
			player_won = 0;
		} else {
			Takeover_Game_Deadlock_Sound();
			message = _("Deadlock");
		}

		ShowPlayground();
		to_show_banner(message, NULL);
		our_SDL_flip_wrapper();
		SDL_Delay(100);

	}

	game_status = old_status;
	return player_won;
}


static void ShowPlayground(void)
{
	int i, j;
	int color, player;
	int block;
	int xoffs, yoffs;
	SDL_Rect Target_Rect;

	xoffs = User_Rect.x + (User_Rect.w - 2 * 290) / 2;
	yoffs = User_Rect.y + (User_Rect.h - 2 * 140) / 2;

	//  SDL_SetColorKey (Screen, 0, 0);
	SDL_SetClipRect(Screen, &User_Rect);

	blit_special_background(TAKEOVER_BACKGROUND_CODE);

	blit_tux(xoffs + DruidStart[YourColor].x, yoffs + DruidStart[YourColor].y + 30);

	if (cDroid) {
		Set_Rect(Target_Rect, xoffs + DruidStart[!YourColor].x, yoffs + DruidStart[!YourColor].y, User_Rect.w, User_Rect.h);
		PutIndividuallyShapedDroidBody(cDroid, Target_Rect, FALSE, FALSE);
	}

	Set_Rect(Target_Rect, xoffs + LEFT_OFFS_X, yoffs + LEFT_OFFS_Y, User_Rect.w, User_Rect.h);

	display_image_on_screen (&ToGroundBlocks[YELLOW_HIGH], Target_Rect.x, Target_Rect.y);

	Target_Rect.y += GROUNDBLOCKHEIGHT;

	for (i = 0; i < 12; i++) {
		display_image_on_screen (&ToGroundBlocks[YELLOW_MIDDLE], Target_Rect.x, Target_Rect.y);
		Target_Rect.y += GROUNDBLOCKHEIGHT;
	}

	display_image_on_screen (&ToGroundBlocks[YELLOW_LOW], Target_Rect.x, Target_Rect.y);

	// the middle column
	Set_Rect(Target_Rect, xoffs + MID_OFFS_X, yoffs + MID_OFFS_Y, 0, 0);

	display_image_on_screen (&ToLeaderBlock, Target_Rect.x, Target_Rect.y);

	Target_Rect.y += LEADERBLOCKHEIGHT;
	for (i = 0; i < 12; i++, Target_Rect.y += COLUMNBLOCKHEIGHT) {
		display_image_on_screen (&ToColumnBlock, Target_Rect.x, Target_Rect.y);
	}

	// the right column
	Set_Rect(Target_Rect, xoffs + RIGHT_OFFS_X, yoffs + RIGHT_OFFS_Y, 0, 0);
	display_image_on_screen (&ToGroundBlocks[PURPLE_HIGH], Target_Rect.x, Target_Rect.y);

	Target_Rect.y += GROUNDBLOCKHEIGHT;

	for (i = 0; i < 12; i++, Target_Rect.y += GROUNDBLOCKHEIGHT) {
		display_image_on_screen (&ToGroundBlocks[PURPLE_MIDDLE], Target_Rect.x, Target_Rect.y);
	}

	display_image_on_screen (&ToGroundBlocks[PURPLE_LOW], Target_Rect.x, Target_Rect.y);

	// Fill the leader-LED with its color 
	Set_Rect(Target_Rect, xoffs + LEADERLED_X, yoffs + LEADERLED_Y, 0, 0);
	display_image_on_screen (&FillBlocks[LeaderColor], Target_Rect.x, Target_Rect.y);

	Target_Rect.y += FILL_BLOCK_HEIGHT;
	display_image_on_screen (&FillBlocks[LeaderColor], Target_Rect.x, Target_Rect.y);

	// Fill the display column with its colors 
	for (i = 0; i < NUM_LINES; i++) {
		Set_Rect(Target_Rect, xoffs + LEDCOLUMN_X, yoffs + LEDCOLUMN_Y + i * (FILL_BLOCK_HEIGHT + 2), 0, 0);
		display_image_on_screen (&FillBlocks[DisplayColumn[i]], Target_Rect.x, Target_Rect.y);
	}

	// Show the yellow playground 
	for (i = 0; i < NUM_LAYERS - 1; i++)
		for (j = 0; j < NUM_LINES; j++) {
			Set_Rect(Target_Rect, xoffs + PlaygroundStart[YELLOW].x + i * TO_BLOCKLEN,
				 yoffs + PlaygroundStart[YELLOW].y + j * TO_BLOCKHEIGHT, 0, 0);
			block = ToPlayground[YELLOW][i][j] + ActivationMap[YELLOW][i][j] * TO_BLOCKS;
			display_image_on_screen (&ToGameBlocks[block], Target_Rect.x, Target_Rect.y);
		}

	// Show the purple playground 
	for (i = 0; i < NUM_LAYERS - 1; i++)
		for (j = 0; j < NUM_LINES; j++) {
			Set_Rect(Target_Rect,
				 xoffs + PlaygroundStart[PURPLE].x + (NUM_LAYERS - i - 2) * TO_BLOCKLEN,
				 yoffs + PlaygroundStart[PURPLE].y + j * TO_BLOCKHEIGHT, 0, 0);
			block = ToPlayground[PURPLE][i][j] + (NUM_PHASES + ActivationMap[PURPLE][i][j]) * TO_BLOCKS;
			display_image_on_screen (&ToGameBlocks[block], Target_Rect.x, Target_Rect.y);
		}

	// Show the capsules left for each player 
	for (player = 0; player < 2; player++) {
		if (player == YOU)
			color = YourColor;
		else
			color = OpponentColor;

		Set_Rect(Target_Rect, xoffs + CurCapsuleStart[color].x,
			 yoffs + CurCapsuleStart[color].y + CapsuleCurRow[color] * (CAPSULE_HEIGHT + 2), 0, 0);
		if (NumCapsules[player]) {
			display_image_on_screen (&CapsuleBlocks[color], Target_Rect.x, Target_Rect.y);
		}

		for (i = 0; i < NumCapsules[player] - 1; i++) {
			Set_Rect(Target_Rect, xoffs + LeftCapsulesStart[color].x,
				 yoffs + LeftCapsulesStart[color].y + i * CAPSULE_HEIGHT, 0, 0);
			display_image_on_screen (&CapsuleBlocks[color], Target_Rect.x, Target_Rect.y);
		}		// for capsules 
	}			// for player 

	return;

};				// ShowPlayground 

/*-----------------------------------------------------------------
 * @Desc: Clears Playground (and ActivationMap) to default start-values
 * @Ret:  void
 *
 *-----------------------------------------------------------------*/
void ClearPlayground(void)
{
	int color, layer, row;

	for (color = YELLOW; color < TO_COLORS; color++)
		for (layer = 0; layer < NUM_LAYERS; layer++)
			for (row = 0; row < NUM_LINES; row++) {
				ActivationMap[color][layer][row] = INACTIVE;
				if (layer < TO_COLORS - 1)
					ToPlayground[color][layer][row] = CABLE;
				else
					ToPlayground[color][layer][row] = INACTIVE;
			}

	for (row = 0; row < NUM_LINES; row++)
		DisplayColumn[row] = row % 2;

};				// void ClearPlayground ( void )

/* -----------------------------------------------------------------
 * This function generates a random playground for the takeover game
 * ----------------------------------------------------------------- */
void InventPlayground(void)
{
	int anElement;
	int newElement;
	int row, layer;
	int color = YELLOW;

	// first clear the playground: we depend on this !! 
	//
	ClearPlayground();

	for (color = YELLOW; color < TO_COLORS; color++) {
		for (layer = 1; layer < NUM_LAYERS - 1; layer++) {
			for (row = 0; row < NUM_LINES; row++) {
				if (ToPlayground[color][layer][row] != CABLE)
					continue;

				newElement = MyRandom(TO_ELEMENTS);
				if (MyRandom(MAX_PROB) > ElementProb[newElement]) {
					row--;
					continue;
				}

				switch (newElement) {
				case EL_CABLE:	/* has not to be set any more */
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					break;

				case EL_CABLE_END:
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					else
						ToPlayground[color][layer][row] = CABLE_END;
					break;

				case EL_AMPLIFIER:
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					else
						ToPlayground[color][layer][row] = AMPLIFIER;
					break;

				case EL_COLOR_EXCHANGER:
					if (layer != 2) {	/* only existing on layer 2 */
						row--;
						continue;
					}

					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR)
						ToPlayground[color][layer][row] = EMPTY;
					else
						ToPlayground[color][layer][row] = COLOR_EXCHANGER;
					break;

				case EL_SEPARATOR:
					if (row > NUM_LINES - 3) {
						/* try again */
						row--;
						break;
					}

					anElement = ToPlayground[color][layer - 1][row + 1];
					if (BlockClass[anElement] == NON_CONNECTOR) {
						/* try again */
						row--;
						break;
					}

					/* don't destroy branch in prev. layer */
					anElement = ToPlayground[color][layer - 1][row];
					if (anElement == SEPARATOR_H || anElement == SEPARATOR_L) {
						row--;
						break;
					}
					anElement = ToPlayground[color][layer - 1][row + 2];
					if (anElement == SEPARATOR_H || anElement == SEPARATOR_L) {
						row--;
						break;
					}

					/* cut off cables in last layer, if any */
					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row] = CABLE_END;

					anElement = ToPlayground[color][layer - 1][row + 2];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row + 2] = CABLE_END;

					/* set the branch itself */
					ToPlayground[color][layer][row] = SEPARATOR_H;
					ToPlayground[color][layer][row + 1] = SEPARATOR_M;
					ToPlayground[color][layer][row + 2] = SEPARATOR_L;

					row += 2;
					break;

				case EL_GATE:
					if (row > NUM_LINES - 3) {
						/* try again */
						row--;
						break;
					}

					anElement = ToPlayground[color][layer - 1][row];
					if (BlockClass[anElement] == NON_CONNECTOR) {
						/* try again */
						row--;
						break;
					}
					anElement = ToPlayground[color][layer - 1][row + 2];
					if (BlockClass[anElement] == NON_CONNECTOR) {
						/* try again */
						row--;
						break;
					}

					/* cut off cables in last layer, if any */
					anElement = ToPlayground[color][layer - 1][row + 1];
					if (BlockClass[anElement] == CONNECTOR)
						ToPlayground[color][layer - 1][row + 1] = CABLE_END;

					/* set the GATE itself */
					ToPlayground[color][layer][row] = GATE_H;
					ToPlayground[color][layer][row + 1] = GATE_M;
					ToPlayground[color][layer][row + 2] = GATE_L;

					row += 2;
					break;

				default:
					row--;
					break;

				}	/* switch NewElement */

			}	/* for row */

		}		/* for layer */

	}			/* for color */

}				/* InventPlayground */

/* -----------------------------------------------------------------
 * This function generates a random playground for the takeover game
 * ----------------------------------------------------------------- */
void EvaluatePlayground(void)
{
	int newElement;
	int row, layer;
	int color = YELLOW;
	float ScoreFound[TO_COLORS];

#define EVALUATE_PLAYGROUND_DEBUG 1

	for (color = YELLOW; color < TO_COLORS; color++) {

		DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\n----------------------------------------------------------------------\n\
Starting to evaluate side nr. %d.  Results displayed below:\n", color);
		ScoreFound[color] = 0;

		for (layer = 1; layer < NUM_LAYERS - 1; layer++) {
			for (row = 0; row < NUM_LINES; row++) {

				// we examine this particular spot
				newElement = ToPlayground[color][layer][row];

				switch (newElement) {
				case CABLE:	/* has not to be set any more */
				case EMPTY:
					break;

				case CABLE_END:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "CABLE_END found --> score -= 1.0\n");
					ScoreFound[color] -= 1.0;
					break;

				case AMPLIFIER:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "AMPLIFIER found --> score += 0.5\n");
					ScoreFound[color] += 0.5;
					break;

				case COLOR_EXCHANGER:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "COLOR_EXCHANGER found --> score -= 1.5\n");
					ScoreFound[color] -= 1.5;
					break;

				case SEPARATOR_H:
				case SEPARATOR_L:
				case SEPARATOR_M:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "SEPARATOR found --> score += 1.0\n");
					ScoreFound[color] += 1.0;
					break;

				case GATE_M:
				case GATE_L:
				case GATE_H:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "GATE found --> score -= 1.0\n");
					ScoreFound[color] -= 1.0;
					break;

				default:
					DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "UNHANDLED TILE FOUND!!\n");
					break;

				}	/* switch NewElement */

			}	/* for row */

		}		/* for layer */

		DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\nResult for this side:  %f.\n", ScoreFound[color]);

	}			/* for color */

	DebugPrintf(EVALUATE_PLAYGROUND_DEBUG, "\n----------------------------------------------------------------------\n");

};				// EvaluatePlayground 

/* -----------------------------------------------------------------
 * This function Evaluates the AI side of the board for the takeover game
 * ----------------------------------------------------------------- */
float EvaluatePosition(const int color, const int row, const int layer, const int endgame)
{
        int player = YOU;
        if (color != YourColor)
                player = ENEMY;

        int opp_color = YELLOW;
        if (opp_color == color)
                opp_color = PURPLE;
        
	int newElement;
	// float ScoreFound [ TO_COLORS ];

#define EVAL_DEBUG 1
        
	DebugPrintf(EVAL_DEBUG, "\nEvaluatePlaygound ( %d , %d , %d ) called: ", color, row, layer);

	if (layer == NUM_LAYERS - 1) {
		DebugPrintf(EVAL_DEBUG, "End layer reached...");
                if (IsActive(color, row)) {
                        DebugPrintf(EVAL_DEBUG, "same color already active... returning 0.01 ");
                        return (0.01*EvaluateCenterPosition(opp_color, row, layer, endgame));
		} else if (DisplayColumn[row] == color) {
			DebugPrintf(EVAL_DEBUG, "same color... returning 0.05 ");
			return (0.05*EvaluateCenterPosition(opp_color, row, layer, endgame));
		} else if (IsActive(opp_color, row)) {
			DebugPrintf(EVAL_DEBUG, "different color, but active... returning 9 ");
                        if (endgame)
                                return (90*EvaluateCenterPosition(opp_color, row, layer, endgame));
                        else
                                return (8*EvaluateCenterPosition(opp_color, row, layer, endgame));
		} else {
			DebugPrintf(EVAL_DEBUG, "different color... returning 10 ");
                        if (endgame)
                                return (100*EvaluateCenterPosition(opp_color, row, layer, endgame));
                        else
                                return (10*EvaluateCenterPosition(opp_color, row, layer, endgame));
		}
	}

	if (ActivationMap[color][layer][row] == ACTIVE1) { return (0); }

	newElement = ToPlayground[color][layer][row];

	switch (newElement) {
	case CABLE:		/* has not to be set any more */
		DebugPrintf(EVAL_DEBUG, "CABLE reached... continuing...");
		return (EvaluatePosition(color, row, layer + 1, endgame));

	case AMPLIFIER:
		DebugPrintf(EVAL_DEBUG, "AMPLIFIER reached... continuing...");
                return ((1 + (0.2 * NumCapsules[player])) * EvaluatePosition(color, row, layer + 1, endgame));
		break;

	case COLOR_EXCHANGER:
		DebugPrintf(EVAL_DEBUG, "COLOR_EXCHANGER reached... continuing...");
                return (-1.5 * EvaluatePosition(color, row, layer + 1, endgame));
		break;

	case SEPARATOR_M:
		DebugPrintf(EVAL_DEBUG, "SEPARATOR reached... double-continuing...");
		return (EvaluatePosition(color, row + 1, layer + 1, endgame) + EvaluatePosition(color, row - 1, layer + 1, endgame));
		break;

	case GATE_H:
		DebugPrintf(EVAL_DEBUG, "GATE reached... stopping...\n");
                if (ActivationMap[color][layer][row + 2] >= ACTIVE1) {
                        return (5 * EvaluatePosition(color, row + 1, layer + 1, endgame));
                } else if (endgame) {
                        return (0.5 * EvaluatePosition(color, row + 1, layer + 1, endgame));
                } else {
                        return (0.3 * EvaluatePosition(color, row + 1, layer + 1, endgame));
                }
		break;

	case GATE_L:
		DebugPrintf(EVAL_DEBUG, "GATE reached... stopping...\n");
                if (ActivationMap[color][layer][row - 2] >= ACTIVE1) {
                        return (5 * EvaluatePosition(color, row - 1, layer + 1, endgame));
                } else if (endgame) {
                        return (0.5 * EvaluatePosition(color, row - 1, layer + 1, endgame));
                } else {
                        return (0.3 * EvaluatePosition(color, row - 1, layer + 1, endgame));
                }
		break;
        //treat the following cases the same:
	case EMPTY:
	case CABLE_END:
	case SEPARATOR_H:
	case SEPARATOR_L:
	case GATE_M:
		return (0);
		break;
	default:
		DebugPrintf(EVAL_DEBUG, "\nUNHANDLED TILE reached\n");
		break;

	}
	return (0);

};

/* -----------------------------------------------------------------
 * This function Evaluates the Player side of the board from AI perspective
 * ----------------------------------------------------------------- */
float EvaluateCenterPosition(const int color, const int row, const int layer, const int endgame)
{
        int player = YOU;  //should match the color owner (and be opposite the AI)
        if (color != YourColor)
                player = ENEMY;

        int newElement;
                
        if (layer == 1) {
                return (1); //hit the player's end
        }

	newElement = ToPlayground[color][layer][row];

	switch (newElement) {
	case CABLE:		// has not to be set any more 
		return (EvaluateCenterPosition(color, row, layer - 1, endgame));
	case AMPLIFIER:
                if (ActivationMap[color][layer+1][row] >= ACTIVE1) { //very low hope to take this over
                        return (0.06);
                } else if (endgame) {
                        return (1);
                } else { 
                        return ((1 - (0.1 * NumCapsules[player])) * EvaluateCenterPosition(color, row, layer - 1, endgame));
		}
                break;
        case SEPARATOR_H: //correctly evaluating this may lead to an infinite loop, using 0.5
                if ((NumCapsules[player]>0) && (!endgame))
                        return ((0.53 - (0.02 * NumCapsules[player])) * EvaluateCenterPosition(color, row - 1, layer - 1, endgame) );
                else
                        return (EvaluateCenterPosition(color, row - 1, layer - 1, endgame));
		break;
	case SEPARATOR_L:
                if ((NumCapsules[player]>0) && (!endgame))
                        return ((0.53 - (0.02 * NumCapsules[player])) * EvaluateCenterPosition(color, row + 1, layer - 1, endgame) );
                else
                        return (EvaluateCenterPosition(color, row + 1, layer - 1, endgame));
		break;

	case GATE_M:
                if (!endgame)
                        return ((0.5 + (0.09 * NumCapsules[player])) * (EvaluateCenterPosition(color, row + 1, layer - 1, endgame) + EvaluateCenterPosition(color, row - 1, layer - 1, endgame)));
                else
                        return (1);
		break;

        //Following cases act the same:
        case COLOR_EXCHANGER:
	case EMPTY:
	case CABLE_END:
	case SEPARATOR_M:
       	case GATE_H:
	case GATE_L:
                if (!endgame)
                        return (1 + (0.2 * NumCapsules[player]));
                else
                        return (1);
                break;
	default:
		DebugPrintf(EVAL_DEBUG, "\nUNHANDLED TILE reached\n");
		break;
	}

	return (0);

};

/*-----------------------------------------------------------------
 * @Desc: process the playground following its intrinsic logic
 *
 * @Ret: void
 *
 *-----------------------------------------------------------------*/
void ProcessPlayground(void)
{
	int color, layer, row;
	int TurnActive = FALSE;

	for (color = YELLOW; color < TO_COLORS; color++) {
		for (layer = 1; layer < NUM_LAYERS; layer++) {
			for (row = 0; row < NUM_LINES; row++) {
				if (layer == NUM_LAYERS - 1) {
					if (IsActive(color, row))
						ActivationMap[color][layer][row] = ACTIVE1;
					else
						ActivationMap[color][layer][row] = INACTIVE;

					continue;
				}
				/* if last layer */
				TurnActive = FALSE;

				switch (ToPlayground[color][layer][row]) {
				case COLOR_EXCHANGER:
				case SEPARATOR_M:
				case GATE_H:
				case GATE_L:
				case CABLE:
					if (ActivationMap[color][layer - 1][row] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case AMPLIFIER:
					if (ActivationMap[color][layer - 1][row] >= ACTIVE1)
						TurnActive = TRUE;

					// additional enforcers stay active by themselves...
					if (ActivationMap[color][layer][row] >= ACTIVE1)
						TurnActive = TRUE;

					break;

				case CABLE_END:
					break;

				case SEPARATOR_H:
					if (ActivationMap[color][layer][row + 1] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case SEPARATOR_L:
					if (ActivationMap[color][layer][row - 1] >= ACTIVE1)
						TurnActive = TRUE;
					break;

				case GATE_M:
					if ((ActivationMap[color][layer][row - 1] >= ACTIVE1)
					    && (ActivationMap[color][layer][row + 1] >= ACTIVE1))
						TurnActive = TRUE;

					break;

				default:
					break;
				}	/* switch */

				if (TurnActive) {
					if (ActivationMap[color][layer][row] == INACTIVE)
						ActivationMap[color][layer][row] = ACTIVE1;
				} else
					ActivationMap[color][layer][row] = INACTIVE;

			}	/* for row */

		}		/* for layer */

	}			/* for color */

	return;
};				// void ProcessPlayground ( void )

/** 
 * This function sets the correct values for the status column in the
 * middle of the takeover game field.
 * Blinking LEDs are realized here as well.
 */
void ProcessDisplayColumn(void)
{
	static int CLayer = 3;	/* the connection-layer to the Column */
	static int flicker_color = 0;
	int row;
	int GelbCounter, ViolettCounter;

	flicker_color = !flicker_color;

	for (row = 0; row < NUM_LINES; row++) {
		// unquestioned yellow
		if ((ActivationMap[YELLOW][CLayer][row] >= ACTIVE1) && (ActivationMap[PURPLE][CLayer][row] == INACTIVE)) {
			// change color?
			if (ToPlayground[YELLOW][CLayer - 1][row] == COLOR_EXCHANGER)
				DisplayColumn[row] = PURPLE;
			else
				DisplayColumn[row] = YELLOW;
			continue;
		}
		// clearly purple
		if ((ActivationMap[YELLOW][CLayer][row] == INACTIVE) && (ActivationMap[PURPLE][CLayer][row] >= ACTIVE1)) {
			// change color?
			if (ToPlayground[PURPLE][CLayer - 1][row] == COLOR_EXCHANGER)
				DisplayColumn[row] = YELLOW;
			else
				DisplayColumn[row] = PURPLE;

			continue;
		}
		// undecided: flimmering
		if ((ActivationMap[YELLOW][CLayer][row] >= ACTIVE1) && (ActivationMap[PURPLE][CLayer][row] >= ACTIVE1)) {
			// change color?
			if ((ToPlayground[YELLOW][CLayer - 1][row] == COLOR_EXCHANGER) &&
			    (ToPlayground[PURPLE][CLayer - 1][row] != COLOR_EXCHANGER))
				DisplayColumn[row] = PURPLE;
			else if ((ToPlayground[YELLOW][CLayer - 1][row] != COLOR_EXCHANGER) &&
				 (ToPlayground[PURPLE][CLayer - 1][row] == COLOR_EXCHANGER))
				DisplayColumn[row] = YELLOW;
			else {
				if (flicker_color == 0)
					DisplayColumn[row] = YELLOW;
				else
					DisplayColumn[row] = PURPLE;
			}	/* if - else if - else */

		}
		/* if undecided */
	}			/* for */

	// evaluate the winning color
	GelbCounter = 0;
	ViolettCounter = 0;
	for (row = 0; row < NUM_LINES; row++)
		if (DisplayColumn[row] == YELLOW)
			GelbCounter++;
		else
			ViolettCounter++;

	if (ViolettCounter < GelbCounter)
		LeaderColor = YELLOW;
	else if (ViolettCounter > GelbCounter)
		LeaderColor = PURPLE;
	else
		LeaderColor = DRAW;

	return;
};				// void ProcessDisplayColumn 

/** 
 * This function does the countdown of the capsules and kills them if 
 * they are too old.
 */
void ProcessCapsules(void)
{
	int row;
	int color;

	for (color = YELLOW; color <= PURPLE; color++)
		for (row = 0; row < NUM_LINES; row++) {
			if (CapsuleCountdown[color][0][row] > 0)
				CapsuleCountdown[color][0][row]--;

			if (CapsuleCountdown[color][0][row] == 0) {
				CapsuleCountdown[color][0][row] = -1;
				ActivationMap[color][0][row] = INACTIVE;
				ToPlayground[color][0][row] = CABLE;
			}

		}		/* for row */

};				// void ProcessCapsules ( void )

/**
 * This function tells, whether a Column-connection is active or not.
 * It returns TRUE or FALSE accordingly.
 */
int IsActive(int color, int row)
{
	int CLayer = 3;		/* the connective Layer */
	int TestElement = ToPlayground[color][CLayer - 1][row];

	if ((ActivationMap[color][CLayer - 1][row] >= ACTIVE1) && (BlockClass[TestElement] == CONNECTOR))
		return TRUE;
	else
		return FALSE;
}				/* IsActive */

/* -----------------------------------------------------------------
 * This function animates the active cables: this is done by cycling 
 * over the active phases ACTIVE1-ACTIVE3, which are represented by 
 * different pictures in the playground
 * ----------------------------------------------------------------- */
void AnimateCurrents(void)
{
	int color, layer, row;

	for (color = YELLOW; color <= PURPLE; color++)
		for (layer = 0; layer < NUM_LAYERS; layer++)
			for (row = 0; row < NUM_LINES; row++)
				if (ActivationMap[color][layer][row] >= ACTIVE1) {
					ActivationMap[color][layer][row]++;
					if (ActivationMap[color][layer][row] == NUM_PHASES)
						ActivationMap[color][layer][row] = ACTIVE1;
				}

	return;
};				// void AnimateCurrents (void)

/**
 *
 *
 */
void to_show_banner(const char *left, const char *right)
{
	char left_box[LEFT_TEXT_LEN + 10];
	char right_box[RIGHT_TEXT_LEN + 10];
	int left_len, right_len;	// the actual string lengths

	// At first the text is prepared.  This can't hurt.
	// we will decide whether to display it or not later...
	//

	if (left == NULL)
		left = "0";

	if (right == NULL) {
		right = "";
	}
	// Now fill in the text
	left_len = strlen(left);
	if (left_len > LEFT_TEXT_LEN) {
		printf("\nWarning: String %s too long for Left Infoline!!", left);
		Terminate(ERR);
	}
	right_len = strlen(right);
	if (right_len > RIGHT_TEXT_LEN) {
		printf("\nWarning: String %s too long for Right Infoline!!", right);
		Terminate(ERR);
	}
	// Now prepare the left/right text-boxes 
	memset(left_box, ' ', LEFT_TEXT_LEN);	// pad with spaces 
	memset(right_box, ' ', RIGHT_TEXT_LEN);

	strncpy(left_box, left, left_len);	// this drops terminating \0 ! 
	strncpy(right_box, right, right_len);	// this drops terminating \0 ! 

	left_box[LEFT_TEXT_LEN] = '\0';	// that's right, we want padding!
	right_box[RIGHT_TEXT_LEN] = '\0';

	// Now the text should be ready and its
	// time to display it...
	DebugPrintf(2, "Takeover said: %s -- %s\n", left_box, right_box);
	SetCurrentFont(Para_BFont);
	display_text(left_box, LEFT_INFO_X, LEFT_INFO_Y, NULL);
	display_text(right_box, RIGHT_INFO_X, RIGHT_INFO_Y, NULL);

};				// void to_show_banner (const char* left, const char* right)

#undef _takeover_c
