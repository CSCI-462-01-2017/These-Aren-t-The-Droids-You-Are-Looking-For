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
 * This file contains all features, movement, fireing, collision and 
 * extras of the influencer.
 */

#define _influ_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "proto.h"

#define BEST_MELEE_DISTANCE (0.8)
#define BEST_CHAT_DISTANCE (BEST_MELEE_DISTANCE+0.2)
#define DISTANCE_TOLERANCE (0.00002)

#define LEVEL_JUMP_DEBUG 1

void limit_tux_speed_to_a_maximum ( );
int autorun_activated = 0;
void check_for_chests_to_open ( int chest_index ) ;
void check_for_barrels_to_smash ( int index_of_barrel_below_mouse_cursor ) ;
void check_for_items_to_pickup ( int );

int no_left_button_press_in_previous_analyze_mouse_click = FALSE ;

/**
 *
 *
 */
float
calc_euklid_distance ( float pos1_x , float pos1_y , float pos2_x , float pos2_y ) 
{
  return sqrt ( (pos1_x-pos2_x)*(pos1_x-pos2_x) + (pos1_y-pos2_y)*(pos1_y-pos2_y) ) ;
};

/**
 *
 *
 */
float
vect_len ( moderately_finepoint our_vector )
{
  return ( sqrt ( powf ( our_vector . x , 2 ) + powf ( our_vector . y , 2 ) ) ) ;
}; // float vect_len ( moderately_finepoint our_vector )

/**
 * This function adapts the influencers current speed to the maximal speed
 * possible for the influencer (determined by the currely used drive type).
 */
void
limit_tux_speed_to_a_maximum ( )
{
    double maxspeed = TUX_RUNNING_SPEED ;
    const double speed = sqrt(Me.speed.x * Me.speed.x + Me.speed.y * Me.speed.y);
    if(speed > maxspeed) {
      const double ratio = maxspeed/speed;    
      Me.speed.x *= ratio;
      Me.speed.y *= ratio;
    }
}; // void limit_tux_speed_to_a_maximum ( ) 

/**
 * This function reduces the influencers speed as long as no direction 
 * key of any form is pressed and also no mouse move target is set.
 */
void
tux_friction_with_air ( )
{
    if ( Me . next_intermediate_point [ 0 ] . x != (-1) ) 
	return;
    
    Me . speed . x = 0 ;
    Me . speed . y = 0 ;
    
}; // tux_friction_with_air ( )

/**
 *
 *
 */
int
find_free_floor_items_index ( int levelnum ) 
{
    int i;
    Level DropLevel = curShip . AllLevels [ levelnum ] ;
    
    for ( i = 0 ; i < MAX_ITEMS_PER_LEVEL ; i ++ )
    {
	if ( DropLevel -> ItemList [ i ] . type == ( -1 ) ) return ( i ) ;
    }
    
    //--------------------
    // Now at this point we know, that we didn't succeed in finding a 
    // free index for new items on the floor of this level.  Ok.  No need
    // to panic.  We can overwrite one old item anyway.  In the mass of
    // items lying around, it shouldn't become apparent anyway, provided
    // we don't overwrite the same position again and again.  A bit of 
    // randomisation should do the trick...
    //
    i = MyRandom ( MAX_ITEMS_PER_LEVEL - 2 );
    DebugPrintf ( -1 , "\n%s():  NOTE:  lots of items on the floor of this level.  Overwriting position %d." ,
		  __FUNCTION__ , i ) ;

    return ( i ) ;
    
}; // int find_free_floor_items_index ( int levelnum ) 

/**
 * Find a reachable position to drop an item near a chest.
 *
 * This function looks for a position near a given obstacle that
 * satisfies : DLC(obstacle, item) && DLW(Tux, item)
 * If no position is found, Tux current position is used as a fallback.
 *
 * Note: a real random position is not used, in order to minimize the CPU cost
 */
void find_position_near_obstacle( float* item_x, float* item_y, int obst_index, Level obst_level )
{
	float obst_x = obst_level->obstacle_list[obst_index].pos.x;
	float obst_y = obst_level->obstacle_list[obst_index].pos.y;
	moderately_finepoint offset_vector;
	int tries;

	// Initialize the item position with the fallback position
	*item_x = Me.pos.x;
	*item_y = Me.pos.y;

	// Step 1: randomly choose one of the 8 main 45° directions around the obstacle
	float obs_diag = obstacle_map[obst_level->obstacle_list[obst_index].type].diaglength;
	offset_vector.x = obs_diag + 0.5;
	offset_vector.y = 0.0;
	RotateVectorByAngle(&offset_vector, (float)MyRandom(8)*45.0);

	// Step 2: rotate offset_vector by 45° until an available start position is found
	colldet_filter filter = { ObstacleByIdPassFilterCallback, &obst_index, NULL };

	tries = 0;
	while ( ( !DirectLineColldet(obst_x, obst_y, obst_x + offset_vector.x, obst_y + offset_vector.y, Me.pos.z, &filter) ||
			  !DirectLineColldet(Me.pos.x, Me.pos.y, obst_x + offset_vector.x, obst_y + offset_vector.y, Me.pos.z, &WalkablePassFilter) )
			&& ( tries < 8 ) )
	{
		RotateVectorByAngle(&offset_vector, 45.0);
		++tries;
	}
	if ( tries == 8 ) return; // No start position available : fallback to Tux's feet

	// New fallback position will be that start position
	*item_x = obst_x + offset_vector.x;
	*item_y = obst_y + offset_vector.y;

	// Step 3 : randomly choose an available position around that start position
	tries = 0;
	float trimmer_x, trimmer_y;
	do
	{
		trimmer_x = (float)MyRandom(10)/20.0 - 0.25;
		trimmer_y = (float)MyRandom(10)/20.0 - 0.25;
		++tries;
	}
	while ( ( !DirectLineColldet(obst_x, obst_y, *item_x + trimmer_x, *item_y + trimmer_y, Me.pos.z, &filter) ||
	          !DirectLineColldet(Me.pos.x, Me.pos.y, *item_x + trimmer_x, *item_y + trimmer_y, Me.pos.z, &WalkablePassFilter) )
	        && ( tries < 100 ) );
	if ( tries == 100 ) return; // No final position available : fallback to start position

	// Step 4 : position found
	*item_x += trimmer_x;
	*item_y += trimmer_y;

} // void find_position_near_obstacle( float* item_x, float* item_y, int obst_index, int obst_level )


/**
 *
 *
 */
void throw_out_all_chest_content ( int obst_index )
{
  Level chest_level;
  int i;
  int j;
  float item_x, item_y;
  int icnt = 0;

  chest_level = curShip . AllLevels [ Me . pos . z ] ;

  DebugPrintf ( 0 , "\nthrow_out_all_chest_content: call confimed." );

  //--------------------
  // First some check if the given obstacle is really a closed chest.
  //
  switch ( chest_level -> obstacle_list [ obst_index ] . type )
    {
    case ISO_H_CHEST_CLOSED:
    case ISO_V_CHEST_CLOSED:
      // all is ok in this case.  it's really a chest.  fine.
      break;
    default: 
      // no chest handed as the chest obstacle!  Clearly a severe error.!
      ErrorMessage ( __FUNCTION__  , "Obstacle given to empty is not really a chest!" ,
				 PLEASE_INFORM, IS_FATAL );
      break;
    }

  //--------------------
  // Now we can throw out all the items from inside the chest and maybe
  // (later) also play a 'chest opening' sound.
  //
  for ( i = 0 ; i < MAX_CHEST_ITEMS_PER_LEVEL ; i ++ )
    {
      if ( chest_level -> ChestItemList [ i ] . type == (-1) ) continue;

      // An item is defined to be in a chest if it is at the same position than the chest
      if ( fabsf ( chest_level -> obstacle_list [ obst_index ] . pos . x - chest_level -> ChestItemList [ i ] . pos . x ) > 0.1 ) continue ;
      if ( fabsf ( chest_level -> obstacle_list [ obst_index ] . pos . y - chest_level -> ChestItemList [ i ] . pos . y ) > 0.1 ) continue ;
      
      //--------------------
      // So this item is one of the right one and will now get thrown out of the chest:
      // 
      // First we find a free items index on this level.
      //
      DebugPrintf ( 0 , "\nOne item now thrown out of the chest..." );
      j = find_free_floor_items_index ( Me . pos . z ) ;
      MoveItem ( & ( chest_level -> ChestItemList [ i ] ) , & ( chest_level -> ItemList [ j ] ) ) ;

      find_position_near_obstacle( &item_x, &item_y, obst_index, chest_level );
      chest_level -> ItemList [ j ] . pos . x = item_x;
      chest_level -> ItemList [ j ] . pos . y = item_y;
      chest_level -> ItemList [ j ] . throw_time = 0.01 ;

	  icnt ++;
    }

  // If the chest was empty, maybe generate a random item to be dropped
  if (!icnt) {
	  find_position_near_obstacle( &item_x, &item_y, obst_index, chest_level );
	  DropRandomItem( Me.pos.z , item_x, item_y, 1 , FALSE );
  }

  //--------------------
  // We play the sound, now that the chest is really opened...
  //
  play_open_chest_sound ( ) ;

}; // void throw_out_all_chest_content ( int obst_index )

/**
 * This is a useful utilitiy subfunction for the checks whether the 
 * mouse cursor is on an enemy, a closed chest or a barrel.
 * The function detects, if the current mouse cursor is over the graphics
 * mentioned in that iso image, using the given position from the 
 * parameter list.
 *
 * TRUE or FALSE is returned, depending on whether the cursor IS or 
 * IS NOT on that particular iso_image, if positioned on that given spot.
 */
int
mouse_cursor_is_on_that_iso_image ( float pos_x , float pos_y , iso_image* our_iso_image )
{
    // our_iso_image = & ( enemy_iso_images [ RotationModel ] [ RotationIndex ] [ (int) this_bot -> animation_phase ] ) ;
    SDL_Rect screen_rectangle ;

    screen_rectangle . x = 
	translate_map_point_to_screen_pixel_x ( pos_x , pos_y ) + 
	our_iso_image -> offset_x ;
    screen_rectangle . y = 
	translate_map_point_to_screen_pixel_y ( pos_x , pos_y ) +
	our_iso_image -> offset_y ;
    screen_rectangle . w = our_iso_image -> original_image_width ;
    screen_rectangle . h = our_iso_image -> original_image_height ;
    
    if ( MouseCursorIsInRect ( & ( screen_rectangle ) , 
			       input_axis.x + User_Rect . w / 2 + User_Rect . x ,
			       input_axis.y + User_Rect . h / 2 + User_Rect . y ) )
    {
	return ( TRUE );
    }      

    return ( FALSE );
};

/**
 * This is a useful utilitiy subfunction for the checks whether the 
 * mouse cursor is on an enemy, a closed chest or a barrel.
 * The function detects, if the current mouse cursor is over the graphics
 * mentioned in that obstacle.
 *
 * TRUE or FALSE is returned, depending on whether the cursor IS or 
 * IS NOT on that particular iso_image, if positioned on that obstacle.
 */
int mouse_cursor_is_on_that_obstacle ( int obst_index ) 
{

	if (mouse_cursor_is_on_that_iso_image ( 
				CURLEVEL() -> obstacle_list [ obst_index ] . pos . x ,
				CURLEVEL() -> obstacle_list [ obst_index ] . pos . y ,
				get_obstacle_image(CURLEVEL()->obstacle_list[obst_index].type))) {
		return ( TRUE ) ;
	}
	return ( FALSE );
}; // int mouse_cursor_is_on_that_obstacle ( int obst_index ) 

/**
 * This function checks if there is a closed chest beneath the current 
 * mouse cursor.  
 * It takes into account the actual size of the barrel/crate
 * graphics and not only the geographic position of the mouse cursor on
 * the floor.
 *
 * If there is a closed chest below the mouse cursor, the function will
 * return the obstacle index of the chest in question.  Else -1 will be
 * returned.
 */
int
closed_chest_below_mouse_cursor ( ) 
{
    int x, y ;
    finepoint MapPositionOfMouse;
    int i;
    int obst_index ;

    //--------------------
    // Now if the cursor is not inside the user rectangle at all, then
    // there can never be a barrel under the mouse cursor...
    //
    if ( ! MouseCursorIsInUserRect ( GetMousePos_x() , 
				     GetMousePos_y() ) )
	return ( -1 ) ;

    //--------------------
    // We find the approximate position of the mouse cursor on the floor.
    // We will use that as the basis for our scanning for barrels under the
    // current mouse cursor.
    //
    MapPositionOfMouse.x = translate_pixel_to_map_location ( 
	(float) input_axis.x , 
	(float) input_axis.y , TRUE ) ;
    MapPositionOfMouse.y = translate_pixel_to_map_location ( 
	(float) input_axis.x , 
	(float) input_axis.y , FALSE ) ;
    
    for ( x = MapPositionOfMouse . x + 3 ; x > MapPositionOfMouse . x - 3 ; x -- )
    {
	for ( y = MapPositionOfMouse . y + 3 ; y > MapPositionOfMouse . y - 3 ; y -- )
	{
	    if ( ( ( (int) x ) < 0 ) ||
		 ( ( (int) y ) < 0 ) ||
		 ( ( (int) x ) >= CURLEVEL() -> xlen ) ||
		 ( ( (int) y ) >= CURLEVEL() -> ylen ) ) continue ;
	    
	    for ( i = 0 ; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; i++ )
	    {
		
		obst_index = CURLEVEL() -> map [ (int) y ] [ (int) x ] . obstacles_glued_to_here [ i ] ;
		
		if ( obst_index == (-1) ) continue;
		
		switch ( CURLEVEL() -> obstacle_list [ obst_index ] . type )
		{
		    case ISO_H_CHEST_CLOSED:
		    case ISO_V_CHEST_CLOSED:
			if ( mouse_cursor_is_on_that_obstacle ( obst_index ) )
			{
			    DebugPrintf ( 1 , "\n%s(): closed chest under cursor identified." , __FUNCTION__ );		    
			    return ( obst_index ) ;
			}
			break;
			
		    default: 
			break;
		}
		
	    }
	}
    }
    
    return ( -1 ) ;

}; // int closed_chest_below_mouse_cursor ( )

/**
 * This function checks if there is a barrel beneath the current mouse
 * cursor.  It takes into account the actual size of the barrel/crate
 * graphics and not only the geographic position of the mouse cursor on
 * the floor.
 *
 * If there is a barrel below the mouse cursor, the function will
 * return the obstacle index of the chest in question.  Else -1 will be
 * returned.
 */
