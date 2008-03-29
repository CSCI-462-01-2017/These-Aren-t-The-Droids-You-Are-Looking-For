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

/* ----------------------------------------------------------------------
 * This file contains all the functions managing the things one gets to see.
 * That includes assembling of enemys, assembling the currently
 * relevant porting of the map (the bricks I mean), drawing all visible
 * elements like bullets, blasts, enemys or influencer in a nonvisible
 * place in memory at first, and finally drawing them to the visible
 * screen for the user.
 * ---------------------------------------------------------------------- */

#define _mission_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"
#include "SDL_rotozoom.h"

int currently_selected_mission = (-1) ;
SDL_Rect mission_list_rect = { 20 , 280 , 280 , 180 } ; 
SDL_Rect mission_description_rect = { 134 , 86 , 280 , 320 } ; 
char complete_mission_display_text [ 50000 ];
float mission_list_offset = 0 ;
int mission_list_scroll_override_from_user = 0 ;

#define QUEST_BROWSER_SHOW_OPEN_MISSIONS (-1011)
#define QUEST_BROWSER_SHOW_DONE_MISSIONS (-1012)
#define QUEST_BROWSER_SHOW_NOTES (-1013)

int current_quest_browser_mode = QUEST_BROWSER_SHOW_OPEN_MISSIONS ;

int quest_browser_mission_lines_needed [ MAX_MISSIONS_IN_GAME ] ;

/* ----------------------------------------------------------------------
 * This function is responsible for making a new quest diary entry 
 * visible inside the quest browser.
 * ---------------------------------------------------------------------- */
void
quest_browser_enable_new_diary_entry ( int mis_num , int mis_diary_entry_num )
{

    if ( ( mis_num < 0 ) || ( mis_num >= MAX_MISSIONS_IN_GAME ) )
    {
	fprintf ( stderr , "\nmission number received: %d." , mis_num );
	ErrorMessage ( __FUNCTION__  , "\
There was an illegal mission number received.",
				   PLEASE_INFORM, IS_FATAL );
    }
    if ( ( mis_diary_entry_num < 0 ) || ( mis_diary_entry_num >= MAX_MISSION_DESCRIPTION_TEXTS ) )
    {
	fprintf ( stderr , "\nmission diary entry number received: %d." , mis_diary_entry_num );
	ErrorMessage ( __FUNCTION__  , "\
There was an illegal mission diary entry number received.",
				   PLEASE_INFORM, IS_FATAL );
    }
    Me . AllMissions [ mis_num ] . mission_description_visible [ mis_diary_entry_num ] = TRUE ;
    Me . AllMissions [ mis_num ] . mission_description_time [ mis_diary_entry_num ] = Me . current_game_date ;
    
}; // void quest_browser_enable_new_diary_entry ( int mis_num , int mis_diary_entry_num )

/* ----------------------------------------------------------------------
 * This function should display the currently assigned/unassigned mission
 * and all that directly over the combat screen without interrupting the
 * game in any other way.
 * ---------------------------------------------------------------------- */
void 
classic_show_mission_list ( void )
{
    int mis_num;
    
    //--------------------
    // If the log is not set to visible right now, we do not need to 
    // do anything more
    //
    if ( GameConfig.Mission_Log_Visible == FALSE ) return;
    if ( GameConfig.Mission_Log_Visible_Time >= GameConfig.Mission_Log_Visible_Max_Time ) return;
    
    SetCurrentFont ( FPS_Display_BFont );

    //--------------------
    // At this point we know, that the quest log is desired and
    // therefore we display it in-game:
    //
    SDL_SetClipRect( Screen , NULL );
    DisplayText( _("\n   See quest log: \n") , User_Rect.x , User_Rect.y , &User_Rect , TEXT_STRETCH );
    
    for ( mis_num = 0 ; mis_num < MAX_MISSIONS_IN_GAME; mis_num ++ )
    {
	// In case the mission does not exist at all, we need not do anything more...
	if ( Me.AllMissions[ mis_num ].MissionExistsAtAll != TRUE ) continue;
	
	// In case the mission was not yet assigned, we need not do anything more...
	// if ( Me.AllMissions[ mis_num ].MissionWasAssigned != TRUE ) continue;
	
	// In case the message is rather old, we need not do anything more...
	// if ( Me.AllMissions[ mis_num ].MissionLastStatusChangeTime > 1000 ) continue;
	
	// At this point we know, that the mission has recently been completed or failed
	
	if ( Me.AllMissions[ mis_num ].MissionIsComplete == TRUE )
	{
	    DisplayText( _("\n* Mission completed: ") , -1 , -1 , &User_Rect , TEXT_STRETCH );
	}
	else if ( Me.AllMissions[ mis_num ].MissionWasFailed == TRUE )
	{
	    DisplayText( _("\n* Mission failed: ") , -1 , -1 , &User_Rect , TEXT_STRETCH );
	}
	else if ( ! Me.AllMissions[ mis_num ].MissionWasAssigned == TRUE )
	{
	    DisplayText( _("\n* Mission not yet assigned: ") , -1 , -1 , &User_Rect , TEXT_STRETCH );
	}
	else 
	    DisplayText( _("\n* Mission assigned: ") , -1 , -1 , &User_Rect , TEXT_STRETCH );
	
	DisplayText( _(Me.AllMissions[ mis_num ].MissionName) , -1 , -1 , &User_Rect , TEXT_STRETCH );
	
    }
}; // void classic_show_mission_list ( void )

/* ----------------------------------------------------------------------
 * If there is some mission selected inside the quest browser, then we
 * should also display all info on the current status and history of that
 * particular mission, which is exactly what this function is responsible
 * for.
 * ---------------------------------------------------------------------- */