int
smashable_barrel_below_mouse_cursor ( )
{
    int x, y ;
    finepoint MapPositionOfMouse;
    int i;
    int obst_index ;

    //--------------------
    // Now if the cursor is not inside the user rectangle at all, then
    // there can never be a barrel under the mouse cursor...
    //
    if ( ! MouseCursorIsInUserRect( GetMousePos_x()  , 
				    GetMousePos_y()  ))
	return ( -1 ) ;


    //--------------------
    // We find the approximate position of the mouse cursor on the floor.
    // We will use that as the basis for our scanning for barrels under the
    // current mouse cursor.
    //
    MapPositionOfMouse.x = translate_pixel_to_map_location ( 
	(float) input_axis.x , 
	(float) input_axis.y , TRUE ) ;
    MapPositionOfMouse.y = translate_pixel_to_map_location ( 
	(float) input_axis.x , 
	(float) input_axis.y , FALSE ) ;
    
    for ( x = MapPositionOfMouse . x + 3 ; x > MapPositionOfMouse . x - 3 ; x -- )
    {
	for ( y = MapPositionOfMouse . y + 3 ; y > MapPositionOfMouse . y - 3 ; y -- )
	{
	    if ( ( ( (int) x ) < 0 ) ||
		 ( ( (int) y ) < 0 ) ||
		 ( ( (int) x ) >= CURLEVEL() -> xlen ) ||
		 ( ( (int) y ) >= CURLEVEL() -> ylen ) ) continue ;
	    
	    for ( i = 0 ; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; i++ )
	    {
		
		obst_index = CURLEVEL() -> map [ (int) y ] [ (int) x ] . obstacles_glued_to_here [ i ] ;
		
		if ( obst_index == (-1) ) continue;
		
		switch ( CURLEVEL() -> obstacle_list [ obst_index ] . type )
		{
		    case ISO_BARREL_1:
		    case ISO_BARREL_2:
		    case ISO_BARREL_3:
		    case ISO_BARREL_4:
			if ( mouse_cursor_is_on_that_obstacle ( obst_index ) )
			{
			    DebugPrintf ( 1 , "\n%s(): barrel under cursor identified." , __FUNCTION__ );		    
			    return ( obst_index ) ;
			}
			break;
			
		    default: 
			break;
		}
		
	    }
	}
    }
    
    return ( -1 ) ;

}; // int smashable_barrel_below_mouse_cursor ( ) 

/**
 * When the player has requested an attack motion, we start the 
 * corresponding code, that should try to attack, if that's currently
 * possible.
 */
    void
tux_wants_to_attack_now ( int use_mouse_cursor_for_targeting ) 
{

    //--------------------
    // Maybe the player requested an attack before the reload/retract
    // phase is completed.  In that case, we don't attack.
    //
    if ( Me . busy_time > 0 ) 
	{
	return; 
	}

    //--------------------
    // If the Tux has a weapon and this weapon requires some ammunition, then
    // we have to check for enough ammunition first...
    //
    if ( Me . weapon_item . type >= 0 )
	{
	if ( ItemMap [ Me . weapon_item . type ] . item_gun_use_ammunition )
	    {
	    if ( Me . weapon_item . ammo_clip <= 0 )
		{
		No_Ammo_Sound( );

		//--------------------
		// So no ammunition... We should say so and reload...
		//
		append_new_game_message(_("Clip empty, reloading..."));
		TuxReloadWeapon();
		return ;

		}
	    }
	}


    if ( PerformTuxAttackRaw ( use_mouse_cursor_for_targeting ) )
	{ // If attack has failed
	return;
	}

    // The weapon was used and therefore looses some of it's durability
    if ( Me . weapon_item . type >= 0 )
	DamageWeapon ( & ( Me . weapon_item  ) );

    //Weapon uses ammo? remove one bullet !
    if ( Me . weapon_item . type >= 0 )
	{
	if ( ItemMap [ Me . weapon_item . type ] . item_gun_use_ammunition )
	    {
	    Me.weapon_item.ammo_clip --;
	    }
	}

}; // void tux_wants_to_attack_now ( ) 

/**
 * 
 *
 */
void
adapt_position_for_jump_thresholds ( gps* old_position, gps* new_position )
{
    int JumpTarget;
    float JumpThreshold;
    float JumpStartThreshold;
    
    new_position -> x = old_position -> x ;
    new_position -> y = old_position -> y ;
    new_position -> z = old_position -> z ;

    //--------------------
    // First we check for the northern threshold
    //
    JumpThreshold = curShip . AllLevels [ old_position -> z ] -> jump_threshold_north ;
    JumpStartThreshold = JumpThreshold / 2.0 ;
    if ( old_position -> y < JumpStartThreshold )
    {
	JumpTarget = curShip . AllLevels [ old_position -> z ] -> jump_target_north ; 
	if ( JumpTarget > -1 ) 
	{
	    new_position -> x = old_position -> x ;
	    new_position -> y = curShip . AllLevels [ JumpTarget ] -> ylen - JumpStartThreshold ;
	    new_position -> z = JumpTarget ;
	    return;
	}
    }
    
    //--------------------
    // Now we check for the southern threshold
    //
    JumpThreshold = curShip.AllLevels [ old_position -> z ] -> jump_threshold_south ;
    JumpStartThreshold = JumpThreshold / 2.0 ;
    if ( old_position -> y > 
	 curShip.AllLevels [ old_position -> z ] -> ylen - 
	 JumpStartThreshold ) 
    {
	JumpTarget = curShip . AllLevels [ old_position -> z ] -> jump_target_south ; 
	if ( JumpTarget > -1 )
	{
	    new_position -> x = old_position -> x ;
	    new_position -> y = JumpStartThreshold ;
	    new_position -> z = JumpTarget ;
	    return;
	}
    }
    
    //--------------------
    // Now we check for the eastern threshold
    //
    JumpThreshold = curShip.AllLevels [ old_position -> z ] -> jump_threshold_east ;
    JumpStartThreshold = JumpThreshold / 2.0 ;
    if ( old_position -> x > 
	 curShip . AllLevels [ old_position -> z ] -> xlen - 
	 JumpStartThreshold ) 
    {
	JumpTarget = curShip.AllLevels [ old_position -> z ] -> jump_target_east ; 
	if ( JumpTarget > -1 ) 
	{
	    new_position -> x = JumpStartThreshold ;
	    new_position -> y = old_position -> y ;
	    new_position -> z = JumpTarget ;
	    return;
	}
    }
    
    //--------------------
    // Now we check for the western threshold
    //
    JumpThreshold = curShip.AllLevels [ old_position -> z ] -> jump_threshold_west ;
    JumpStartThreshold = JumpThreshold / 2.0 ;
    
    if ( old_position -> x < JumpStartThreshold )
    {
	JumpTarget = curShip.AllLevels [ old_position -> z ] -> jump_target_west ; 
	if ( JumpTarget > -1 ) 
	{
	    new_position -> x = curShip.AllLevels [ JumpTarget ] -> xlen - JumpStartThreshold ;
	    new_position -> y = old_position -> y ;
	    new_position -> z = JumpTarget ;
	    return;
	}
    }


}; // void adapt_position_for_jump_thresholds ( gps* old_position, gps* new_position )

/**
 * The Tux might be close to the borders of a level, so close in fact, 
 * that he has crossed the internal threshold area.  In that case, we 
 * must move the Tux silently to the corresponding other level to the
 * corresponding position inside the threshold area there.  This is what
 * this function is for.
 */
	void
correct_tux_position_according_to_jump_thresholds ( )
{
	int JumpTarget;
	float JumpThreshold;
	float SafetyBonus = 0.0 ;
	float JumpStartThreshold;
	gps old_mouse_move_target ;
	int oldz = Me.pos.z;

	old_mouse_move_target . x = Me . mouse_move_target . x ;
	old_mouse_move_target . y = Me . mouse_move_target . y ;
	old_mouse_move_target . z = Me . mouse_move_target . z ;

	//--------------------
	// First we check for the northern threshold
	//
	JumpThreshold = curShip.AllLevels [ Me . pos . z ] -> jump_threshold_north ;
	JumpStartThreshold = JumpThreshold / 2.0 ;

	if ( Me . pos . y < JumpStartThreshold )
		{

		JumpTarget = curShip.AllLevels [ Me . pos . z ] -> jump_target_north ; 

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJUMP TO THE NORTH CONSIDERED!!" );
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJumpStartThreshold was: %f. " , JumpStartThreshold ); 

		if ( JumpTarget <= -1 ) return;

		Teleport ( JumpTarget , 
				Me . pos . x ,
				curShip.AllLevels [ JumpTarget ] -> ylen - 0 - JumpStartThreshold - SafetyBonus ,
				FALSE ) ; 

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nReally doing it now.  Need to check the running target still..." );

		//--------------------
		// We try to translate the mouse move target to the new levels
		// coordinates (if it was a SIMPLE mouse move, NOT if it was a
		// combo-action).
		//
		if ( ( Me . mouse_move_target_combo_action_type == NO_COMBO_ACTION_SET ) &&
				( enemy_resolve_address(Me . current_enemy_target_n, &Me.current_enemy_target_addr) == NULL ) )
			{
			Me . mouse_move_target . z = Me . pos . z ;
			Me . mouse_move_target . x = old_mouse_move_target . x ;

			//--------------------
			// The offset how far into the new levels map we must go is this:
			//
			Me . mouse_move_target . y = - ( JumpThreshold - old_mouse_move_target . y ) ;
			//--------------------
			// The correction because we're coming from south now it this:
			Me . mouse_move_target . y += curShip . AllLevels [ Me . pos . z ] -> ylen ;

			DebugPrintf (  LEVEL_JUMP_DEBUG , "\nJUMP TO THE NORTH:  target translated:  y=%f!" , Me . mouse_move_target . y );
			if ( !SinglePointColldet ( Me . mouse_move_target . x , Me . mouse_move_target . y , Me . mouse_move_target . z, NULL ) )
				{
				Me . mouse_move_target . x = ( -1 ) ;
				Me . mouse_move_target . y = ( -1 ) ;
				Me . mouse_move_target . z = ( -1 ) ;
				}

			}

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nOk.  Jump done.  returning..." );

		return;

		}

	//--------------------
	// Now we check for the southern threshold
	//
	JumpThreshold = curShip.AllLevels [ Me . pos . z ] -> jump_threshold_south ;
	JumpStartThreshold = JumpThreshold / 2.0 ;

	if ( Me . pos . y > 
			curShip.AllLevels [ Me . pos . z ] -> ylen - 
			JumpStartThreshold ) 
		{

		JumpTarget = curShip.AllLevels [ Me . pos . z ] -> jump_target_south ; 

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJUMP TO THE SOUTH CONSIDERED!!" );
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJumpStartThreshold was: %f. " , JumpStartThreshold ); 
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nCurrent Level Y-len: %d. " , curShip.AllLevels [ Me . pos . z ] -> ylen );
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nCurrent Y-pos: %f. " , Me . pos . y );

		if ( JumpTarget <= -1 ) return;

		Teleport ( JumpTarget , 
				Me . pos . x ,
				0 + JumpStartThreshold + SafetyBonus ,
				FALSE ) ; 


		//--------------------
		// We try to translate the mouse move target to the new levels
		// coordinates (if it was a SIMPLE mouse move, NOT if it was a
		// combo-action).
		//
		if ( ( Me . mouse_move_target_combo_action_type == NO_COMBO_ACTION_SET ) &&
				( enemy_resolve_address(Me . current_enemy_target_n, &Me.current_enemy_target_addr) == NULL ) )
			{
			Me . mouse_move_target . z = Me . pos . z ;
			Me . mouse_move_target . x = old_mouse_move_target . x ;

			//--------------------
			// The offset how far into the new levels map we must go is this:
			//
			Me . mouse_move_target . y = 
				- ( curShip.AllLevels[oldz]->ylen - JumpThreshold )
				+ old_mouse_move_target . y ;
			//--------------------
			// The correction because we're coming from north is nothing:
			Me . mouse_move_target . y += 0 ;

			DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJUMP TO THE SOUTH:  target translated to y=%f!" ,  Me . mouse_move_target . y );
			if ( ! SinglePointColldet ( Me . mouse_move_target . x , Me . mouse_move_target . y , Me . mouse_move_target . z, NULL ) )
				{
				Me . mouse_move_target . x = ( -1 ) ;
				Me . mouse_move_target . y = ( -1 ) ;
				Me . mouse_move_target . z = ( -1 ) ;
				}

			}

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nOk.  Jump done.  returning..." );

		return;
		}

	//--------------------
	// Now we check for the eastern threshold
	//
	JumpThreshold = curShip.AllLevels [ Me . pos . z ] -> jump_threshold_east ;
	JumpStartThreshold = JumpThreshold / 2.0 ;

	if ( Me . pos . x > 
			curShip.AllLevels [ Me . pos . z ] -> xlen - 
			JumpStartThreshold ) 
		{

		JumpTarget = curShip.AllLevels [ Me . pos . z ] -> jump_target_east ; 

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJUMP TO THE EAST CONSIDERED!!" );
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJumpStartThreshold was: %f. " , JumpStartThreshold ); 
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nCurrent Level X-len: %d. " , curShip.AllLevels [ Me . pos . z ] -> xlen );
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nCurrent X-pos: %f. " , Me . pos . x );

		if ( JumpTarget <= -1 ) return;

		Teleport ( JumpTarget , 
				0 + JumpStartThreshold + SafetyBonus ,
				Me . pos . y ,
				FALSE ) ; 
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nTeleported to level %d, xlen %d, at x %f\n", JumpTarget, curShip.AllLevels[JumpTarget]->xlen,  (float)(0 + JumpStartThreshold + SafetyBonus) );

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nReally doing it now.  Need to check the running target still..." );

		//--------------------
		// We try to translate the mouse move target to the new levels
		// coordinates (if it was a SIMPLE mouse move, NOT if it was a
		// combo-action).
		//
		if ( ( Me . mouse_move_target_combo_action_type == NO_COMBO_ACTION_SET ) &&
				( enemy_resolve_address ( Me.current_enemy_target_n, &Me.current_enemy_target_addr) == NULL ) )
			{
			Me . mouse_move_target . z = Me . pos . z ;
			Me . mouse_move_target . y = old_mouse_move_target . y ;

			//--------------------
			// The offset how far into the new levels map we must go is this:
			//
			Me . mouse_move_target . x = 
				- ( curShip.AllLevels[oldz]->xlen - JumpThreshold )
				+ old_mouse_move_target . x ;

			DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJUMP TO THE EAST:  target translated to x=%f!" ,  Me . mouse_move_target . x );
			if ( ! SinglePointColldet ( Me . mouse_move_target . x , Me . mouse_move_target . y , Me . mouse_move_target . z, NULL ) )
				{
				Me . mouse_move_target . x = ( -1 ) ;
				Me . mouse_move_target . y = ( -1 ) ;
				Me . mouse_move_target . z = ( -1 ) ;
				}

			}

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nOk.  Jump done.  returning..." );

		return;
		}

	//--------------------
	// Now we check for the western threshold
	//
	JumpThreshold = curShip.AllLevels [ Me . pos . z ] -> jump_threshold_west ;
	JumpStartThreshold = JumpThreshold / 2.0 ;

	if ( Me . pos . x < JumpStartThreshold )
		{

		JumpTarget = curShip.AllLevels [ Me . pos . z ] -> jump_target_west ; 

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJUMP TO THE WEST CONSIDERED!!" );
		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nJumpStartThreshold was: %f. " , JumpStartThreshold ); 

		if ( JumpTarget <= -1 ) return;

		Teleport ( JumpTarget , 
				curShip.AllLevels [ JumpTarget ] -> xlen - 0 - JumpStartThreshold - SafetyBonus ,
				Me . pos . y ,
				FALSE ) ; 

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nTeleported to level %d, xlen %d, at x %d\n", JumpTarget, curShip.AllLevels[JumpTarget]->xlen,  curShip.AllLevels [ JumpTarget ] -> xlen - 0 - JumpStartThreshold - SafetyBonus);

		//--------------------
		// We try to translate the mouse move target to the new levels
		// coordinates (if it was a SIMPLE mouse move, NOT if it was a
		// combo-action).
		//
		if ( ( Me . mouse_move_target_combo_action_type == NO_COMBO_ACTION_SET ) &&
				( enemy_resolve_address ( Me.current_enemy_target_n, &Me.current_enemy_target_addr) == NULL ) )
			{
			Me . mouse_move_target . z = Me . pos . z ;
			Me . mouse_move_target . y = old_mouse_move_target . y ;

			//--------------------
			// The offset how far into the new levels map we must go is this:
			//
			Me . mouse_move_target . x = - ( JumpThreshold - old_mouse_move_target . x ) ;
			//--------------------
			// The correction because we're coming from south now it this:
			Me . mouse_move_target . x += curShip . AllLevels [ Me . pos . z ] -> xlen ;

			DebugPrintf (  LEVEL_JUMP_DEBUG , "\nJUMP TO THE NORTH:  target translated:  x=%f!" , Me . mouse_move_target . x );
			if ( ! SinglePointColldet ( Me . mouse_move_target . x , Me . mouse_move_target . y , Me . mouse_move_target . z, NULL ) )
				{
				Me . mouse_move_target . x = ( -1 ) ;
				Me . mouse_move_target . y = ( -1 ) ;
				Me . mouse_move_target . z = ( -1 ) ;
				}

			}

		DebugPrintf ( LEVEL_JUMP_DEBUG , "\nOk.  Jump done.  returning..." );

		return;

		}

}; // void CheckForJumpThresholds ( )

/**
 * This function initializes the influencers position history, which is
 * a ring buffer and is needed for throwing the influencer back (only one
 * or two positions would be needed for that) and for influencers followers
 * to be able to track the influencers path (10000 or so positions are used
 * for that, and that's why it is a ring buffer).
 */
void 
InitInfluPositionHistory( )
{
  int RingPosition;

  for ( RingPosition = 0 ; RingPosition < MAX_INFLU_POSITION_HISTORY ; RingPosition ++ )
    {
      Me . Position_History_Ring_Buffer [ RingPosition ] . x = Me . pos . x ;
      Me . Position_History_Ring_Buffer [ RingPosition ] . y = Me . pos . y ;
      Me . Position_History_Ring_Buffer [ RingPosition ] . z = Me . pos . z ;
    }
} // void InitInfluPositionHistory( void )

float 
GetInfluPositionHistoryX( int HowLongPast )
{
  int RingPosition;

  HowLongPast >>= 1;

  RingPosition = Me . current_zero_ring_index - HowLongPast;

  RingPosition += MAX_INFLU_POSITION_HISTORY; // We don't want any negative values, for safety

  RingPosition %= MAX_INFLU_POSITION_HISTORY; // We do MODULO for the Ring buffer length 

  return Me.Position_History_Ring_Buffer[ RingPosition ].x;
}

float 
GetInfluPositionHistoryY( int HowLongPast )
{
  int RingPosition;

  HowLongPast >>= 1;
  RingPosition = Me . current_zero_ring_index - HowLongPast;

  RingPosition += MAX_INFLU_POSITION_HISTORY; // We don't want any negative values, for safety

  RingPosition %= MAX_INFLU_POSITION_HISTORY; // We do MODULO for the Ring buffer length 

  return Me.Position_History_Ring_Buffer[ RingPosition ].y;
}

float 
GetInfluPositionHistoryZ( int HowLongPast )
{
  int RingPosition;

  HowLongPast >>= 1;
  RingPosition = Me . current_zero_ring_index - HowLongPast;

  RingPosition += MAX_INFLU_POSITION_HISTORY; // We don't want any negative values, for safety

  RingPosition %= MAX_INFLU_POSITION_HISTORY; // We do MODULO for the Ring buffer length 

  return Me.Position_History_Ring_Buffer[ RingPosition ].z;
}

/**
 * This function should check if the Tux is still ok, i.e. if he is still
 * alive or if the death sequence should be initiated.
 */
void 
CheckIfCharacterIsStillOk ( ) 
{
    if ( Me . AllMissions [ GetMissionIndexByName("Propagating a faulty firmware") ] . MissionIsComplete )
	{
	ThouHastWon ();
	}
    
    if ( Me . god_mode )
	Me . energy = Me . maxenergy;

    //------------------------------
    // Now we check if the main character is really still ok.
    //
    if ( Me . energy <= 0 )
    {
	ThouArtDefeated ();

	DebugPrintf ( 1 , "\n%s():  Alternate end of function reached." , __FUNCTION__ );
	return;
    }

	    
}; // void CheckIfCharacterIsStillOk ( )

/**
 * Even the Tux must not leave the map!  A sanity check is done here...
 */
void
CheckForTuxOutOfMap ( )
{
  Level MoveLevel = curShip.AllLevels[ Me . pos . z ] ;

  //--------------------
  // Now perhaps the influencer is out of bounds, i.e. outside of the map.
  //
  if ( ( (int) rintf( Me . pos.y ) >= MoveLevel->ylen ) ||
       ( (int) rintf( Me . pos.x ) >= MoveLevel->xlen ) ||
       ( (int) rintf( Me . pos.y ) <  0              ) ||
       ( (int) rintf( Me . pos.x ) <  0              ) )
    {
      fprintf ( stderr, "\n\nplayer's last position: X=%f, Y=%f, Z=%d.\n" , 
		Me . pos . x ,
		Me . pos . y ,
		Me . pos . z );
      ErrorMessage ( __FUNCTION__  , "\
A player's Tux was found outside the map.\n\
This indicates either a bug in the Freedroid RPG code or\n\
a bug in the currently used map system of Freedroid RPG.",
				 PLEASE_INFORM, IS_FATAL );
    }
}; // void CheckForTuxOutOfMap ( )

/**
 * If an enemy was specified as the mouse move target, this enemy will
 * maybe move here and there.  But this means that also the mouse move
 * target of the influencer must adapt, which is done in this function.
 */
void
tux_get_move_target_and_attack ( moderately_finepoint * movetgt )
{
    moderately_finepoint RemainingWay;
    float RemainingWayLength;
    
    // if there is a mouse move target, we are not going to move towards the enemy
    if ( Me . mouse_move_target . x != (-1) ) 
	{
	movetgt->x = Me . mouse_move_target . x;
	movetgt->y = Me . mouse_move_target . y;
	return;
	}
 
    enemy * t = enemy_resolve_address(Me . current_enemy_target_n, & Me.current_enemy_target_addr);

    if (! t || ( t -> energy <= 0 )) //No enemy or dead enemy, remove enemy
	{
	enemy_set_reference( & Me.current_enemy_target_n, & Me . current_enemy_target_addr, NULL);
	movetgt->x = -1;
	movetgt->y = -1;
	return;
	}	


    update_virtual_position(&t->virt_pos, &t->pos, Me.pos.z);

    //--------------------
    // If we have a ranged weapon in hand, there is no need to approach the
    // enemy in question.  We just try to fire a shot.
    //
    if ( Me . weapon_item . type != (-1) )
    {
	if ( ! ItemMap [ Me . weapon_item . type ] . item_weapon_is_melee )
	{ //ranged weapon
	    if ( ! t -> is_friendly )
		tux_wants_to_attack_now ( FALSE ) ;
	    movetgt->x = -1;
	    movetgt->y = -1;
	    return;
	}
    }


    RemainingWay . x = Me . pos . x - t -> virt_pos . x ;
    RemainingWay . y = Me . pos . y - t -> virt_pos . y ;
    
    RemainingWayLength = sqrtf ( ( RemainingWay . x ) * ( RemainingWay . x ) +
				 ( RemainingWay . y ) * ( RemainingWay . y ) ) ;
    
    if(RemainingWayLength > 0.05)
	{
	    RemainingWay . x = ( RemainingWay . x / RemainingWayLength ) * 
		( RemainingWayLength - ( BEST_MELEE_DISTANCE - 0.1 ) ) ;
	    RemainingWay . y = ( RemainingWay . y / RemainingWayLength ) * 
		( RemainingWayLength - ( BEST_MELEE_DISTANCE - 0.1 ) ) ;
	}
    
    if ( ( RemainingWayLength <= BEST_MELEE_DISTANCE * sqrt(2) + 0.01 ) && ( ! t->is_friendly ) )
    {
	tux_wants_to_attack_now ( FALSE ) ;
    } 
    
    movetgt -> x = Me . pos . x - RemainingWay . x ;
    movetgt -> y = Me . pos . y - RemainingWay . y ;
    return;
    
    
}; // void UpdateMouseMoveTargetAccoringToEnemy ( )

/**
 * Self-explanatory.
 * This function also takes into account any conveyor belts the Tux might
 * be standing on.
 */
void
MoveTuxAccordingToHisSpeed ( )
{
  float planned_step_x;
  float planned_step_y;

  //--------------------
  // Now we move influence according to current speed.  But there has been a problem
  // reported from people, that the influencer would (*very* rarely) jump throught walls
  // and even out of the ship.  This has *never* occured on my fast machine.  Therefore
  // I assume that the problem is related to sometimes very low framerates on these machines.
  // So, we do a sanity check not to make steps too big.
  //
  // And on machines with FPS << 20, it will certainly alter the game behaviour, so people
  // should really start using a pentium or better machine.
  //
  planned_step_x = Me . speed . x * Frame_Time ();
  planned_step_y = Me . speed . y * Frame_Time ();

  //--------------------
  // Maybe the Tux is just executing a weapon strike.  In this case, there should
  // be no movement at all, so in this case we'll just not go anywhere...
  //
/*XXX  if ( Me . weapon_swing_time > 0 )
    {
      planned_step_x = 0 ;
      planned_step_y = 0 ;
    }*/

  Me . pos . x += planned_step_x;
  Me . pos . y += planned_step_y;

  //--------------------
  // If the Tux got stuck, i.e. if he got no speed at all and still is 
  // currently not in a 'passable' position, the fallback handling needs
  // to be applied to move the Tux out of the offending obstacle (i.e. 
  // simply away from the offending obstacles center)
  //
  if ( ( fabsf ( Me . speed . x ) < 0.1 ) &&
       ( fabsf ( Me . speed . y ) < 0.1 ) )
    {
      //--------------------
      // So there is no speed, so we check for passability...
      //
		if ( ! SinglePointColldet( Me . pos . x , Me . pos . y , Me . pos . z, &WalkablePassFilter ) )
	{
	  //--------------------
	  // Now it's time to launch the stuck-fallback handling...
	  //
			DebugPrintf ( -3 , "\nTux looks stuck...ESCAPING just for this frame..." );
			float new_x = Me.pos.x;
			float new_y = Me.pos.y;
			int rtn = EscapeFromObstacle( &new_x, &new_y, Me.pos.z, &WalkablePassFilter);
			if ( !rtn ) 
			{
				DebugPrintf( -3, "\nNo escape position found around Tux... Looking in position history...");
				// First : look for a suitable position in Tux's position_history
				int i;
				float old_x;
				float old_y;
				for (i=1; i<10; i++) 
				{
					if ( GetInfluPositionHistoryZ(10*i) == Me.pos.z )
					{
						old_x = GetInfluPositionHistoryX(10*i);
						old_y = GetInfluPositionHistoryY(10*i);
						if ( old_x != Me.pos.x && old_y != Me.pos.y && SinglePointColldet(old_x, old_y, Me.pos.z, &WalkablePassFilter) )
						{
							// Found...
							Me.pos.x = old_x;
							Me.pos.y = old_y;
							break;
						}
					}
				}
				// If no luck, last fallback
				if ( i == 10 )
				{
					DebugPrintf( -3, "\nNo luck with position_history, last fallback...");
					// Get a random direction, and move by random length from 0.5 to 1.5.
					// With some luck, Tux will escape now, or in the future tries
					Me.pos.x += (0.5 + (float)MyRandom(10)/10.0) * (MyRandom(10)<5)?(1):(-1);
					Me.pos.y += (0.5 + (float)MyRandom(10)/10.0) * (MyRandom(10)<5)?(1):(-1);
				}
			}
			else 
		{
				Me.pos.x = new_x;
				Me.pos.y = new_y;
		    }
		}
	    }

  //--------------------
  // Even the Tux must not leave the map!  A sanity check is done
  // here...
  //
  CheckForTuxOutOfMap ( );

}; // void MoveTuxAccordingToHisSpeed ( )

/**
 * This function contains the final dumb movement code, that, without
 * any checks and any refinement, just moves the tux thowards the given
 * target position.
 *
 * The return value indicates, if the target has been sufficiently 
 * approximated (TRUE) already or not (FALSE) .
 *
 */