void
quest_browser_show_mission_info ( int mis_num )
{
    char temp_text[10000];
    int mission_diary_index;

    if ( ( mis_num < 0 ) || ( mis_num >= MAX_MISSIONS_IN_GAME ) )
    {
	fprintf ( stderr , "\nmission number received: %d." , mis_num );
	ErrorMessage ( __FUNCTION__  , "\
There was an illegal mission number received.",
				   PLEASE_INFORM, IS_FATAL );
    }

    SetTextCursor ( mission_description_rect . x , 
		    mission_description_rect . y );

    DisplayText( _("Mission: ") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    DisplayText( _(Me . AllMissions [ mis_num ] . MissionName) , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    DisplayText( _("\nStatus: ") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    if ( Me . AllMissions [ mis_num ] . MissionIsComplete )
	DisplayText( _("COMPLETE") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    else if ( Me . AllMissions [ mis_num ] . MissionWasFailed )
	DisplayText( _("FAILED") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    else
	DisplayText( _("STILL OPEN") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    DisplayText( _("\nDetails: ") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
    
    for ( mission_diary_index = 0 ; mission_diary_index < MAX_MISSION_DESCRIPTION_TEXTS ;
	  mission_diary_index ++ )
    {
	if ( Me . AllMissions [ mis_num ] . mission_description_visible [ mission_diary_index ] )
	{
	    sprintf ( temp_text , "[day %d %02d:%02d] " , 
		      get_days_of_game_duration ( Me . AllMissions [ mis_num ] . mission_description_time [ mission_diary_index ] ) , 
		      get_hours_of_game_duration ( Me . AllMissions [ mis_num ] . mission_description_time [ mission_diary_index ] ) , 
		      get_minutes_of_game_duration ( Me . AllMissions [ mis_num ] . mission_description_time [ mission_diary_index ] ) ) ;
	    DisplayText( temp_text , -1 , -1 , &mission_description_rect , TEXT_STRETCH );	    
	    DisplayText( D_(mission_diary_texts [ mis_num ] [ mission_diary_index ]), -1 , -1 , &mission_description_rect , TEXT_STRETCH );	    
	    DisplayText( "\n" , -1 , -1 , &mission_description_rect , TEXT_STRETCH );	    
	}
    }

}; // void quest_browser_show_mission_info ( int mis_num )

/* ----------------------------------------------------------------------
 * If there is some mission selected inside the quest browser, then we
 * should also display all info on the current status and history of that
 * particular mission, which is exactly what this function is responsible
 * for.
 * ---------------------------------------------------------------------- */
void
quest_browser_append_mission_info ( int mis_num , int full_description )
{
    char temp_text[10000];
    int mission_diary_index;

    if ( ( mis_num < 0 ) || ( mis_num >= MAX_MISSIONS_IN_GAME ) )
    {
	fprintf ( stderr , "\nmission number received: %d." , mis_num );
	ErrorMessage ( __FUNCTION__  , "\
There was an illegal mission number received.",
				   PLEASE_INFORM, IS_FATAL );
    }

    SetTextCursor ( mission_description_rect . x , 
		    mission_description_rect . y );

    strcat ( complete_mission_display_text , _("Mission: "));
    strcat ( complete_mission_display_text , _(Me . AllMissions [ mis_num ] . MissionName));
    strcat ( complete_mission_display_text , "\n" );

    //--------------------
    // Depending on whether we want the full mission description or only
    // some closed file description, we can either return or must continue
    // to the detailed description.
    //
    if ( ! full_description ) return;

    strcat ( complete_mission_display_text , _("Status: "));
    if ( Me . AllMissions [ mis_num ] . MissionIsComplete )
	strcat ( complete_mission_display_text , _("COMPLETE"));
    else if ( Me . AllMissions [ mis_num ] . MissionWasFailed )
	strcat ( complete_mission_display_text , _("FAILED"));
    else
	strcat ( complete_mission_display_text , _("STILL OPEN"));
    strcat ( complete_mission_display_text , _("\nDetails: "));
    
    for ( mission_diary_index = 0 ; mission_diary_index < MAX_MISSION_DESCRIPTION_TEXTS ;
	  mission_diary_index ++ )
    {
	if ( Me . AllMissions [ mis_num ] . mission_description_visible [ mission_diary_index ] )
	{
	    sprintf ( temp_text , "[day %d %02d:%02d] " , 
		      get_days_of_game_duration ( Me . AllMissions [ mis_num ] . mission_description_time [ mission_diary_index ] ) , 
		      get_hours_of_game_duration ( Me . AllMissions [ mis_num ] . mission_description_time [ mission_diary_index ] ) , 
		      get_minutes_of_game_duration ( Me . AllMissions [ mis_num ] . mission_description_time [ mission_diary_index ] ) ) ;
	    strcat ( complete_mission_display_text , temp_text );	    
	    strcat ( complete_mission_display_text , D_(mission_diary_texts [ mis_num ] [ mission_diary_index ]));
	    strcat ( complete_mission_display_text , "\n" );
	}
    }

}; // void quest_browser_append_mission_info ( int mis_num )

extern int MyCursorY;
/* ----------------------------------------------------------------------
 *
 *
 * ---------------------------------------------------------------------- */
void
quest_browser_display_mission_list ( int list_type )
{
    int mis_num ;
    int something_was_displayed = FALSE ;

    strcpy ( complete_mission_display_text , "" );

    for ( mis_num = 0 ; mis_num < MAX_MISSIONS_IN_GAME; mis_num ++ )
    {
	//--------------------
	// The mission short/long symbol positions must be initialized
	// with some default values.
	//
	quest_browser_mission_lines_needed [ mis_num ] = (-1) ;

	// In case the mission does not exist at all, we need not do anything more...
	if ( Me . AllMissions [ mis_num ] . MissionExistsAtAll != TRUE ) continue;
	
	//  In case the mission was not yet assigned, we need not do anything more...
	if ( Me . AllMissions[ mis_num ] . MissionWasAssigned != TRUE ) continue;

	//--------------------
	// We record the number of lines needed so far, so that we may later
	// draw suitable long/short buttons in front of the text, so that the
	// user can then select to see the full/short information on this quest.
	//
	if ( list_type != QUEST_BROWSER_SHOW_NOTES )
	{
	    quest_browser_mission_lines_needed [ mis_num ] = 
		GetNumberOfTextLinesNeeded ( complete_mission_display_text , 
					     mission_description_rect , 
					     TEXT_STRETCH );
	    DebugPrintf ( 2 , "\n%s(): new mission start pos at lines needed: %d.",
			  __FUNCTION__ , quest_browser_mission_lines_needed [ mis_num ] );
	}

	if ( ( list_type == QUEST_BROWSER_SHOW_OPEN_MISSIONS ) &&
	     ( Me . AllMissions[ mis_num ] . MissionIsComplete == FALSE ) )
	{
	    quest_browser_append_mission_info ( mis_num , Me . AllMissions [ mis_num ] . 
						expanded_display_for_this_mission );
	    something_was_displayed = TRUE ;
	}
	else if ( ( list_type == QUEST_BROWSER_SHOW_DONE_MISSIONS ) &&
	     ( Me . AllMissions[ mis_num ] . MissionIsComplete != FALSE ) )
	{
	    quest_browser_append_mission_info ( mis_num , Me . AllMissions [ mis_num ] . 
						expanded_display_for_this_mission );
	    something_was_displayed = TRUE ;
	}
	else
	{
	    quest_browser_mission_lines_needed [ mis_num ] = (-1) ;
	}

    }

    SetTextCursor ( mission_description_rect . x , 
		    mission_description_rect . y );

    if ( something_was_displayed )
    {
	mission_list_offset = ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) 
	    * ( mission_list_scroll_override_from_user ) * 1.00 ;
	SetTextCursor ( mission_description_rect . x , 
			mission_description_rect . y );
	DisplayText( complete_mission_display_text , mission_description_rect . x , 
		     mission_description_rect . y - mission_list_offset , &mission_description_rect , TEXT_STRETCH );	
	
	//--------------------
	// Now it's time to display some short/long symbols in front
	// of each of the missions.
	//
	for ( mis_num = 0 ; mis_num < MAX_MISSIONS_IN_GAME; mis_num ++ )
	{
            SDL_Rect * rect_short = &(AllMousePressButtons [ QUEST_BROWSER_ITEM_SHORT_BUTTON ] . button_rect);
            SDL_Rect * rect_long = &(AllMousePressButtons [ QUEST_BROWSER_ITEM_LONG_BUTTON ] . button_rect);
	    if ( ! Me . AllMissions [ mis_num ] . MissionWasAssigned ) continue ;
	    if ( ( list_type == QUEST_BROWSER_SHOW_OPEN_MISSIONS ) &&
		                 ( Me . AllMissions[ mis_num ] . MissionIsComplete == TRUE ) ) continue;
	    if ( ( list_type == QUEST_BROWSER_SHOW_DONE_MISSIONS ) &&
		                 ( Me . AllMissions[ mis_num ] . MissionIsComplete == FALSE ) ) continue;
	    //--------------------
	    // At first we bring the short/long buttons into position.
	    // This position might be well off the screen.  That's no
	    // problem, because we won't blit the thing in this case
	    // anyway.
	    //
	    if ( quest_browser_mission_lines_needed [ mis_num ] != (-1) )
	    {
		rect_short -> y = 
		    mission_description_rect . y - mission_list_offset + 
		    ( FontHeight ( GetCurrentFont() ) * TEXT_STRETCH ) *
		    ( quest_browser_mission_lines_needed [ mis_num ] - 1 ) - 3 ;
                rect_short -> x = mission_description_rect . x - rect_short -> w;
		rect_long -> y = rect_short -> y ;
		rect_long -> x = rect_short -> x ;
	    }

	    //--------------------
	    // Now we check if the y coordinate of the buttons are 
	    // somewhat reasonable or not.  For those buttons that are
	    // off the screen, things are simple, because then we can
	    // skip the rest of this pass of the loop.
	    //
	    if ( rect_short -> y <= mission_description_rect . y - 4 ) continue;
	    if ( rect_short -> y 
		 >= mission_description_rect . y + mission_description_rect . h - FontHeight ( GetCurrentFont() ) ) 
                continue;

	    if ( Me . AllMissions [ mis_num ] . expanded_display_for_this_mission )
		ShowGenericButtonFromList ( QUEST_BROWSER_ITEM_LONG_BUTTON );
	    else
		ShowGenericButtonFromList ( QUEST_BROWSER_ITEM_SHORT_BUTTON );

	    if ( MouseLeftPressed() )
	    {
		if ( MouseCursorIsOnButton ( QUEST_BROWSER_ITEM_SHORT_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
		{
		    Me . AllMissions [ mis_num ] . expanded_display_for_this_mission =
			! Me . AllMissions [ mis_num ] . expanded_display_for_this_mission ;
		    while ( MouseLeftPressed() );
		}
	    }
	}
    }
    else
    {
	switch ( list_type )
	{
	    case QUEST_BROWSER_SHOW_OPEN_MISSIONS:
		DisplayText( _("No open quests yet.") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
		break;
	    case QUEST_BROWSER_SHOW_DONE_MISSIONS:
		DisplayText( _("No completed quests yet.") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
		break;
	    case QUEST_BROWSER_SHOW_NOTES:
		DisplayText( _("No notes yet.") , -1 , -1 , &mission_description_rect , TEXT_STRETCH );
		break;
	    default:
		ErrorMessage ( __FUNCTION__  , "\
Illegal quest browser status encountered.",
					   PLEASE_INFORM, IS_FATAL );
		break;
	}
    }
    
    ShowGenericButtonFromList ( QUEST_BROWSER_SCROLL_UP_BUTTON );
    ShowGenericButtonFromList ( QUEST_BROWSER_SCROLL_DOWN_BUTTON );

}; // void quest_browser_display_mission_list ( void )

/* ----------------------------------------------------------------------
 * This function manages the quest browser.
 * ---------------------------------------------------------------------- */
void
quest_browser_interface ( void )
{
    int back_to_game = FALSE;
    static int first_call = TRUE ;

    //--------------------
    // On the very first 
    if ( first_call )
    {
	first_call = FALSE ;
	mission_description_rect . x *= (((float)GameConfig . screen_width) / 640.0 ) ;
	mission_description_rect . y *= (((float)GameConfig . screen_height) / 480.0 ) ;
	mission_description_rect . w *= (((float)GameConfig . screen_width) / 640.0 ) ;
	mission_description_rect . h *= (((float)GameConfig . screen_height) / 480.0 ) ;
    }

    //--------------------
    // This might take some time, so we need to be careful here,
    // so as not to generate a massive frame time, that would
    // throw every moving thing from the map.
    //
    Activate_Conservative_Frame_Computation ();
    make_sure_system_mouse_cursor_is_turned_off ( );
    SetCurrentFont ( FPS_Display_BFont );

    while ( EscapePressed() );
    while ( QPressed() );

    blit_special_background ( QUEST_BROWSER_BACKGROUND_CODE );
    StoreMenuBackground ( 1 );

    while ( ! back_to_game )
    {
	track_last_frame_input_status(); //enable use of *WasPressed functions
	
	SDL_Delay ( 1 ); 

	RestoreMenuBackground ( 1 );
	if ( current_quest_browser_mode == QUEST_BROWSER_SHOW_OPEN_MISSIONS )
	    ShowGenericButtonFromList ( QUEST_BROWSER_OPEN_QUESTS_BUTTON );
	else
	    ShowGenericButtonFromList ( QUEST_BROWSER_OPEN_QUESTS_OFF_BUTTON );
	if ( current_quest_browser_mode == QUEST_BROWSER_SHOW_DONE_MISSIONS )
	    ShowGenericButtonFromList ( QUEST_BROWSER_DONE_QUESTS_BUTTON );
	else
	    ShowGenericButtonFromList ( QUEST_BROWSER_DONE_QUESTS_OFF_BUTTON );
	if ( current_quest_browser_mode == QUEST_BROWSER_SHOW_NOTES )
	    ShowGenericButtonFromList ( QUEST_BROWSER_NOTES_BUTTON );
	else
	    ShowGenericButtonFromList ( QUEST_BROWSER_NOTES_OFF_BUTTON );

	quest_browser_display_mission_list ( current_quest_browser_mode );

	if ( currently_selected_mission != (-1) )
	    quest_browser_show_mission_info ( currently_selected_mission );

	blit_our_own_mouse_cursor();
	our_SDL_flip_wrapper ( Screen );
	    
	if ( QPressed() )
	{
	    while ( QPressed() ) SDL_Delay(1);
	    back_to_game = TRUE ;
	}

	if ( MouseLeftPressed() || SpacePressed() )
	{
	    if ( MouseCursorIsOnButton ( QUEST_BROWSER_OPEN_QUESTS_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
	    {
		current_quest_browser_mode = QUEST_BROWSER_SHOW_OPEN_MISSIONS ;
		mission_list_scroll_override_from_user = 0 ;
		while ( MouseLeftPressed() ) SDL_Delay(1);
	    }
	    if ( MouseCursorIsOnButton ( QUEST_BROWSER_DONE_QUESTS_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
	    {
		current_quest_browser_mode = QUEST_BROWSER_SHOW_DONE_MISSIONS ;
		mission_list_scroll_override_from_user = 0 ;
		while ( MouseLeftPressed() ) SDL_Delay(1);
	    }
	    if ( MouseCursorIsOnButton ( QUEST_BROWSER_NOTES_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
	    {
		current_quest_browser_mode = QUEST_BROWSER_SHOW_NOTES ;
		mission_list_scroll_override_from_user = 0 ;
		while ( MouseLeftPressed() ) SDL_Delay(1);
	    }
	    if ( MouseCursorIsOnButton ( QUEST_BROWSER_SCROLL_UP_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
	    {
		mission_list_scroll_override_from_user -- ;
		if ( mission_list_scroll_override_from_user < 0 )
		    mission_list_scroll_override_from_user = 0; 
		while ( MouseLeftPressed() ) SDL_Delay(1);
	    }
	    if ( MouseCursorIsOnButton ( QUEST_BROWSER_SCROLL_DOWN_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
	    {
		mission_list_scroll_override_from_user ++ ;
		while ( MouseLeftPressed() ) SDL_Delay(1);
	    }

            if ( MouseCursorIsOnButton ( QUEST_BROWSER_EXIT_BUTTON , GetMousePos_x() , GetMousePos_y() ) )
	    {
		back_to_game = TRUE ;
		while ( MouseLeftPressed() ) SDL_Delay(1);
	    }


	}

	    
    }

}; // void quest_browser_interface ( void )


/*----------------------------------------------------------------------
 * This function checks, if the influencer has succeeded in his given 
 * mission.  If not it returns, if yes the EndTitle/Debriefing is
 * started.
 ----------------------------------------------------------------------*/
void 
CheckIfMissionIsComplete (void)
{
    int ItemCounter;
    int mis_num;
    int ActionNum;
    static int CheckMissionGrid; 
    int this_mission_seems_completed = TRUE ;
#define MIS_COMPLETE_DEBUG 1
    
    //--------------------
    // We do not need to check for mission completed EVERY frame
    // It will be enough to do it now and then..., e.g. every 50th frame
    //
    CheckMissionGrid++;
    if ( ( CheckMissionGrid % 50 ) != 0 ) return;
    
    for ( mis_num = 0 ; mis_num < MAX_MISSIONS_IN_GAME ; mis_num ++ )
    {
	
	//--------------------
	// We need not do anything, if the mission has already failed or if
	// the mission is already completed or if the mission does not exist
	// at all or if the mission was not assigned yet
	//
	if ( Me . AllMissions [ mis_num ] . MissionIsComplete == TRUE ) continue;
	if ( Me . AllMissions [ mis_num ] . MissionWasFailed == TRUE ) continue;
	if ( Me . AllMissions [ mis_num ] . MissionExistsAtAll != TRUE ) continue;
	if ( Me . AllMissions [ mis_num ] . MissionWasAssigned != TRUE ) continue;
	
	DebugPrintf ( MIS_COMPLETE_DEBUG , "\nSomething was assigned at all..... mis_num = %d " , mis_num );
	
	this_mission_seems_completed = TRUE ;
	
	//--------------------
	// Continue if the Mission target KillOne is given but not fullfilled
	//
	if ( Me . AllMissions [ mis_num ].KillOne != (-1) )
	{
	    enemy *erot, *nerot;
BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
	    {
		if ( ( erot->marker == Me . AllMissions [ mis_num ] . KillOne ) )
		{
		    DebugPrintf ( MIS_COMPLETE_DEBUG , "\nOne of the marked droids is still alive...");
		    this_mission_seems_completed = FALSE ;
		    break;
		}

	    }
	}
	
	//--------------------
	// Continue if the Mission target fetch_item is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].fetch_item != (-1) )
	{
	    
	    for ( ItemCounter = 0 ; ItemCounter < MAX_ITEMS_IN_INVENTORY ; ItemCounter++ )
	    {
		if ( Me . Inventory [ ItemCounter ] . type == Me.AllMissions[ mis_num ].fetch_item )
		{
		    DebugPrintf ( MIS_COMPLETE_DEBUG , "\nDesired item IS PRESENT!!");
		    break;
		}
	    }
	    if ( ItemCounter >= MAX_ITEMS_IN_INVENTORY ) 
	    {
		// goto CheckNextMission;
		this_mission_seems_completed = FALSE ;
	    }
	}
	
	//--------------------
	// Continue if the Mission target must_clear_first_level is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ]. must_clear_first_level != (-1) )
	{
	    enemy *erot, *nerot;
BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
	    {
	    	if ( ( ! erot->is_friendly ) && 
		     ( erot->pos . z == Me.AllMissions[ mis_num ].must_clear_first_level ) ) 
		{
		    this_mission_seems_completed = FALSE ;
		    break;
		}
	    }
	}
	
	//--------------------
	// Continue if the Mission target must_clear_second_level is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ]. must_clear_second_level != (-1) )
	{
	    enemy *erot, *nerot;
BROWSE_ALIVE_BOTS_SAFE(erot, nerot)
	    {
		if ( ( ! erot->is_friendly ) && 
		     ( erot->pos . z == Me.AllMissions[ mis_num ].must_clear_second_level ) ) 
		{
		    this_mission_seems_completed = FALSE ;
		    break;
		}
	    }
	}
	
	
	//--------------------
	// Continue if the Mission target MustBeType is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].MustBeType != (-1) )
	{
	    DebugPrintf ( MIS_COMPLETE_DEBUG , "\nPlayer.type is now: %d.", Me.type );
	    if ( Me.type != Me.AllMissions[ mis_num ].MustBeType ) 
	    {
		DebugPrintf ( MIS_COMPLETE_DEBUG , "\nPlayer.type does not match...");
		continue;
	    }
	}
	
	
	//--------------------
	// Continue if the Mission target MustReachLevel is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].MustReachLevel != (-1) )
	{
	    if ( Me . pos . z != Me.AllMissions[ mis_num ].MustReachLevel ) 
	    {
		DebugPrintf ( MIS_COMPLETE_DEBUG , "\nLevel number does not match...");
		continue;
	    }
	}
	
	//--------------------
	// Continue if the Mission target MustReachPoint.x is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].MustReachPoint.x != (-1) )
	{
	    if ( Me.pos.x != Me.AllMissions[ mis_num ].MustReachPoint.x ) 
	    {
		DebugPrintf ( MIS_COMPLETE_DEBUG , "\nX coordinate does not match...");
		continue;
	    }
	}
	
	//--------------------
	// Continue if the Mission target MustReachPoint.y is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].MustReachPoint.y != (-1) )
	{
	    if ( Me.pos.y != Me.AllMissions[ mis_num ].MustReachPoint.y ) 
	    {
		DebugPrintf ( MIS_COMPLETE_DEBUG , "\nY coordinate does not match..."); 
		continue;
	    }
	}
	
	//--------------------
	// Continue if the Mission target MustLiveTime is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].MustLiveTime != (-1) )
	{
	    if ( Me.MissionTimeElapsed < Me.AllMissions[ mis_num ].MustLiveTime ) 
	    {
		DebugPrintf ( MIS_COMPLETE_DEBUG , "\nTime Limit not yet reached...");
		continue;
	    }
	}
	
	//--------------------
	// Continue if the Mission target MustBeOne is given but not fullfilled
	//
	if ( Me.AllMissions[ mis_num ].MustBeOne != (-1) )
	{
	    if ( Me.marker != Me.AllMissions[ mis_num ].MustBeOne ) 
	    {
		DebugPrintf ( MIS_COMPLETE_DEBUG , "\nYou're not yet one of the marked ones...");
		continue;
	    }
	}
	
	if ( this_mission_seems_completed )
	{
	    //--------------------
	    // AT THIS POINT WE KNOW THAT ALL OF THE GIVEN TARGETS FOR THIS MISSION ARE FULLFILLED
	    // We therefore mark the mission as completed
	    //
	    GameConfig.Mission_Log_Visible_Time = 0;
	    GameConfig.Mission_Log_Visible = TRUE;
	    Me.AllMissions[ mis_num ].MissionIsComplete = TRUE;
	    Mission_Status_Change_Sound ( );
	    for ( ActionNum = 0 ; ActionNum < MAX_MISSION_TRIGGERED_ACTIONS ; ActionNum ++ )
	    {
		ExecuteEvent( Me.AllMissions[ mis_num ].ListOfActionsToBeTriggeredAtCompletition[ ActionNum ] );
	    }
	}
	
    } // for AllMissions
    
}; // void CheckIfMissionIsComplete

/* ----------------------------------------------------------------------
 * This function assigns a new mission to the influencer, which means 
 * that the status of the mission in the mission array is changed and
 * perhaps the mission log activated.
 * ---------------------------------------------------------------------- */
void 
AssignMission( int MissNum )
{
    int j;

    //--------------------
    // First some sanity check for the mission number received.
    //
    if ( ( MissNum < 0 ) || ( MissNum >= MAX_MISSIONS_IN_GAME ) )
    {
	fprintf ( stderr , "\nmission number: %d." , MissNum ) ;
	ErrorMessage ( __FUNCTION__  , "\
There was a mission number received that is outside the range of allowed values.",
				   PLEASE_INFORM, IS_FATAL );
    }
    
    Mission_Status_Change_Sound ( );
    GameConfig.Mission_Log_Visible = TRUE;
    GameConfig.Mission_Log_Visible_Time = 0;
    Me . AllMissions [ MissNum ] . MissionWasAssigned = TRUE;
    
    for ( j = 0 ; j < MAX_MISSION_TRIGGERED_ACTIONS ; j ++ )
    {
	ExecuteEvent( Me . AllMissions [ MissNum ] . ListOfActionsToBeTriggeredAtAssignment [ j ] );
    }

    //--------------------
    // We also make visible the very first of the mission diary enties. 
    // That should be sane as upon mission assignment, there should always
    // be some first diary entry, and usually there's only one way a mission
    // can be assigned, so it's safe to do that automatically.
    //
    Me . AllMissions [ MissNum ] . mission_description_visible [ 0 ] = TRUE;
    quest_browser_enable_new_diary_entry ( MissNum , 0 );

}; // void AssignMission( int MissNum );

/* ----------------------------------------------------------------------
 * At the start of every new game, the mission info (i.e. which missions
 * are already assigned, completed, failed, available and such) should
 * be reset to default state, so that no zombie mission entries can appear. 
 * ---------------------------------------------------------------------- */
void
clear_tux_mission_info ( )
{
    int MissionTargetIndex;
    int diary_entry_nr;

    //--------------------
    // At first we clear out all existing mission entries, so that no 'zombies' remain
    // when the game is restarted and (perhaps less) new missions are loaded.
    //
    for ( MissionTargetIndex = 0 ; MissionTargetIndex < MAX_MISSIONS_IN_GAME ; MissionTargetIndex ++ )
    {
	Me . AllMissions [ MissionTargetIndex ] . MissionExistsAtAll = FALSE;
	Me . AllMissions [ MissionTargetIndex ] . MissionIsComplete = FALSE;
	Me . AllMissions [ MissionTargetIndex ] . MissionWasFailed = FALSE;
	Me . AllMissions [ MissionTargetIndex ] . MissionWasAssigned = FALSE;

	for ( diary_entry_nr = 0 ; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS ; diary_entry_nr ++ )
	{
	    mission_diary_texts [ MissionTargetIndex ] [ diary_entry_nr ] = "" ;
	    Me . AllMissions [ MissionTargetIndex ] . mission_description_visible [ diary_entry_nr ] = FALSE ;
	    Me . AllMissions [ MissionTargetIndex ] . mission_description_time [ diary_entry_nr ] = 0 ;
	}
    }

}; // void clear_tux_mission_info ( )

/* ----------------------------------------------------------------------
 * This function reads the mission specifications from the mission file
 * which is assumed to be loaded into memory already.
 * ---------------------------------------------------------------------- */
void 
GetQuestList ( char* QuestListFilename )
{
    char *EndOfMissionTargetPointer;
    char *NextEventPointer;
    int MissionTargetIndex = 0;
    int NumberOfEventsToTriggerAtThisAssignment;
    int NumberOfEventsToTriggerAtThisCompletition;
    int ActionNr;
    char* ActionLabel;
    char* MissionTargetPointer;
    char fpath[2048];
    char* MissionFileContents;
    char InnerPreservedLetter=0;
    int diary_entry_nr;
    char* next_diary_entry_pointer;
    int number_of_diary_entries;

    
#define MISSION_TARGET_SUBSECTION_START_STRING "** Start of this mission target subsection **"
#define MISSION_TARGET_SUBSECTION_END_STRING "** End of this mission target subsection **"

#define MISSION_TARGET_NAME_INITIALIZER "Mission Name=\""

#define MISSION_AUTOMATICALLY_ASSIGN_STRING "Assign this mission to influencer automatically at start : "
#define MISSION_TARGET_FETCH_ITEM_STRING "Mission target is to fetch item : \""
#define MISSION_TARGET_KILL_ONE_STRING "Mission target is to kill droids with marker : "
#define MISSION_TARGET_MUST_CLEAR_FIRST_LEVEL "Mission target is to kill all hostile droids this first level : "
#define MISSION_TARGET_MUST_CLEAR_SECOND_LEVEL "Mission target is to also kill all hostile droids on second level : "
#define MISSION_TARGET_MUST_REACH_LEVEL_STRING "Mission target is to reach level : "
#define MISSION_TARGET_MUST_REACH_POINT_X_STRING "Mission target is to reach X-Pos : "
#define MISSION_TARGET_MUST_REACH_POINT_Y_STRING "Mission target is to reach Y-Pos : "
#define MISSION_TARGET_MUST_LIVE_TIME_STRING "Mission target is to live for how many seconds : "
#define MISSION_TARGET_MUST_BE_TYPE_STRING "Mission target is to become type : "
#define MISSION_TARGET_MUST_BE_ONE_STRING "Mission target is to overtake a droid with marker : "

#define MISSION_ASSIGNMENT_TRIGGERED_ACTION_STRING "On mission assignment immediately trigger action Nr. : "
#define MISSION_COMPLETITION_TRIGGERED_ACTION_STRING "On mission completition immediately trigger action labeled=\""
#define MISSION_DIARY_ENTRY_STRING "Mission diary entry=\""

    //--------------------
    // At first we must load the quest list file given...
    //
    find_file (QuestListFilename , MAP_DIR , fpath, 0 );
    MissionFileContents = ReadAndMallocAndTerminateFile( fpath , "*** END OF QUEST LIST *** LEAVE THIS TERMINATOR IN HERE ***" ) ;
    MissionTargetPointer = MissionFileContents;

    for ( MissionTargetIndex = 0 ; MissionTargetIndex < MAX_MISSIONS_IN_GAME ; MissionTargetIndex ++ )
    {
	Me . AllMissions [ MissionTargetIndex ] . MissionExistsAtAll = FALSE;
    }
    
    MissionTargetIndex = 0;
    while ( ( MissionTargetPointer = strstr( MissionTargetPointer , MISSION_TARGET_SUBSECTION_START_STRING ) ) != NULL )
    {
	EndOfMissionTargetPointer = LocateStringInData ( MissionTargetPointer , MISSION_TARGET_SUBSECTION_END_STRING ) ;
	
	//--------------------
	// We need to add an inner terminator here, so that the strstr operation
	// below will know where to stop within this subsection.
	//
	InnerPreservedLetter = * EndOfMissionTargetPointer;
	* EndOfMissionTargetPointer = 0 ;
	
	Me . AllMissions [ MissionTargetIndex ] . MissionExistsAtAll = TRUE;
	
	// Me.AllMissions[ MissionTargetIndex ].MissionName = 
	// ReadAndMallocStringFromData ( MissionTargetPointer , MISSION_TARGET_NAME_INITIALIZER , "\"" ) ;
	char * tmname =  ReadAndMallocStringFromData ( MissionTargetPointer , MISSION_TARGET_NAME_INITIALIZER , "\"" );
	strcpy ( Me.AllMissions[ MissionTargetIndex ].MissionName , tmname ) ;
	free ( tmname ) ;
	//--------------------
	// No we read in if this mission should be assigned to the influencer
	// automatically at the game start and without the influencer having to apply
	// for the mission first.
	//
	// The assignment however will take at the end of this mission list initialisation function,
	// cause we need the rest of the mission target data and the events to properly 'assign' the mission.
	// 
	ReadValueFromString( MissionTargetPointer , MISSION_AUTOMATICALLY_ASSIGN_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].AutomaticallyAssignThisMissionAtGameStart , 
			     EndOfMissionTargetPointer );
	
	//--------------------
	// From here on we read the details of the mission target, i.e. what the
	// influencer has to do, so that the mission can be thought of as completed
	//
	if ( strstr( MissionTargetPointer, MISSION_TARGET_FETCH_ITEM_STRING ) )
	    {
	    char * iname = ReadAndMallocStringFromData ( MissionTargetPointer , MISSION_TARGET_FETCH_ITEM_STRING , "\"");
	    Me.AllMissions[ MissionTargetIndex ].fetch_item = GetItemIndexByName(iname);
	    free ( iname ); 
	    }
	else Me.AllMissions[ MissionTargetIndex ].fetch_item = -1; 
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_KILL_ONE_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].KillOne , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_CLEAR_FIRST_LEVEL , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ]. must_clear_first_level , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_CLEAR_SECOND_LEVEL , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ]. must_clear_second_level , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_BE_TYPE_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].MustBeType , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_BE_ONE_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].MustBeOne , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_REACH_POINT_X_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].MustReachPoint.x , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_REACH_POINT_Y_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].MustReachPoint.y , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_REACH_LEVEL_STRING , "%d" , 
			     &Me.AllMissions[ MissionTargetIndex ].MustReachLevel , EndOfMissionTargetPointer );
	
	ReadValueFromString( MissionTargetPointer , MISSION_TARGET_MUST_LIVE_TIME_STRING , "%lf" , 
			     &Me.AllMissions[ MissionTargetIndex ].MustLiveTime , EndOfMissionTargetPointer );
	
	//--------------------
	// At this point we have read in the target values.  Now it is time to
	// read in the events, that need to be triggered immediately after the mission has been
	// assigned.
	//
	// But first we initialize all the actions to be triggered with -1
	//
	for ( ActionNr = 0 ; ActionNr < MAX_MISSION_TRIGGERED_ACTIONS; ActionNr ++ )
	{
	    Me.AllMissions[ MissionTargetIndex ].ListOfActionsToBeTriggeredAtAssignment[ ActionNr ] = (-1) ;
	}
	
	NextEventPointer = MissionTargetPointer;
	NumberOfEventsToTriggerAtThisAssignment = 0;
	while ( ( NextEventPointer = strstr( NextEventPointer , MISSION_ASSIGNMENT_TRIGGERED_ACTION_STRING ) ) != NULL )
	{
	    
	    ReadValueFromString( NextEventPointer , MISSION_ASSIGNMENT_TRIGGERED_ACTION_STRING , "%d" ,
				 &Me.AllMissions[ MissionTargetIndex ].ListOfActionsToBeTriggeredAtAssignment[ NumberOfEventsToTriggerAtThisAssignment ] ,
				 EndOfMissionTargetPointer );
	    
	    
	    NumberOfEventsToTriggerAtThisAssignment ++;
	    NextEventPointer ++;
	}
	DebugPrintf ( 1 , "\nDetected %d events to be triggered at this assignment." , 
		      NumberOfEventsToTriggerAtThisAssignment ) ;
	
	//--------------------
	// Now it is time to read in the events, that need to be triggered immediately after the
	// mission has been completed.
	//
	// But first we initialize all the actions to be triggered with -1
	//
	for ( ActionNr = 0 ; ActionNr < MAX_MISSION_TRIGGERED_ACTIONS; ActionNr ++ )
	{
	    Me.AllMissions[ MissionTargetIndex ].ListOfActionsToBeTriggeredAtCompletition[ ActionNr ] = (-1) ;
	}
	NextEventPointer = MissionTargetPointer;
	NumberOfEventsToTriggerAtThisCompletition = 0;
	while ( ( NextEventPointer = strstr( NextEventPointer , MISSION_COMPLETITION_TRIGGERED_ACTION_STRING ) ) != NULL )
	{
	    
	    ActionLabel=
		ReadAndMallocStringFromData ( NextEventPointer , MISSION_COMPLETITION_TRIGGERED_ACTION_STRING , "\"" ) ;
	    
	    Me . AllMissions [ MissionTargetIndex ] . ListOfActionsToBeTriggeredAtCompletition [ NumberOfEventsToTriggerAtThisCompletition ] = GiveNumberToThisActionLabel ( ActionLabel );
	    
	    NumberOfEventsToTriggerAtThisCompletition ++;
	    NextEventPointer ++;
	    free ( ActionLabel );
	}
	DebugPrintf ( 1 , "\nDetected %d events to be triggered at this mission completition." , 
		      NumberOfEventsToTriggerAtThisCompletition );
	
	//--------------------
	// Now it is time to read in the mission diary entries, that might
	// be displayed in the quest browser later.
	//
	for ( diary_entry_nr = 0 ; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS ; diary_entry_nr ++ )
	{
	    mission_diary_texts [ MissionTargetIndex ] [ diary_entry_nr ] = "" ;
	    // Me . AllMissions [ MissionTargetIndex ] . mission_description_visible [ diary_entry_nr ] = FALSE ;
	    // Me . AllMissions [ MissionTargetIndex ] . mission_description_time [ diary_entry_nr ] = 0 ;
	}
	next_diary_entry_pointer = MissionTargetPointer;
	number_of_diary_entries = 0;
	while ( ( next_diary_entry_pointer = strstr( next_diary_entry_pointer , MISSION_DIARY_ENTRY_STRING ) ) != NULL )
	{    
	    mission_diary_texts [ MissionTargetIndex ] [ number_of_diary_entries ] = 
		ReadAndMallocStringFromData ( next_diary_entry_pointer , MISSION_DIARY_ENTRY_STRING , "\"" ) ;
	    DebugPrintf ( 1 , "\n\nFound new mission_diary_text (%d,%d): %s.\n\n" ,
			  MissionTargetIndex , number_of_diary_entries , mission_diary_texts [ MissionTargetIndex ] [ number_of_diary_entries ] );
	    number_of_diary_entries ++;
	    next_diary_entry_pointer ++;
	}
	DebugPrintf ( 1 , "\n%s(): Detected %d mission description entries." , 
		      __FUNCTION__ , number_of_diary_entries );
	
	//--------------------
	// Now we are done with reading in THIS one mission target
	// We need to advance the MissionTargetPointer, so that we avoid doubly
	// reading in this mission OR ONE OF THIS MISSIONS VALUES!!!!
	// 
	// And we need of course to advance the array index for mission targets too...
	//
	MissionTargetPointer = EndOfMissionTargetPointer; // to avoid double entering the same target
	MissionTargetIndex++; // to avoid overwriting the same entry again
	
	//--------------------
	// We restore the termination character we added before, even if that
	// is maybe not really nescessary...
	//
	* EndOfMissionTargetPointer = InnerPreservedLetter ;
	
	
    } // while mission target found...
    

    //--------------------
    // Finally we record the number of mission targets scanned and are done with this function
    DebugPrintf ( 1 , "\nNUMBER OF MISSION TARGETS FOUND: %d.\n" , MissionTargetIndex );
    fflush( stdout );
    free ( MissionFileContents );
}; // void Get_Mission_Targets( ... )


#undef _mission_c