int move_tux_thowards_raw_position ( float x , float y )
{
    moderately_finepoint RemainingWay;
    moderately_finepoint planned_step;
    float length;

    if ( Me . energy <= 0 ) return ( FALSE ) ;

    RemainingWay . x = - Me . pos . x + x ;
    RemainingWay . y = - Me . pos . y + y ;

    length = vect_len ( RemainingWay );

    //--------------------
    // Maybe the remaining way is VERY!! small!  Then we must not do
    // a division at all.  We also need not do any movement, so the
    // speed can be eliminated and we're done here.
    //
    if ( length < 0.05 )
	{
	Me . speed . x = 0 ;
	Me . speed . y = 0 ;
	return ( TRUE ) ;
	}

    //--------------------
    // Now depending on whether the running key is pressed or not,
    // we have the Tux go on running speed or on walking speed.
    //
    if ( Me . running_power <= 0 ) {
	Me . running_must_rest = TRUE ;
	}

    if (Me . running_must_rest)
	{
	autorun_activated = 0;
	planned_step . x = RemainingWay . x * TUX_WALKING_SPEED / length ;
	planned_step . y = RemainingWay . y * TUX_WALKING_SPEED / length ;
	//DebugPrintf( -2, "\n Now walking...");
	}

    if ( (LeftCtrlPressed() || autorun_activated ) && !( LeftCtrlPressed() && autorun_activated ) && ( ! Me . running_must_rest ) )
	{ 
	planned_step . x = RemainingWay . x * TUX_RUNNING_SPEED / length ;
	planned_step . y = RemainingWay . y * TUX_RUNNING_SPEED / length ;
	// DebugPrintf ( -2 , "\nNow running..." );
	}
    else
	{
	planned_step . x = RemainingWay . x * TUX_WALKING_SPEED / length ;
	planned_step . y = RemainingWay . y * TUX_WALKING_SPEED / length ;
	// DebugPrintf ( -2 , "\nNow walking..." );
	}

    //--------------------
    // Now that the speed is set, we can start to make the step
    //
    Me . speed . x = planned_step . x ;
    Me . speed . y = planned_step . y ;

    // --------------------
    // If we might step over the target,
    // we reduce the speed.
    //
    if ( fabsf ( planned_step . x * Frame_Time() ) >= fabsf ( RemainingWay .x  ) )
	Me . speed . x = RemainingWay . x / Frame_Time() ;
    if ( fabsf ( planned_step . y * Frame_Time() ) >= fabsf ( RemainingWay .y  ) )
	Me . speed . y = RemainingWay . y / Frame_Time() ;



    //--------------------
    // In case we have reached our target, we can remove this mouse_move_target again,
    // but also if we have been thrown onto a different level, we cancel our current
    // mouse move target...
    //
    if ( ( ( fabsf ( RemainingWay.y ) <= DISTANCE_TOLERANCE ) && 
		( fabsf ( RemainingWay.x ) <= DISTANCE_TOLERANCE )     ) ||
	    ( Me . mouse_move_target . z != Me . pos . z ) )
	{
	return ( TRUE );
	}

    return ( FALSE );
}; // int move_tux_thowards_raw_position ( float x , float y )

/**
 *
 *
 */
void
move_tux_thowards_intermediate_point ( )
{
    int i;
    Level PlayerLevel;

    PlayerLevel = curShip . AllLevels [ Me . pos. z ] ;
    
    //--------------------
    // If there is no intermediate course, we don't need to do anything
    // in this function.
    //
    if ( Me . next_intermediate_point [ 0 ] . x == (-1) )
    {
	Me . mouse_move_target . x = -1;
	Me . mouse_move_target . y = -1;

	//--------------------
	// The fact that there is no more intermediate course can mean, that
	// there never has been any intermediate course or we have now arrived
	// at the end of the previous intermediate course.
	//
	// But that maybe means, that it is now time for the combo_action, that
	// can occur on the end of any intermediate course, like e.g. open a
	// chest or pick up some item.
	//
	// DebugPrintf ( 2 , "\nAm I now at the last intermediate point???" );
	switch ( Me . mouse_move_target_combo_action_type )
	{
	    case NO_COMBO_ACTION_SET:
		break;
	    case COMBO_ACTION_OPEN_CHEST:
		check_for_chests_to_open ( Me . mouse_move_target_combo_action_parameter ) ;
		break;
	    case COMBO_ACTION_SMASH_BARREL:
		check_for_barrels_to_smash ( Me . mouse_move_target_combo_action_parameter ) ;
		break;
	    case COMBO_ACTION_PICK_UP_ITEM:
		check_for_items_to_pickup ( Me . mouse_move_target_combo_action_parameter ); 
		break;
	    default:
		ErrorMessage ( __FUNCTION__  , 
					   "Unhandled combo action for intermediate course encountered!" ,
					   PLEASE_INFORM, IS_FATAL );
		break;
	}

	Me . mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;
	return;
    }
    
    //--------------------
    // Now we move the Tux thowards the next intermediate course point
    //
    if ( move_tux_thowards_raw_position ( Me . next_intermediate_point [ 0 ] . x , 
					  Me . next_intermediate_point [ 0 ] . y ) )
	{	
	DebugPrintf ( DEBUG_TUX_PATHFINDING , "\nMOVING ON TO NEXT INTERMEDIATE WAYPOINT! " );
	for ( i = 1 ; i < MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX ; i ++ )
	    {
	    Me . next_intermediate_point [ i-1 ] . x = 
		Me . next_intermediate_point [ i ] . x ;
	    Me . next_intermediate_point [ i-1 ] . y = 
		Me . next_intermediate_point [ i ] . y ;
	    }
	}
    
}; // void move_tux_thowards_intermediate_point ( )

/** 
 * When the player has rolled the mouse wheel up or down, we change the
 * global mode of the game, i.e. switch to examine mode or loot mode or
 * the like.  The rolling of game mode can also be controlled with cursor
 * keys left and right.
 * After the mode is applied, it will be reset automatically to 'normal'
 * mode again.  The mode switching from mouse wheel/cursor action is done
 * in here.  Some global modes are currently not reachable via mouse
 * wheel action, simply because there implementation is far from finished
 * and we can't do everything at once and maybe not within one release.
 */
void
adapt_global_mode_for_player ( )
{
    static int right_pressed_previous_frame = FALSE ;
    static int left_pressed_previous_frame = FALSE ;

    //--------------------
    // At first we check if maybe the player is scrolling the game
    // message window.
    //
    SDL_Rect upper_rect = { ( 98 * GameConfig . screen_width ) / 640 , GameConfig . screen_height - (102 * GameConfig . screen_height) / 480 , 444 * GameConfig . screen_width / 640 , (48/2)*GameConfig.screen_height / 480 } ;
    SDL_Rect lower_rect = { ( 98 * GameConfig . screen_width ) / 640 , GameConfig . screen_height - (102 * GameConfig . screen_height) / 480 + (48/2)*GameConfig.screen_height/480, 444 * GameConfig . screen_width / 640 , (48/2)*GameConfig.screen_height / 480 } ;

    if ( GameConfig.screen_height == 480 && ( GameConfig . Inventory_Visible || GameConfig . CharacterScreen_Visible || GameConfig . SkillScreen_Visible || GameConfig . skill_explanation_screen_visible ))
	    goto done_handling_scroll_updown;

    if ( MouseCursorIsInRect ( &upper_rect , GetMousePos_x() , GetMousePos_y() ) )
	{
	    if ( global_ingame_mode != GLOBAL_INGAME_MODE_IDENTIFY )
		    global_ingame_mode = GLOBAL_INGAME_MODE_SCROLL_UP ;
	    return;
	}
    if ( MouseCursorIsInRect ( &lower_rect , GetMousePos_x() , GetMousePos_y() ) )
	{
	    if ( global_ingame_mode != GLOBAL_INGAME_MODE_IDENTIFY )
		    global_ingame_mode = GLOBAL_INGAME_MODE_SCROLL_DOWN ;
	    return;
	}

done_handling_scroll_updown:
    if ( ( global_ingame_mode == GLOBAL_INGAME_MODE_SCROLL_UP ) ||
	 ( global_ingame_mode == GLOBAL_INGAME_MODE_SCROLL_DOWN ) )
    {
	global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    }

    if ( ( RightPressed ( ) && ( ! right_pressed_previous_frame) ) || MouseWheelDownPressed ( ) )
    {
	if ( global_ingame_mode == GLOBAL_INGAME_MODE_NORMAL )	
	    global_ingame_mode = GLOBAL_INGAME_MODE_EXAMINE ;
	else if ( global_ingame_mode == GLOBAL_INGAME_MODE_EXAMINE )	
	    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    }


    if ( ( LeftPressed ( ) && ( ! left_pressed_previous_frame ) ) || MouseWheelUpPressed ( ) )
    {
	if ( global_ingame_mode == GLOBAL_INGAME_MODE_NORMAL )	
	    global_ingame_mode = GLOBAL_INGAME_MODE_EXAMINE ;
	else
	    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;
    }

    right_pressed_previous_frame =  RightPressed ( ) ;
    left_pressed_previous_frame =  LeftPressed ( ) ;

}; // void adapt_global_mode_for_player ( )

/**
 * This function moves the influencer, adjusts his speed according to
 * keys pressed and also adjusts his status and current "phase" of his 
 * rotation.
 */
void
move_tux ( )
{
	Level MoveLevel = curShip.AllLevels[ Me . pos . z ] ;
	static moderately_finepoint last_given_course_target = { -2 , -2 };

	// check, if the influencer is still ok
	CheckIfCharacterIsStillOk ( ) ;

	//--------------------
	// We store the influencers position for the history record and so that others
	// can follow his trail.
	//
	static int i = 0;
	i++;
	if ( i & 1 )
	{
		Me . current_zero_ring_index++;
		Me . current_zero_ring_index %= MAX_INFLU_POSITION_HISTORY;
		Me . Position_History_Ring_Buffer [ Me . current_zero_ring_index ] . x = Me .pos.x;
		Me . Position_History_Ring_Buffer [ Me . current_zero_ring_index ] . y = Me .pos.y;
		Me . Position_History_Ring_Buffer [ Me . current_zero_ring_index ] . z = MoveLevel->levelnum ;
	}

	if  ( Me . paralyze_duration ) 
	{
		Me . speed . x = 0;
		Me . speed . y = 0;
		return;  //If tux is paralyzed, we do nothing more
	}

	//--------------------
	// As a preparation for the later operations, we see if there is
	// a living droid set as a target, and if yes, we correct the move
	// target to something suiting that new droids position.
	//
	//
	//

	moderately_finepoint move_target;

	tux_get_move_target_and_attack(&move_target);

	if ( move_target . x != -1 )
	{
		//--------------------
		// For optimisation purposes, we'll not do anything unless a new target
		// has been given.
		//
		if (!( ( fabsf ( move_target . x - last_given_course_target . x ) < 0.3 ) &&
		       ( fabsf ( move_target . y - last_given_course_target . y ) < 0.3 ) ))
		{
			freeway_context frw_ctx = { FALSE, { NULL, NULL } };
			pathfinder_context pf_ctx = { &WalkablePassFilter, &frw_ctx };
			set_up_intermediate_course_between_positions ( &Me.pos, &move_target, &Me.next_intermediate_point[0], MAX_INTERMEDIATE_WAYPOINTS_FOR_TUX, &pf_ctx ) ;
			last_given_course_target . x = move_target . x ;
			last_given_course_target . y = move_target . y ;
		}
	}

	// Perhaps the player has turned the mouse wheel.  In that case we might
	// need to change the current global mode, depending on whether a change
	// of global mode (with the current obstacles under the mouse cursor)
	// makes sense or not.
	// 
	adapt_global_mode_for_player ( );

	//--------------------
	// But in case of some mouse move target present, we proceed to move
	// thowards this mouse move target.
	//
	move_tux_thowards_intermediate_point ( );

	//--------------------
	// Perhaps the player has pressed the right mouse button, indicating the use
	// of the currently selected special function or spell.
	//
	HandleCurrentlyActivatedSkill( );
   
	// --------------------
	// Maybe we need to fire a bullet or set a new mouse move target
	// for the new move-to location
	//
	AnalyzePlayersMouseClick ( );
	
	if ( MouseLeftPressed () )
		no_left_button_press_in_previous_analyze_mouse_click = FALSE ;
	else
		no_left_button_press_in_previous_analyze_mouse_click = TRUE ;
	
	//--------------------
	// During inventory operations, there should not be any (new) movement
	//
	if ( Item_Held_In_Hand != (-1) )
	{
		Me . mouse_move_target . x = Me . pos . x ;
		Me . mouse_move_target . y = Me . pos . y ;
		Me . mouse_move_target . z = Me . pos . z ;
		enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
		return; 
	}
	
	//--------------------
	// The influ should lose some of his speed when no key is pressed and
	// also no mouse move target is set.
	//
	tux_friction_with_air ( ) ;
	
	limit_tux_speed_to_a_maximum ( ) ;
	
	MoveTuxAccordingToHisSpeed ( );
	
	animate_tux ( ) ;	// move the "phase" of influencers rotation
}; // void move_tux( );

/**
 * This function does the 'rotation' of the influencer, according to the
 * current energy level of the influencer.  If his energy is low, the
 * rotation will also go slow, if his energy is high, rotation will go
 * fast. 
 */
void
animate_tux ( )
{
  float my_speed;
#define TOTAL_SWING_TIME 0.55
#define FULL_BREATHE_TIME 3
#define TOTAL_STUNNED_TIME 0.35
#define STEP_TIME (0.28)
  static float step_countdown = 0;

  //--------------------
  // First we handle the case of just getting hit...
  //
  if ( Me . got_hit_time != (-1) )
    {
	Me . phase = TUX_SWING_PHASES + TUX_BREATHE_PHASES;
	if ( Me . got_hit_time > TOTAL_STUNNED_TIME ) Me .got_hit_time = (-1) ;
	Me . walk_cycle_phase = 17 ;
    }

  //--------------------
  // Now we handle the case of nothing going on and the Tux just standing around...
  // or moving to some place.
  //
  else if ( Me . weapon_swing_time == (-1) )
    {
      Me . phase = ( (int) ( Me . MissionTimeElapsed * TUX_BREATHE_PHASES / FULL_BREATHE_TIME ) ) % TUX_BREATHE_PHASES ;

      if ( fabsf ( Me . speed . x ) + fabsf ( Me . speed . y ) < 0.3 )
	Me . walk_cycle_phase = 17 ;
      else
	{
	  my_speed = sqrt ( Me . speed . x * Me . speed . x +
			    Me . speed . y * Me . speed . y ) ;
	  if ( my_speed <= ( TUX_WALKING_SPEED + TUX_RUNNING_SPEED ) * 0.5 )
	    Me . walk_cycle_phase += Frame_Time() * 10.0 * my_speed ;
	  else
	    {
	      Me . walk_cycle_phase += Frame_Time() * 3.0 * my_speed ;

	      step_countdown += Frame_Time() ;
	      if ( step_countdown > STEP_TIME )
		{
		  play_sample_using_WAV_cache_v( "effects/tux_footstep.ogg" , FALSE , FALSE,0.2 );
		  step_countdown -= STEP_TIME ;
		  // Me . running_power -= STEP_TIME * 2.0 ;
		}
	    }

	  if ( Me . walk_cycle_phase > 25.0 ) Me . walk_cycle_phase = 15.0 ;
	}

    }

  //--------------------
  // Now we handle the case of a weapon swing just going on...
  //
  else
    {
      Me . phase = ( TUX_BREATHE_PHASES + ( Me . weapon_swing_time * TUX_SWING_PHASES * 1.0 / TOTAL_SWING_TIME ) ) ;
      if ( Me . weapon_swing_time > TOTAL_SWING_TIME ) Me . weapon_swing_time = (-1) ;
      if ( ( (int) Me . phase ) > TUX_SWING_PHASES + TUX_BREATHE_PHASES )
	{
	  Me .phase = 0;
	}
      Me . walk_cycle_phase = 17 ;
      // DebugPrintf ( 0 , "\nphase = %d. " , (int) Me . phase );
    }

  
  if ( ( ( int ) ( Me . phase ) ) >= TUX_TOTAL_PHASES )
    {
      Me .phase = 0;
    }

  // Me . walk_cycle_phase = Me . phase ;

}; // void animate_tux ( void )

/**
 * This function creates several exprosions around the location where the
 * influencer is (was) positioned.  It is used after the influencers 
 * death to make his death more spectacular.
 */
void
start_tux_death_explosions (void)
{
    int i;
    int counter;
    
    DebugPrintf ( 1, "\n%s(): Real function call confirmed." , __FUNCTION__ );
    
    // create a few shifted explosions...
    for (i = 0; i < 10; i++)
    {
	
	// find a free blast
	counter = 0;
	while (AllBlasts[counter++].type != INFOUT);
	counter -= 1;
	if (counter >= MAXBLASTS)
	{
	    ErrorMessage ( __FUNCTION__  , 
				       "Ran out of blasts!!!" ,
				       PLEASE_INFORM, IS_FATAL );
	}
	AllBlasts[counter].type = DRUIDBLAST;
	AllBlasts[counter].pos.x =
	    Me.pos.x - 0.125 / 2 + MyRandom (10)*0.05;
	AllBlasts[counter].pos.y =
	Me.pos.y - 0.125 / 2 + MyRandom (10)*0.05;
	AllBlasts[counter].phase = i;
    }

    DebugPrintf ( 1 , "\n%s(): Usual end of function reached." , __FUNCTION__ );

}; // void start_tux_death_explosions ( void )

/**
 * This function checks if the influencer is currently colliding with an
 * enemys and throws him back in that case.
 */
void
check_tux_enemy_collision (void)
{
    float xdist;
    float ydist;
   
    enemy *erot, *nerot;
    BROWSE_LEVEL_BOTS_SAFE(erot, nerot, Me . pos .z) 
    {
	
	if ( erot->type == ( -1 ) || erot->pure_wait )
	    continue;
	
	//--------------------
	// We determine the distance and back out immediately if there
	// is still one whole square distance or even more...
	//
	xdist = Me . pos . x - erot-> pos . x;
	if (abs (xdist) > 1)
	    continue;
	ydist = Me . pos . y - erot-> pos . y;
	if (abs (ydist) > 1)
	    continue;
	
	//--------------------
	// Now at this point we know, that we are pretty close.  It is time
	// to calculate the exact distance and to see if the exact distance
	// indicates a collision or not, in which case we can again back out
	//
	//
	if ( ( fabsf( xdist ) >= 2.0*0.25 ) ||
	     ( fabsf( ydist ) >= 2.0*0.25 ) ) 
	    continue;
	
	erot->pure_wait = WAIT_COLLISION;

        short int swap = erot->nextwaypoint;
	erot->nextwaypoint = erot->lastwaypoint;
	erot->lastwaypoint = swap;
	
    }			

}; // void check_tux_enemy_collision( void )

/**
 * This function checks if there is some living droid below the current
 * mouse cursor and returns the index number of this droid in the array.
 * 
 * Earlier we did this by computing the map location the mouse was pointing
 * to and using that for the computation of the distance to droid coordinates.
 * The problem with this method is, that some droids are very 'high' in
 * the sense that the graphics (e.g. 302 body) is very far away from the
 * 'foot' point, where the droid is in X-Y coordinates on the map.  Therefore
 * some correction has to be done to fix this.  We can't use the map position
 * of the mouse any more... except maybe to exclude some bots from the start.
 *
 */
enemy * 
GetLivingDroidBelowMouseCursor ( )
{
    float Mouse_Blocks_X, Mouse_Blocks_Y;
    // float DistanceFound = 1000;
    // float CurrentDistance;
    // SDL_Rect enemy_screen_rectangle;
    int RotationModel, RotationIndex;
    iso_image* our_iso_image ;

    Mouse_Blocks_X = translate_pixel_to_map_location ( (float) input_axis.x ,
						       (float) input_axis.y , TRUE ) ;
    Mouse_Blocks_Y = translate_pixel_to_map_location ( (float) input_axis.x ,
						       (float) input_axis.y , FALSE ) ;
    

    enemy *this_bot;
    BROWSE_ALIVE_BOTS(this_bot)
    {
	if ( this_bot -> pos . z != Me . pos . z )
	    continue;
	if ( fabsf ( this_bot -> pos . x - ( Mouse_Blocks_X ) ) >= 5.0 )
	    continue;
	if ( fabsf ( this_bot -> pos . y - ( Mouse_Blocks_Y ) ) >= 5.0 )
	    continue;
	
	//--------------------
	// We properly set the direction this robot is facing.
	//
	RotationIndex = set_rotation_index_for_this_robot ( this_bot ) ;
	
	//--------------------
	// We properly set the rotation model number for this robot, i.e.
	// which shape (like 302, 247 or proffa) to use for drawing this bot.
	//
	RotationModel = set_rotation_model_for_this_robot ( this_bot ) ;

	our_iso_image = & ( enemy_iso_images [ RotationModel ] [ RotationIndex ] [ (int) this_bot -> animation_phase ] ) ;

	if ( mouse_cursor_is_on_that_iso_image ( this_bot -> pos . x , 
						 this_bot -> pos . y , 
						 our_iso_image ) )
	{
	    return this_bot;
	}
    }
    
    return ( NULL );
    
}; // int GetLivingDroidBelowMouseCursor ( )

/**
 * This function checks if there is some living droid below the current
 * mouse cursor and returns the index number of this droid in the array.
 * 
 * Earlier we did this by computing the map location the mouse was pointing
 * to and using that for the computation of the distance to droid coordinates.
 * The problem with this method is, that some droids are very 'high' in
 * the sense that the graphics (e.g. 302 body) is very far away from the
 * 'foot' point, where the droid is in X-Y coordinates on the map.  Therefore
 * some correction has to be done to fix this.  We can't use the map position
 * of the mouse any more... except maybe to exclude some bots from the start.
 *
 */
int 
GetObstacleBelowMouseCursor ( )
{
    int i;
    float Mouse_Blocks_X, Mouse_Blocks_Y;
    int TargetFound = (-1);
    iso_image* our_iso_image ;
    int x ;
    int y ;
    level* our_level;
    int obstacle_index;
    obstacle* our_obstacle;

    Mouse_Blocks_X = translate_pixel_to_map_location ( (float) input_axis.x , 
						       (float) input_axis.y , TRUE ) ;
    Mouse_Blocks_Y = translate_pixel_to_map_location ( (float) input_axis.x , 
						       (float) input_axis.y , FALSE ) ;

    our_level = curShip . AllLevels [ Me . pos . z ] ;


    for ( x = (int) Mouse_Blocks_X - 3 ; x < (int) Mouse_Blocks_X + 4 ; x ++ )
    {
	if ( x < 0 ) continue ;
	if ( x >= our_level -> xlen ) continue ;

	for ( y = (int) Mouse_Blocks_Y - 3 ; y < (int) Mouse_Blocks_Y + 4 ; y ++ )
	{
	    if ( y < 0 ) continue ;
	    if ( y >= our_level -> ylen ) continue ;

	    for ( i = 0 ; i < MAX_OBSTACLES_GLUED_TO_ONE_MAP_TILE ; i ++ )
	    {
		obstacle_index = our_level -> map [ y ] [ x ] . obstacles_glued_to_here [ i ] ;
		if ( obstacle_index == (-1) ) continue;
		our_obstacle = & ( our_level -> obstacle_list [ obstacle_index ] ) ;
		our_iso_image = get_obstacle_image(our_obstacle -> type);
		
		// DebugPrintf ( -4 , "\n%s(): rectangle check reached." , __FUNCTION__ );
		if ( mouse_cursor_is_on_that_iso_image ( our_obstacle -> pos . x , 
							 our_obstacle -> pos . y , 
							 our_iso_image ) )
		{
		    TargetFound = obstacle_index ;
		}
	    }
	}
	 
    }
   
    // DebugPrintf ( -4 , "\n%s(): found: %d." , __FUNCTION__ , TargetFound );

    return ( TargetFound );
    
}; // int GetObstacleBelowMouseCursor ( )

void FillInDefaultBulletStruct ( bullet * CurBullet, int bullet_image_type, short int weapon_item_type ) 
{

    memset(CurBullet, 0, sizeof(bullet));
    // Fill in what wasn't specified
    CurBullet -> pos . x = Me . pos . x ;
    CurBullet -> pos . y = Me . pos . y ;
    CurBullet -> pos . z = Me . pos . z ;
    CurBullet -> type = bullet_image_type;
    
    //--------------------
    // Previously, we had the damage done only dependant upon the weapon used.  Now
    // the damage value is taken directly from the character stats, and the UpdateAll...stats
    // has to do the right computation and updating of this value.  hehe. very conventient.
    CurBullet->damage = Me . base_damage + MyRandom( Me .damage_modifier);
    CurBullet->mine = TRUE;
    CurBullet->owner = -1;
    CurBullet -> time_to_hide_still = 0.3 ;
    CurBullet->bullet_lifetime        = ItemMap[ weapon_item_type ].item_gun_bullet_lifetime;
    CurBullet->ignore_wall_collisions = ItemMap[ weapon_item_type ].item_gun_bullet_ignore_wall_collisions;
    CurBullet->was_reflected = FALSE;
    CurBullet->reflect_other_bullets   = ItemMap[ weapon_item_type ].item_gun_bullet_reflect_other_bullets;
    CurBullet->pass_through_explosions = ItemMap[ weapon_item_type ].item_gun_bullet_pass_through_explosions;
    CurBullet->pass_through_hit_bodies = ItemMap[ weapon_item_type ].item_gun_bullet_pass_through_hit_bodies;
    
    CurBullet->time_in_frames = 0;
    CurBullet->time_in_seconds = 0;

    CurBullet->to_hit = Me .to_hit;
    
    CurBullet->freezing_level = 0;
    CurBullet->poison_duration = 0;
    CurBullet->poison_damage_per_sec = 0;
    CurBullet->paralysation_duration = 0;
}

/**
 * This function fires a bullet from the influencer in some direction, 
 * no matter whether this is 'allowed' or not, not questioning anything
 * and SILENTLY TRUSTING THAT THIS TUX HAS A RANGED WEAPON EQUIPPED.
 */
void
FireTuxRangedWeaponRaw ( short int weapon_item_type , int bullet_image_type, bullet * bullet_parameters , moderately_finepoint target_location ) 
{
    int i = 0;
    bullet * CurBullet = NULL;
    float BulletSpeed = ItemMap [ weapon_item_type ] . item_gun_speed;
    double speed_norm;
    moderately_finepoint speed;
    float OffsetFactor;
    moderately_finepoint offset;
    OffsetFactor = 0.25 ;
#define FIRE_TUX_RANGED_WEAPON_RAW_DEBUG 1
    
    DebugPrintf ( 1 , "\n%s(): target location: x=%f, y=%f." , __FUNCTION__ , 
		  target_location . x , target_location . y );

    //--------------------
    // We search for the first free bullet entry in the 
    // bullet list...
    //
    i = find_free_bullet_index ();
    CurBullet = & ( AllBullets [ i ] ) ;

    if ( bullet_parameters ) memcpy(CurBullet, bullet_parameters, sizeof(bullet));    
    else FillInDefaultBulletStruct (CurBullet, bullet_image_type, weapon_item_type );

    
    //--------------------
    // Now we can set up recharging time for the Tux...
    // The firewait time is now modified by the ranged weapon skill
    // 
    Me . busy_time = ItemMap[ weapon_item_type ].item_gun_recharging_time;
    Me . busy_time *= RangedRechargeMultiplierTable [ Me . ranged_weapon_skill ] ;
    Me . busy_type = WEAPON_FIREWAIT;
    
    //--------------------
    // Use the map location to
    // pixel translation and vice versa to compute firing direction...
    //
    // But this is ONLY A FIRST ESTIMATE!  It will be fixed shortly!
    //
    // speed . x = translate_pixel_to_map_location ( input_axis.x , input_axis.y , TRUE ) - Me . pos . x ;
    // speed . y = translate_pixel_to_map_location ( input_axis.x , input_axis.y , FALSE ) - Me . pos . y ;
    speed . x = target_location . x - Me . pos . x ; 
    speed . y = target_location . y - Me . pos . y ; 
    speed_norm = sqrt ( speed . x * speed . x + speed . y * speed . y );
    if ( ! speed_norm ) fprintf(stderr, "Bullet speed is zero\n");
    CurBullet -> speed.x = ( speed . x / speed_norm );
    CurBullet -> speed.y = ( speed . y / speed_norm );
    CurBullet -> speed . x *= BulletSpeed;
    CurBullet -> speed . y *= BulletSpeed;
    
    DebugPrintf( FIRE_TUX_RANGED_WEAPON_RAW_DEBUG , 
		 "\nFireTuxRangedWeaponRaw(...) : speed_norm = %f." , speed_norm );
    
    //--------------------
    // Now the above vector would generate a fine bullet, but it will
    // be modified later to come directly from the Tux weapon muzzle.
    // Therefore it might often miss a target standing very close.
    // As a reaction, we will shift the target point by a similar vector
    // and then re-compute the bullet vector.
    //
    offset . x = OffsetFactor * ( CurBullet -> speed . x / BulletSpeed );
    offset . y = OffsetFactor * ( CurBullet -> speed . y / BulletSpeed );
    RotateVectorByAngle ( & ( offset ) , -60 );
    
    //--------------------
    // And now we re-do it all!  But this time with the extra offset
    // applied to the SHOT TARGET POINT!
    //
    speed . x = target_location . x - Me . pos . x - offset . x ;
    speed . y = target_location . y - Me . pos . y - offset . y ;
    speed_norm = sqrt ( speed . x * speed . x + speed . y * speed . y );
    CurBullet -> speed.x = ( speed . x / speed_norm );
    CurBullet -> speed.y = ( speed . y / speed_norm );
    CurBullet -> speed . x *= BulletSpeed;
    CurBullet -> speed . y *= BulletSpeed;
    
    DebugPrintf( FIRE_TUX_RANGED_WEAPON_RAW_DEBUG , 
		 "\nFireTuxRangedWeaponRaw(...) : speed_norm = %f." , speed_norm );
    
    //--------------------
    // Now we determine the angle of rotation to be used for
    // the picture of the bullet itself
    //
    
    CurBullet -> angle = - ( atan2 ( speed . y ,  speed . x ) * 180 / M_PI + 90 + 45 );
    
    DebugPrintf( FIRE_TUX_RANGED_WEAPON_RAW_DEBUG , 
		 "\nFireTuxRangedWeaponRaw(...) : Phase of bullet=%d." , CurBullet->phase );
    DebugPrintf( FIRE_TUX_RANGED_WEAPON_RAW_DEBUG , 
		 "\nFireTuxRangedWeaponRaw(...) : angle of bullet=%f." , CurBullet->angle );
    
    //--------------------
    // To prevent influ from hitting himself with his own bullets,
    // move them a bit..
    //
    offset . x = OffsetFactor * ( CurBullet -> speed . x / BulletSpeed );
    offset . y = OffsetFactor * ( CurBullet -> speed . y / BulletSpeed );
    RotateVectorByAngle ( & ( offset ) , -60 );
    
    CurBullet -> pos . x += offset . x ;
    CurBullet -> pos . y += offset . y ; 

    CurBullet -> is_friendly = 1;

    DebugPrintf ( 0 , "\nOffset:  x=%f y=%f." , offset . x , offset . y );
    
    DebugPrintf( FIRE_TUX_RANGED_WEAPON_RAW_DEBUG , 
		 "\nFireTuxRangedWeaponRaw(...) : final position of bullet = (%f/%f)." , 
		 CurBullet->pos . x , CurBullet->pos . y );
    DebugPrintf( FIRE_TUX_RANGED_WEAPON_RAW_DEBUG , 
		 "\nFireTuxRangedWeaponRaw(...) : BulletSpeed=%f." , BulletSpeed );
    
}; // void FireTuxRangedWeaponRaw ( ) 

/**
 * In some cases, the mouse button will be pressed, but still some signs
 * might tell us, that this mouse button press was not intended as a move
 * or fire command to the Tux.  This function checks for these cases.
 */
int 
ButtonPressWasNotMeantAsFire( )
{
  //--------------------
  // If the influencer is holding something from the inventory
  // menu via the mouse, also just return
  //
  if ( Item_Held_In_Hand != (-1) ) return ( TRUE ) ;
  if ( timeout_from_item_drop > 0 ) return ( TRUE ) ;

  //--------------------
  // Maybe the player just pressed the mouse button but INSIDE one of the character/skills/inventory
  // screens.  Then of course we will not interpret the intention to fire the weapon but rather 
  // return from here immediately.
  //
  if ( MouseLeftPressed() && 
       ( GameConfig.Inventory_Visible || GameConfig.CharacterScreen_Visible || GameConfig.SkillScreen_Visible || GameConfig.skill_explanation_screen_visible) && 
       ! MouseCursorIsInUserRect( User_Rect.x + User_Rect.w/2 + input_axis.x , User_Rect.y + User_Rect.h/2 + input_axis.y ) )
    { 
      DebugPrintf( 0 , "\nCursor outside user-rect:\n  User_Rect.x=%d, User_Rect.w=%d, User_Rect.y=%d, User_Rect.h=%d." ,
		   User_Rect.x , User_Rect.w , User_Rect.y , User_Rect.h );
      DebugPrintf( 0 , "\nCursor position: X=%d, Y=%d." ,
		   input_axis.x , input_axis.y );
      return ( TRUE );
    }

  return ( FALSE );

}; // int ButtonPressWasNotMeantAsFire ( )

/**
 * When the user clicks on a button in the HUD, launch the related action.
 * Return TRUE if the mouse is pressed inside the HUD's area.
 */
int
handle_click_in_hud( )
{
	if ( !MouseLeftPressed() ) return (FALSE);

	if ( MouseCursorIsOnButton( INV_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
		if ( MouseLeftClicked() )
		{
			toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_INVENTORY );
		}
		return (TRUE);
	}

	if ( MouseCursorIsOnButton( CHA_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
		if ( MouseLeftClicked() )
		{
			toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_CHARACTER );
		}
		return (TRUE);
	}

	if ( MouseCursorIsOnButton( SKI_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
		if ( MouseLeftClicked() )
		{
			toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_SKILLS );
		}
		return (TRUE);
	}

	if ( MouseCursorIsOnButton( LOG_SCREEN_TOGGLE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
		if ( MouseLeftClicked() )
		{
			quest_browser_interface ( );
		}
		return (TRUE);
	}

	if ( MouseCursorIsOnButton( WEAPON_MODE_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
		if ( MouseLeftClicked() )
		{
			TuxReloadWeapon ( );
		}
		return (TRUE);
	}

	if ( MouseCursorIsOnButton( SKI_ICON_BUTTON , GetMousePos_x()  , GetMousePos_y()  ) )
	{
		if ( MouseLeftClicked() )
		{
			toggle_game_config_screen_visibility ( GAME_CONFIG_SCREEN_VISIBLE_SKILLS );
		}
		return (TRUE);
	}

	// Finally, protect all the other parts of the hud from user's clicks
	if ( GetMousePos_y() >= UNIVERSAL_COORD_H(SUBTITLEW_RECT_Y) ) return (TRUE);

	// The mouse is outside the hud.
	return (FALSE);

} // int handle_click_in_hud

/**
 * At some point in the analysis of the users mouse click, we'll be 
 * certain, that a fireing/weapon swing was meant with the click.  Once
 * this is knows, this function can be called to do the mechanics of the
 * weapon use.
 */
int
PerformTuxAttackRaw ( int use_mouse_cursor_for_targeting ) 
{
    int guntype;
    if ( Me . weapon_item . type > 0 ) 
	guntype = ItemMap[ Me . weapon_item . type ] . item_gun_bullet_image_type ;
    else guntype = -1;

    float angle;
    moderately_finepoint Weapon_Target_Vector ;
    moderately_finepoint target_location ;
    enemy * droid_under_melee_attack_cursor = NULL;
    int melee_weapon_hit_something = FALSE ;
    int do_melee_strike ;
    finepoint MapPositionOfMouse;

#define PERFORM_TUX_ATTACK_RAW_DEBUG 1

    //--------------------
    // We should always make the sound of a fired bullet (or weapon swing)
    // and then of course also subtract a certain fee from the remaining weapon
    // duration in the course of the swing/hit
    //
    DebugPrintf ( PERFORM_TUX_ATTACK_RAW_DEBUG , "\nWeapon_item: %d guntype: %d . " ,  Me . weapon_item . type , guntype );

    
    //--------------------
    // We always start the weapon application cycle, i.e. change of Tux
    // motion phases
    //
    Me . weapon_swing_time = 0;

    //--------------------
    // Now that an attack is being made, the Tux must turn thowards the direction
    // of the attack, no matter what.
    //
    // Note:  This is just some default value.  The real fine-tuning of the direction
    //        is done further below...
    //
    if ( use_mouse_cursor_for_targeting )
    {
	Me . angle = - ( atan2 ( input_axis.y ,  
						input_axis.x ) * 180 / M_PI + 90 );
    }

    if ( !APressed() )
    {
	//--------------------
	// By default, we set an attack target according to the mouse 
	// cursor.  If there is something else going on, this will simply
	// be overwritten.  But it's a good default this way.
	//
	MapPositionOfMouse . x = translate_pixel_to_map_location ( 
	    (float) input_axis.x , 
	    (float) input_axis.y , TRUE ) ;
	MapPositionOfMouse . y = translate_pixel_to_map_location ( 
	    (float) input_axis.x , 
	    (float) input_axis.y , FALSE ) ;
	angle = - ( atan2 ( Me . pos . y - MapPositionOfMouse . y ,
			    Me . pos . x - MapPositionOfMouse . x ) * 180 / M_PI - 90 + 22.5 );
	target_location . x = MapPositionOfMouse . x ;
	target_location . y = MapPositionOfMouse . y ;

	//--------------------
	//
	if ( enemy_resolve_address(Me . current_enemy_target_n, &Me.current_enemy_target_addr) != NULL )
	{
	    droid_under_melee_attack_cursor = enemy_resolve_address(Me . current_enemy_target_n, &Me.current_enemy_target_addr) ;
	    DebugPrintf ( 1 , "\n%s(): using MOUSE MOVE TARGET at X=%d Y=%d for attack direction of tux." , __FUNCTION__ , (int) Me . mouse_move_target . x , (int) Me . mouse_move_target . y );
	}
	else
	{
	    droid_under_melee_attack_cursor = GetLivingDroidBelowMouseCursor ( ) ;
	    DebugPrintf ( 1 , "\n%s(): Using droid under mouse cursor for attack positioning..." , __FUNCTION__ );
	}
	
	
	if ( droid_under_melee_attack_cursor != NULL )
	{
	    angle = - ( atan2 ( Me . pos . y - 
				droid_under_melee_attack_cursor->pos . y ,
				Me . pos . x - 
				droid_under_melee_attack_cursor->pos . x ) * 180 / M_PI - 90 + 22.5 );
	    target_location . x = droid_under_melee_attack_cursor->pos . x ;
	    target_location . y = droid_under_melee_attack_cursor->pos . y ; 
	}
	else
	{
	    //--------------------
	    // We leave the angle at the current value...
	    // (this is because later angle is written over Me[..].angle
	    //
	    angle = Me . angle ;
	    DebugPrintf ( 1 , "\n%s(): defaulting to PREVIOUS ANGLE!" , __FUNCTION__ );
	}
    }
    else
    {
	MapPositionOfMouse . x = translate_pixel_to_map_location ( 
	    (float) input_axis.x , 
	    (float) input_axis.y , TRUE ) ;
	MapPositionOfMouse . y = translate_pixel_to_map_location ( 
	    (float) input_axis.x , 
	    (float) input_axis.y , FALSE ) ;
	angle = - ( atan2 ( Me . pos . y - MapPositionOfMouse . y ,
			    Me . pos . x - MapPositionOfMouse . x ) * 180 / M_PI - 90 + 22.5 );
	target_location . x = MapPositionOfMouse . x ;
	target_location . y = MapPositionOfMouse . y ;
	// DebugPrintf ( -4 , "\n%s(): target location: x=%f, y=%f." , __FUNCTION__ , 
	// target_location . x , target_location . y );	
    }
    
    
    //--------------------
    // We need to write this back into the Tux struct, because the
    // value from there is used in the blitting code.
    //
    Me . angle = angle ;
    
    
    //--------------------
    // But if the currently used weapon is a melee weapon, the tux no longer
    // generates a bullet, but rather does his weapon swinging motion and
    // only the damage is done to the robots in the area of effect
    //
    do_melee_strike = FALSE ;
    if ( Me . weapon_item . type == (-1) )
	do_melee_strike = TRUE ;
    else if ( ItemMap [ Me . weapon_item . type ] . item_weapon_is_melee != 0 )
	do_melee_strike = TRUE ;
    if ( do_melee_strike )
    {
	//--------------------
	// Since a melee weapon is swung, which may be only influencers fists,
	// we calculate where the point
	// of the weapon should be finally hitting and do some damage
	// to all the enemys in that area.
	//
	// However, simply using the pixel-to-map-location code with the current
	// mouse pointer is not the way, since the droid (e.g. 302) might be high
	// above the floor and the click right on it might point to a floor spot
	// right behind the Tux actual position, so care is advised in that case...
	// 

	DebugPrintf( PERFORM_TUX_ATTACK_RAW_DEBUG , "\n===> Fire Bullet: angle=%f. " , angle ) ;
	DebugPrintf( PERFORM_TUX_ATTACK_RAW_DEBUG , "\n===> Fire Bullet: InpAxis: X=%d Y=%d . " , 
		     input_axis.x , 
		     input_axis.y ) ;
	Weapon_Target_Vector.x = 0 ;
	Weapon_Target_Vector.y = - 0.8 ;
	RotateVectorByAngle ( & Weapon_Target_Vector , angle );
	Weapon_Target_Vector.x += Me . pos . x;
	Weapon_Target_Vector.y += Me . pos . y;
	DebugPrintf( PERFORM_TUX_ATTACK_RAW_DEBUG , "\n===> Fire Bullet target: x=%f, y=%f. " , Weapon_Target_Vector.x , Weapon_Target_Vector.y ) ;
	
	enemy *erot, *nerot;
	BROWSE_LEVEL_BOTS_SAFE(erot, nerot, Me.pos.z)
	    {
	    if ( ( fabsf ( erot-> pos . x - Weapon_Target_Vector.x ) > 0.5 ) ||
		    ( fabsf ( erot-> pos . y - Weapon_Target_Vector.y ) > 0.5 ) ||
		    !DirectLineColldet(Me.pos.x, Me.pos.y, erot->pos.x, erot->pos.y, Me.pos.z, NULL)
	       )
		continue;

	    /* Set up a melee attack */
	    int shot_index = find_free_melee_shot_index ();
	    melee_shot *NewShot = & ( AllMeleeShots [ shot_index ] );

	    NewShot -> attack_target_type = ATTACK_TARGET_IS_ENEMY;
	    NewShot -> mine = 1;

	    NewShot -> bot_target_n = erot->id;
	    NewShot -> bot_target_addr = erot;

	    NewShot->to_hit = Me.to_hit;
	    NewShot->damage = Me . base_damage + MyRandom ( Me . damage_modifier );
	    NewShot->owner = -1; //no "bot class number" owner

	    melee_weapon_hit_something = TRUE;

	    //--------------------
	    // War tux freezes enemys with the appropriate plugin...
	    erot->frozen += Me . freezing_melee_targets ; 

	    }
	
	//--------------------
	// Also, we should check if there was perhaps a chest or box
	// or something that can be smashed up, cause in this case, we
	// must open pendoras box now.
	//
	// SmashBox ( Weapon_Target_Vector.x , Weapon_Target_Vector.y );
	if ( smash_obstacle ( Weapon_Target_Vector . x , Weapon_Target_Vector . y ) )
		melee_weapon_hit_something = TRUE;
	
	//--------------------
	// Finally we add a new wait-counter, so that bullets or swings
	// cannot be started in too rapid succession.  
	// 
	// And then we can return, for real bullet generation in the sense that
	// we would have to enter something into the AllBullets array or that
	// isn't required in our case here.
	//
	if ( Me . weapon_item . type != ( -1 ) )
	    Me . busy_time = ItemMap [ Me . weapon_item.type ] . item_gun_recharging_time;
	else
	    Me . busy_time = 0.5;
	
	// Now we modify for melee weapon skill...
	Me . busy_time *= 
	    MeleeRechargeMultiplierTable [ Me . melee_weapon_skill ] ;
	Me . busy_type = WEAPON_FIREWAIT;

	if ( melee_weapon_hit_something ) play_melee_weapon_hit_something_sound();
	else play_melee_weapon_missed_sound();
	
	return 0;
    }

    //--------------------
    //
    if ( Me . weapon_item . type != (-1) ) Fire_Bullet_Sound ( guntype );
    else Fire_Bullet_Sound ( LASER_SWORD_1 );
    
    FireTuxRangedWeaponRaw ( Me . weapon_item . type , guntype, NULL , target_location ) ;
    return 0;
}; // void PerformTuxAttackRaw ( ) ;

/**
 * Reload the ammo clip
 *
 *
 */

void TuxReloadWeapon()
{
    int munition_type = -1;

    if ( ItemMap [ Me . weapon_item . type ] . item_gun_ammo_clip_size == Me . weapon_item . ammo_clip )
	return ; //clip full, return without reloading 

    if  ( Me . paralyze_duration ) //do not reload when paralyzed 
	return;

    if ( Me . weapon_item . type == -1 )
	return; // do not reload Tux's fists

    switch ( ItemMap [ Me . weapon_item . type ] . item_gun_use_ammunition )
	{
	case 0: //no ammo
	    return; //no reloading occurs
	    break;
	case 1: //laser
	    munition_type = GetItemIndexByName("Laser power pack");
	    break;
	case 2: //plasma
	    munition_type = GetItemIndexByName("Plasma energy container");
	    break;
	case 3: //exterminator
	    munition_type = GetItemIndexByName("2 mm Exterminator Ammunition");
	    break;
	case 4: //22LR
	    munition_type = GetItemIndexByName(".22 LR Ammunition"); 
	    break; 
	case 5: //Sshell
	    munition_type = GetItemIndexByName("Shotgun shells"); 
	    break; 
	case 6: //9mm
	    munition_type = GetItemIndexByName("9x19mm Ammunition"); 
	    break;
	case 7: //7.62mm
	    munition_type = GetItemIndexByName("7.62x39mm Ammunition"); 
	    break;
	case 8: //50BMG
	    munition_type = GetItemIndexByName(".50 BMG (12.7x99mm) Ammunition"); 
	    break;
	default:
	    ErrorMessage ( __FUNCTION__  , "Got an unknown munition type %d for your current weapon %s.",						                                          PLEASE_INFORM, IS_FATAL, ItemMap [ Me . weapon_item . type ] . item_gun_use_ammunition, ItemMap [ Me . weapon_item . type ] . item_name );
	}

    int count = CountItemtypeInInventory ( munition_type ); 
    if(count > ItemMap [ Me . weapon_item . type ] . item_gun_ammo_clip_size - Me . weapon_item . ammo_clip )
	count =  ItemMap [ Me . weapon_item . type ] . item_gun_ammo_clip_size - Me . weapon_item . ammo_clip;

    if(!count) //no ammo found, tell the player that he "has it in the baba"
	{
	No_Ammo_Sound( );
	No_Ammo_Sound( );
	append_new_game_message(_("No more ammunition !"));
	return;
	}
    int i;
    for(i = 0; i < count; i++)
	DeleteOneInventoryItemsOfType( munition_type );
    Me . weapon_item . ammo_clip += count;
    Me . busy_time = ItemMap[ Me . weapon_item . type ].item_gun_reloading_time;
    Me . busy_time *= RangedRechargeMultiplierTable [ Me . ranged_weapon_skill ] ;
    Me . busy_type = WEAPON_RELOAD;
}

/**
 * When the player has left-clicked into the game area (i.e. the isometric
 * display of the game world), we need to check if maybe the click was
 * targeted on a chest, that can be opened.
 *
 * This function checks if that really was the case, taking into account 
 * not only the floor position of the mouse cursor (which would lead to 
 * rather unintuitive clicking areas) but rather the full chest graphics.
 */
void
check_for_chests_to_open ( int chest_index ) 
{
    Level our_level = curShip . AllLevels [ Me . pos . z ] ;

    if ( chest_index != (-1) )
    {
	//--------------------
	// So the player clicked on a chest.  Well, if the chest is already close
	// enough, it should be sufficient to just spill out the contents of the
	// chest and then return.  However, if the player was not yet close enough
	// to the chest, we need to SET A COMBINED_ACTION, i.e. first set the walk
	// thowards the chest and then set the open_chest action, which is more
	// complicated of course.
	//
	if ( fabsf ( Me . pos . x - our_level -> obstacle_list [ chest_index ] . pos . x ) +
	     fabsf ( Me . pos . y - our_level -> obstacle_list [ chest_index ] . pos . y ) < 1.1 )
	{
	    //--------------------
	    // Check that the chest is really reachable, and not behind an obstacle
		colldet_filter filter = { ObstacleByIdPassFilterCallback, &chest_index, NULL };
	    if ( DirectLineColldet( Me.pos.x, Me.pos.y,
                                our_level->obstacle_list[chest_index].pos.x, our_level->obstacle_list[chest_index].pos.y,
                                Me.pos.z, &filter) )
	    {
	    throw_out_all_chest_content ( chest_index ) ;
	    
	    if ( our_level -> obstacle_list [ chest_index ] . type == ISO_H_CHEST_CLOSED )
		our_level -> obstacle_list [ chest_index ] . type = ISO_H_CHEST_OPEN  ;
	    if ( our_level -> obstacle_list [ chest_index ] . type == ISO_V_CHEST_CLOSED )
		our_level -> obstacle_list [ chest_index ] . type = ISO_V_CHEST_OPEN  ;
	    }
	    
	    //--------------------
	    // Maybe a combo_action has made us come here and open the chest.  Then of
	    // course we can remove the combo action setting now...
	    //
	    Me . mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET ;
	    Me . mouse_move_target_combo_action_parameter = ( -1 ) ;
	    
	    //--------------------
	    // Now that the chest has been possibly opened, we don't need to do anything more
	    //
	    return;
	}
	else
	{
	    //--------------------
	    // So here we know, that we must set the course thowards the chest.  We
	    // do so first.
	    //
	    DebugPrintf ( 2 , "\ncheck_for_chests_to_open:  setting up combined mouse move target!" );
	    
	    switch ( our_level -> obstacle_list [ chest_index ] . type )
	    {
		case ISO_V_CHEST_CLOSED:
		case ISO_V_CHEST_OPEN:
		    Me . mouse_move_target . x = our_level -> obstacle_list [ chest_index ] . pos . x ;
		    Me . mouse_move_target . y = our_level -> obstacle_list [ chest_index ] . pos . y ;
		    Me . mouse_move_target . z = Me . pos . z ;
		    Me . mouse_move_target . x += 0.8 ;
		   
		    enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
		    Me . mouse_move_target_combo_action_type = COMBO_ACTION_OPEN_CHEST ;
		    Me . mouse_move_target_combo_action_parameter = chest_index ;
		    
		    break;
		case ISO_H_CHEST_CLOSED:
		case ISO_H_CHEST_OPEN:
		    Me . mouse_move_target . x = our_level -> obstacle_list [ chest_index ] . pos . x ;
		    Me . mouse_move_target . y = our_level -> obstacle_list [ chest_index ] . pos . y ;
		    Me . mouse_move_target . z = Me . pos . z ;
		    Me . mouse_move_target . y += 0.8 ;
		    
		    enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
		    Me . mouse_move_target_combo_action_type = COMBO_ACTION_OPEN_CHEST ;
		    Me . mouse_move_target_combo_action_parameter = chest_index ;
		    
		    break;
		default:
		    ErrorMessage ( __FUNCTION__  , 
					       "chest to be approached is not a chest obstacle!!" ,
					       PLEASE_INFORM, IS_FATAL );
		    break;
	    }
	}
    }
    
}; // void check_for_chests_to_open ( ) 

void check_for_items_to_pickup ( int index_of_item_under_mouse_cursor )
{
    Level our_level = curShip . AllLevels [ Me . pos . z ] ;

    if ( index_of_item_under_mouse_cursor != (-1) )
	{ 
	//--------------------
	// We only take the item directly into out 'hand' i.e. the mouse cursor,
	// if the item in question can be reached directly and isn't blocked by
	// some walls or something...
	//
	if (( calc_euklid_distance( Me . pos . x, Me . pos . y, our_level -> ItemList [ index_of_item_under_mouse_cursor ] . pos .x, our_level -> ItemList [ index_of_item_under_mouse_cursor ] . pos . y ) < ITEM_TAKE_DIST ) && 
		DirectLineColldet ( our_level -> ItemList [ index_of_item_under_mouse_cursor ] . pos . x , our_level -> ItemList [ index_of_item_under_mouse_cursor ] . pos . y , Me . pos . x , Me . pos . y , Me . pos . z, NULL ) )
	    {
	    if ( GameConfig.Inventory_Visible == FALSE || MatchItemWithName(our_level -> ItemList [ index_of_item_under_mouse_cursor ] . type, "Cyberbucks") )
		{
		AddFloorItemDirectlyToInventory( & ( our_level -> ItemList [ index_of_item_under_mouse_cursor ] ) );
		return;
		}
	    else
		{
		/* Handled in HandleInventoryScreen(). Dirty and I don't plan on changing that right now.
		 * A.H., 2008-04-06 */
		;
		}
	    }
	else
	    {
	    Me . mouse_move_target . x = 
		our_level -> ItemList [ index_of_item_under_mouse_cursor ] . pos . x ;
	    Me . mouse_move_target . y = 
		our_level -> ItemList [ index_of_item_under_mouse_cursor ] . pos . y ;
	    Me . mouse_move_target . z = 
		Me . pos . z ;

	    //--------------------
	    // We set up the combo_action, so that the barrel can be smashed later...
	    //
	    enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
	    Me . mouse_move_target_combo_action_type = COMBO_ACTION_PICK_UP_ITEM ;
	    Me . mouse_move_target_combo_action_parameter = index_of_item_under_mouse_cursor ;
	    }
	}
}

/**
 * When the player has left-clicked into the game area (i.e. the isometric
 * display of the game world), we need to check if maybe the click was
 * targeted on a barrel or crate that can be smashed.
 *
 * This 
 * function checks if that is really so, taking into account not only the
 * floor position of the mouse cursor (which would lead to rather 
 * unintuitive clicking areas) but rather the full chest graphics.
 */
void
check_for_barrels_to_smash ( int barrel_index ) 
{
	Level our_level = curShip . AllLevels [ Me . pos . z ] ;
	int i;
	moderately_finepoint step_vector;
	float vec_len;

	if ( barrel_index == (-1) ) return;
	
	//--------------------
	// If the smash distance for a barrel is not yet reached, then we must set up
	// a course that will lead us to the barrel and also a combo_action specification,
	// that will cause the corresponding barrel to be smashed upon arrival.
	//
	if ( calc_euklid_distance ( Me . pos . x , Me . pos . y , 
								our_level -> obstacle_list [ barrel_index ] . pos . x ,
								our_level -> obstacle_list [ barrel_index ] . pos . y ) 
		   > ( obstacle_map [ ISO_BARREL_1 ] . block_area_parm_1 * sqrt(2) ) / 2.0 + 0.5 )
	{
		//--------------------
		// We set up a course, that will lead us directly to the barrel, that we are
		// supposed to smash (upon arrival, later).
		//
		// For this purpose, we take a vector and rotate it around the barrel center to
		// find the 'best' location to go to for the smashing motion...

		// First, if the barrel is in direct line with Tux, we search a position that the
		// Tux can reach.

		step_vector . x = Me . pos . x - our_level -> obstacle_list [ barrel_index ] . pos . x ;
		step_vector . y = Me . pos . y - our_level -> obstacle_list [ barrel_index ] . pos . y ;
		vec_len = vect_len ( step_vector );

		//--------------------
		// We normalize the distance of the final walk-point to the barrel center just
		// so, that it will be within the 'strike_distance' we have used just above in
		// the 'distance-met' query.
		//
		step_vector . x *= ( ( obstacle_map [ ISO_BARREL_1 ] . block_area_parm_1 * sqrt(2) ) 
							 / 2.0 + 0.05 ) / vec_len ;
		step_vector . y *= ( ( obstacle_map [ ISO_BARREL_1 ] . block_area_parm_1 * sqrt(2) ) 
							 / 2.0 + 0.05 ) / vec_len ;

		for ( i = 0 ; i < 8 ; i ++ )
		{
			if ( DirectLineColldet( Me.pos.x, Me.pos.y,
									our_level->obstacle_list[barrel_index].pos.x + step_vector.x ,
									our_level->obstacle_list[barrel_index].pos.y + step_vector.y, 
									Me.pos.z, 
									&WalkablePassFilter ) )
			{
				//--------------------
				// The obstacle plus the step vector give us the position to move the
				// Tux to for the optimal strike...
				//
				Me . mouse_move_target . x = our_level -> obstacle_list [ barrel_index ] . pos . x + step_vector . x ;
				Me . mouse_move_target . y = our_level -> obstacle_list [ barrel_index ] . pos . y + step_vector . y ;
				Me . mouse_move_target . z = Me . pos . z ;

				//--------------------
				// We set up the combo_action, so that the barrel can be smashed later...
				//
				enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
				Me . mouse_move_target_combo_action_type = COMBO_ACTION_SMASH_BARREL ;
				Me . mouse_move_target_combo_action_parameter = barrel_index ;
				break;
			}

			//--------------------
			// If this vector didn't bring us any luck, we rotate by 45 degrees and try anew...
			//
			RotateVectorByAngle ( & ( step_vector ) , 45.0 ) ;
		}

		// Second : if the barrel is not in direct line with Tux, we search a position
		// near the barrel that is free of obstacle. However, it could happen that the
		// pathfinder will not be able to find a way to go there, but it would be too
		// costly to launch the pathfinder here. Anyway, if no path exists, Tux will
		// not start to move, so the player will certainly try to get closer to the
		// barrel.

		// we will rotate step_vector around the center of the barrel
		step_vector . x = Me . pos . x - our_level -> obstacle_list [ barrel_index ] . pos . x ;
		step_vector . y = Me . pos . y - our_level -> obstacle_list [ barrel_index ] . pos . y ;
		vec_len = vect_len ( step_vector );
		step_vector . x /= vec_len;
		step_vector . y /= vec_len;

		// point_near_barrel is a point very near the barrel in the step_vector's direction,
		// point_away_from_barrel is a point in the same direction but a bit farther
		moderately_finepoint point_near_barrel, point_away_from_barrel;

		// half-size of the barrel
		int barrel_type = our_level -> obstacle_list [ barrel_index ] . type;
		moderately_finepoint half_size = { ( obstacle_map [ barrel_type ] . block_area_parm_1 * sqrt(2) ) / 2.0,
										   ( obstacle_map [ barrel_type ] . block_area_parm_2 * sqrt(2) ) / 2.0
										 };

		for ( i = 0 ; i < 8 ; i ++ )
		{
			// (no need to optimize this, the compiler will do its job...)
			point_near_barrel . x      = our_level -> obstacle_list [ barrel_index ] . pos . x +
										 step_vector . x * ( half_size . x + 0.05 );
			point_near_barrel . y      = our_level -> obstacle_list [ barrel_index ] . pos . y +
										 step_vector . y * ( half_size . y + 0.05 );
			point_away_from_barrel . x = our_level -> obstacle_list [ barrel_index ] . pos . x +
										 step_vector . x * ( half_size . x + 0.2  );
			point_away_from_barrel . y = our_level -> obstacle_list [ barrel_index ] . pos . y +
										 step_vector . y * ( half_size . y + 0.2  );

			// check if the line from point_near_barrel to point_away_from_barrel is walkable
			if ( DirectLineColldet( point_near_barrel.x, point_near_barrel.y,
									point_away_from_barrel . x, point_away_from_barrel . y,
									Me.pos.z, 
									&WalkablePassFilter) )
			{
				//--------------------
				// point_to_barrel seems good, move Tux there
				//
				Me . mouse_move_target . x = point_near_barrel . x ;
				Me . mouse_move_target . y = point_near_barrel . y ;
				Me . mouse_move_target . z = Me . pos . z ;

				//--------------------
				// We set up the combo_action, so that the barrel can be smashed later, on the
				// second call (made by move_tux_thowards_intermediate_point)...
				//
				enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
				Me . mouse_move_target_combo_action_type = COMBO_ACTION_SMASH_BARREL ;
				Me . mouse_move_target_combo_action_parameter = barrel_index ;

				return;
			}

			// rotate the step_vector to check an other point
			RotateVectorByAngle ( & ( step_vector ) , 45.0 ) ;
		}

		return;
	}

	//===================
	// The character is near enough from the barrel to smash it.
	//
	
	//--------------------
	// Before the barrel can get destroyed and we loose the position information,
	// we record the vector of the charecter's strike direction...
	//
	step_vector . x = - Me . pos . x + our_level -> obstacle_list [ barrel_index ] . pos . x ;
	step_vector . y = - Me . pos . y + our_level -> obstacle_list [ barrel_index ] . pos . y ;

	//--------------------
	// Check that the barrel is really reachable, and not behind an obstacle
	//
	colldet_filter filter = { ObstacleByIdPassFilterCallback, &barrel_index, NULL };
	
	if ( DirectLineColldet( Me.pos.x, Me.pos.y,
							our_level->obstacle_list[barrel_index].pos.x, our_level->obstacle_list[barrel_index].pos.y,
							Me.pos.z, &filter) )
	{
		//--------------------
		// We make sure the barrel gets smashed, even if the strike made by the
		// Tux would be otherwise a miss...
		//
		smash_obstacle ( our_level -> obstacle_list [ barrel_index ] . pos . x , 
						 our_level -> obstacle_list [ barrel_index ] . pos . y);

		//--------------------
		// We start an attack motion...
		// Since the character is just aside the barrel, we use a melee shot,
		// in order to avoid the lost of an ammunition.
		//
		int store_weapon_type = Me.weapon_item.type;
		Me.weapon_item.type = -1;
		tux_wants_to_attack_now ( FALSE ) ;
		Me.weapon_item.type = store_weapon_type;
		
		//--------------------
		// We set a direction of facing directly thowards the barrel in question
		// so that the strike motion looks authentic...
		//
		Me . angle = - ( atan2 ( step_vector . y ,  step_vector . x ) * 180 / M_PI - 180 - 45 );
		Me . angle += 360 / ( 2 * MAX_TUX_DIRECTIONS );
		while ( Me . angle < 0 ) Me . angle += 360;
	}

	Me . mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET ;
	Me . mouse_move_target_combo_action_parameter = ( -1 ) ;
	DebugPrintf ( 2 , "\ncheck_for_barrels_to_smash(...):  combo_action now unset." );
}; // void check_for_barrels_to_smash ( int barrel_index ) 

/**
 * When the player has left-clicked into the game area (i.e. the isometric
 * display of the game world), we need to check if maybe the click was
 * targeted on a droid.  
 * In case that was so, we need to start a dialog or maybe launch an 
 * attack movement.
 */
void
check_for_droids_to_attack_or_talk_with ( ) 
{
    enemy *droid_below_mouse_cursor = GetLivingDroidBelowMouseCursor ( ) ;
    if ( droid_below_mouse_cursor == NULL && (!APressed()) )
    {
	Me . mouse_move_target . x = 
	    translate_pixel_to_map_location ( input_axis.x , input_axis.y , TRUE ) ;
	Me . mouse_move_target . y = 
	    translate_pixel_to_map_location ( input_axis.x , input_axis.y , FALSE ) ;
	Me . mouse_move_target . z = Me . pos . z ;
	if ( ! ShiftPressed() )
	    {
	    enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
	    }

	
	return; 
    }
    else if ( APressed() )
    {
	tux_wants_to_attack_now ( TRUE ) ;
    }
    
    if ( droid_below_mouse_cursor != NULL &&
    	 DirectLineColldet(Me.pos.x, Me.pos.y, droid_below_mouse_cursor->pos.x, droid_below_mouse_cursor->pos.y, Me.pos.z, &VisiblePassFilter) )
    {
       
   	enemy_set_reference(&Me.current_enemy_target_n, &Me.current_enemy_target_addr, droid_below_mouse_cursor); 
	
	if ( enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr)->is_friendly ) 
	{
	    if ( no_left_button_press_in_previous_analyze_mouse_click )
	    {
		ChatWithFriendlyDroid ( enemy_resolve_address(Me.current_enemy_target_n, &Me.current_enemy_target_addr) ) ;
		enemy_set_reference(&Me . current_enemy_target_n, &Me . current_enemy_target_addr, NULL);
	    }
	    
	    return ;
	}
	
	if ( ! ShiftPressed() )
	    {
	    Me . mouse_move_target .x = -1;
	    Me . mouse_move_target .y = -1;
	    }

	if ( Me . weapon_item . type >= 0 )
	{
	    if ( ( ItemMap [ Me . weapon_item . type ] . item_weapon_is_melee ) &&
		 ( calc_euklid_distance ( Me . pos . x , Me . pos . y , 
					  droid_below_mouse_cursor->pos . x ,
					  droid_below_mouse_cursor->pos . y ) 
		   > BEST_MELEE_DISTANCE+0.1 ) )
	    {
		
		

		return;
	    }
	}
	else if ( calc_euklid_distance ( Me . pos . x , Me . pos . y , 
					 droid_below_mouse_cursor->pos . x ,
					 droid_below_mouse_cursor->pos . y ) 
		  > BEST_MELEE_DISTANCE+0.1 )
	{
	    


	    return;
	}

	//--------------------
	// But if we're close enough or there is a ranged weapon in Tux hands,
	// then we can finally start the attack motion right away...
	//
	tux_wants_to_attack_now ( TRUE ) ;
    }
}; // void check_for_droids_to_attack ( ) 

/**
 * When the player has actiavted global examine aid and clicked the
 * left button, the examine command must be executed.  This function
 * should deal with the effects of one such examine click by the player.
 */
void
handle_player_examine_command ( void ) 
{
    int obstacle_index ;
    obstacle* our_obstacle;
    char game_message_text[ 1000 ] ;
    int chat_section;
    enemy * final_bot_found = NULL;

    if ( ! MouseCursorIsInUserRect(GetMousePos_x(), GetMousePos_y()))
	return;

    //--------------------
    // The highest priority is other droids and characters.  If one
    // of those is under the mouse cursor, then the examine/look command
    // is interpretet to
    final_bot_found = GetLivingDroidBelowMouseCursor ( ) ;
    if ( final_bot_found != NULL)
    {
	//--------------------
	// If the character encountered is a character with a 
	// unique and special dialog section associated to him,
	// then we will also print out the unique description for
	// that character according to the dialog section used.
	// Otherwise we will revert to using the standard droid
	// model description found in the droid archetypes.
	//
	chat_section = ResolveDialogSectionToChatFlagsIndex ( final_bot_found -> dialog_section_name ) ; 
	if ( chat_section != PERSON_STANDARD_BOT_AFTER_TAKEOVER )
	{
	    sprintf( game_message_text , "%s" , character_descriptions [ chat_section ] );
	}
	else
	    sprintf( game_message_text , _("This is a %s. %s") , Druidmap [ final_bot_found -> type ] . druidname , Druidmap [ final_bot_found -> type ] . notes );

	global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL; 
	append_new_game_message ( game_message_text );
	return;
    }

    //--------------------
    // Now if there wasn't any living droid encountered, we can now
    // start thinking about obstacles.  Let's see if we find something here
    //
    obstacle_index = GetObstacleBelowMouseCursor ( ) ;
    if ( obstacle_index != (-1) )
    {
	our_obstacle = & ( curShip . AllLevels [ Me . pos . z ] -> obstacle_list [ obstacle_index ] ) ;
	char * longdesc;

	//--------------------
	// Now if the obstacle has an individual description text, then we can 
	// use that for the obstacle examination result.  Otherwise we'll revert
	// to the generic description text for the obstacle type of this obstacle.
	//
	if ( our_obstacle -> description_index >= 0 ) {
	    longdesc = curShip . AllLevels [ Me . pos . z ] -> obstacle_description_list [ our_obstacle -> description_index ];
	}
	else {
	    longdesc = obstacle_map [ our_obstacle -> type ] . obstacle_long_description;
	}

	sprintf ( game_message_text, _("Examining %s. %s"), obstacle_map [ our_obstacle -> type ] . obstacle_short_name, longdesc );
	append_new_game_message ( game_message_text );

	global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL;
	return;
    }

    //--------------------
    // So here we know that neither living droid nor obstacle
    // was found under the mouse cursor.  Then it's the floor being
    // examined.  We say so, currently rather unspecific, but still.
    //
    sprintf ( game_message_text , _("There is nothing here.")) ;
    append_new_game_message ( game_message_text );
    global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL;
}; // void handle_player_examine_command ( ) 

/**
 * If the user clicked his mouse, this might have several reasons.  It 
 * might happen to open some windows, pick up some stuff, smash a box,
 * move somewhere or fire a shot or make a weapon swing.
 * 
 * Therefore it is not so easy to decide what to do upon a users mouse
 * click and so this function analyzes the situation and decides what to
 * do.
 */
void
AnalyzePlayersMouseClick ( )
{
	DebugPrintf ( 2 , "\n===> void AnalyzePlayersMouseClick(): real function call confirmed. " ) ;
	int tmp;

	// This flag avoids the mouse_move_target to change while the user presses
	// LMB to start a combo action.
	static int wait_mouseleft_release = FALSE;
	
	//---------------
	// No action is associated to MouseLeftRelease event or state.
	//
	
	if ( !MouseLeftPressed() ) 
	{
		wait_mouseleft_release = FALSE;
		return;
	}

	//---------------
	// The action associated to MouseLeftPress depends on the global game state
	//
	
	switch ( global_ingame_mode )
	{
	case GLOBAL_INGAME_MODE_SCROLL_UP:
		if (no_left_button_press_in_previous_analyze_mouse_click)
		{
			game_message_protocol_scroll_override_from_user -- ;
			Activate_Conservative_Frame_Computation();
		}
		break;

	case GLOBAL_INGAME_MODE_SCROLL_DOWN:
		if (no_left_button_press_in_previous_analyze_mouse_click)
		{
			game_message_protocol_scroll_override_from_user ++ ;
			Activate_Conservative_Frame_Computation();
		}
		break;

	case GLOBAL_INGAME_MODE_IDENTIFY:
		handle_player_identification_command( );
		global_ingame_mode = GLOBAL_INGAME_MODE_NORMAL ;

		while ( SpacePressed() || MouseLeftPressed() || MouseRightPressed());
		Activate_Conservative_Frame_Computation();

		break;

	case GLOBAL_INGAME_MODE_NORMAL:
		if ( ButtonPressWasNotMeantAsFire( ) ) return;
		if ( handle_click_in_hud() ) return;
		if ( no_left_button_press_in_previous_analyze_mouse_click )
		{
			Me . mouse_move_target_combo_action_type = NO_COMBO_ACTION_SET;

			if ( (tmp = closed_chest_below_mouse_cursor ( )) != -1 )
			{
				check_for_chests_to_open ( tmp ) ;
				if (Me . mouse_move_target_combo_action_type != NO_COMBO_ACTION_SET)
					wait_mouseleft_release = TRUE;
				break;
			}

			// Nota : If 'A' key is held down, we don't directly smash a barrel.
			// This will possibly be the result of the character's attack.
			if ( !APressed() && ( tmp = smashable_barrel_below_mouse_cursor ( ) ) != -1 )
			{ 
				check_for_barrels_to_smash ( tmp );
				if (Me . mouse_move_target_combo_action_type != NO_COMBO_ACTION_SET)
					wait_mouseleft_release = TRUE;
				break;
			}

			if (( tmp = get_floor_item_index_under_mouse_cursor () ) != -1 )
			{
				check_for_items_to_pickup ( tmp );
				if (Me . mouse_move_target_combo_action_type != NO_COMBO_ACTION_SET)
					wait_mouseleft_release = TRUE;
				break;
			}
		}

		// Just after the beginning of a combo action, and while LMB is
		// always pressed, mouse_move_target must not be changed (so that
		// the player's character will actually move to the combo action's target)
		
		if ( !wait_mouseleft_release ) check_for_droids_to_attack_or_talk_with ( ) ;
		
		break;

	case GLOBAL_INGAME_MODE_EXAMINE:
		handle_player_examine_command ( ) ;
		break;

	default:
		DebugPrintf ( -4 , "\n%s(): global_ingame_mode: %d." , __FUNCTION__ , 
				global_ingame_mode );
		ErrorMessage ( __FUNCTION__  , 
				"Illegal global ingame mode encountered!" ,
				PLEASE_INFORM, IS_FATAL );
		break;
	}
    
}; // void AnalyzePlayersMouseClick ( )

#undef _influ_c
