/* 
 *
 *   Copyright (c) 2003 Johannes Prix
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

#ifndef _itemdefs_h
#define _itemdefs_h

//--------------------
// At first we give here all the definitions of (regular) items
// appearing in the game.
//

#define ITEM_SMALL_HEALTH_POTION 1
#define ITEM_SMALL_MANA_POTION 41
#define ITEM_MEDIUM_HEALTH_POTION 48
#define ITEM_MEDIUM_MANA_POTION 49
#define ITEM_FULL_HEALTH_POTION 2
#define ITEM_FULL_MANA_POTION 42
#define ITEM_RUNNING_POWER_POTION 57
#define ITEM_TEMP_STRENGTH_POTION 92
#define ITEM_TEMP_DEXTERITY_POTION 93

#define ITEM_VMX_GAS_GRENADE 69
#define ITEM_EMP_SHOCK_GRENADE 70
#define ITEM_PLASMA_GRENADE 72

#define ITEM_LASER_PISTOL 22
#define ITEM_TWIN_LASER 3
#define ITEM_ADVANCED_SINGLE_LASER 115
#define ITEM_LASER_RIFLE 12

#define ITEM_PLASMA_PISTOL 23

#define ITEM_EXTERMINATOR 13

#define ITEM_ANTIGRAV_POD 28

#define ITEM_SHOES 5
#define ITEM_BOOTS 11
#define ITEM_LIGHT_BOOTS 94
#define ITEM_REINFORCED_BOOTS 95
#define ITEM_HIKING_BOOTS 96

#define ITEM_BUCKLER 19
#define ITEM_SMALL_SHIELD 43
#define ITEM_STANDARD_SHIELD 33
#define ITEM_MEDIUM_SHIELD 97
#define ITEM_LARGE_SHIELD 56
/*
#define ITEM_MEDIUM_SHIELD 0
#define ITEM_ROUND_SHIELD 0
#define ITEM_KITE_SHIELD 0
#define ITEM_DOOR_SHIELD 0
#define ITEM_TOWER_SHIELD 0
*/

#define ITEM_ARMOR_SIMPLE_JACKET 25
#define ITEM_ARMOR_REINFORCED_JACKET 29
#define ITEM_ARMOR_PROTECTIVE_JACKET 30
#define ITEM_RED_GUARD_LIGHT_ARMOUR 31
#define ITEM_RED_GUARD_HEAVY_ARMOUR 18

/*Cutting*/
#define ITEM_SHORT_SWORD 130
#define ITEM_LONG_SWORD 131
#define ITEM_GREAT_SWORD 132
//#define ITEM_CHAINSAW 133
#define ITEM_CLAYMORE 133 //temporary entry until IDByName & class dropping is implemented
/*Cleaving*/
#define ITEM_MEAT_CLEAVER 134
//#define ITEM_SMALL_AXE 135
#define ITEM_SCIMITAR 135 //temporary entry until IDByName & class dropping is implemented
//#define ITEM_AXE 136
#define ITEM_FALCHION 136 //temporary entry until IDByName & class dropping is implemented
#define ITEM_LARGE_AXE 137
/*Piercing*/
#define ITEM_DAGGER 138
//#define ITEM_WOODEN_SPEAR 139
#define ITEM_STAFF 139 //temporary entry until IDByName & class dropping is implemented
#define ITEM_METAL_SPEAR 140
/*Impact*/
//#define ITEM_CROWBAR 141
#define ITEM_SABER 141
#define ITEM_IRON_PIPE 142
#define ITEM_GOLF_CLUB 143
//#define ITEM_TREE_BRANCH 144
#define ITEM_CLUB 144 //temporary entry until IDByName & class dropping is implemented
#define ITEM_TABLE_LEG 145
#define ITEM_BASEBALL_BAT 146
#define ITEM_IRON_BAR 147
#define ITEM_MACE 148
#define ITEM_FLAIL 149
#define ITEM_SLEDGEHAMMER 150
/*Energy*/
#define ITEM_LIGHTSABER 151
#define ITEM_LASERSTAFF 152


#define ITEM_CAP 20
#define ITEM_SMALL_HELM 50
#define ITEM_IRON_HAT 53
#define ITEM_IRON_HELM 54
#define ITEM_POTHELM 55

#define ITEM_SPELLBOOK_OF_CHECK_SYSTEM_INTEGRITY 44
#define ITEM_SPELLBOOK_OF_CALCULATE_PI 46
#define ITEM_SPELLBOOK_OF_BLUE_SCREEN 47
#define ITEM_SPELLBOOK_OF_TELEPORT_HOME 45

#define ITEM_SPELLBOOK_OF_MALFORMED_PACKET 83
#define ITEM_SPELLBOOK_OF_ANALYZE_ITEM 84
#define ITEM_SPELLBOOK_OF_NETHACK 85
#define ITEM_SPELLBOOK_OF_DISPELL_SMOKE 86
#define ITEM_SPELLBOOK_OF_KILLER_POKE 87
#define ITEM_SPELLBOOK_OF_BROADCAST_KILLER_POKE 88
#define ITEM_SPELLBOOK_OF_RICER_CFLAGS 89
#define ITEM_SPELLBOOK_OF_PLASMA_DISCHARGE 90
#define ITEM_SPELLBOOK_OF_INVISIBILITY 91

#define ITEM_SPELLBOOK_OF_SANCTUARY 122 
#define ITEM_SPELLBOOK_OF_VIRUS 123
#define ITEM_SPELLBOOK_OF_BROADCAST_VIRUS 124 
#define ITEM_SPELLBOOK_OF_REVERSE_ENGINEER 125
#define ITEM_SPELLBOOK_OF_EMERGENCY_SHUTDOWN 126
#define ITEM_SPELLBOOK_OF_LIGHT 127
#define ITEM_SPELLBOOK_OF_SATELLITE_IMAGE 128



#define ITEM_SCRIPT_OF_IDENTIFY 98
#define ITEM_SCRIPT_OF_TELEPORT_HOME 99

/*#define ITEM_START_PLUGIN_WARRIOR 21
#define ITEM_START_PLUGIN_SNIPER 51
#define ITEM_START_PLUGIN_HACKER 52*/

// This is a special item for a mission....
#define ITEM_ENERGETIC_HEART 4
#define ITEM_ULTIMATE_BOOK_OF_FREE_CODE 57
#define ITEM_ULTIMATE_COFFEE_MACHINE 57
#define ITEM_BLUE_ENERGY_DRINK 58
#define ITEM_DIXONS_TOOLBOX 59

#define ITEM_PLASMA_AMMUNITION 66
#define ITEM_LASER_AMMUNITION 67
#define ITEM_EXTERMINATOR_AMMUNITION 68

#define ITEM_RED_DILITIUM_CRYSTAL 74
#define ITEM_MAP_MAKER_SIMPLE 75

#define ITEM_STRENGTH_PILL 76
#define ITEM_DEXTERITY_PILL 77
#define ITEM_MAGIC_PILL 78

#define ITEM_EATING_FORK 80
#define ITEM_EATING_PLATE 81
#define ITEM_DRINKING_MUG 82

//--------------------
// And now we put here all the rectangle definitions and that...
//
// the inventory rect is the rectangle inside the inventory screen,
// where all the items "in the backpack" including the quick inventory
// are displayed.  it's relative to the top left corner, as always.
//
#define INVENTORY_RECT_Y 260
#define INVENTORY_RECT_X 8

#define CURRENT_SKILL_RECT_X 537
#define CURRENT_SKILL_RECT_Y (GameConfig . screen_height - 80 * GameConfig . screen_height / 480)
#define CURRENT_SKILL_RECT_W 64
#define CURRENT_SKILL_RECT_H 64

#define CURRENT_WEAPON_RECT_X 32
#define CURRENT_WEAPON_RECT_Y 400
#define CURRENT_WEAPON_RECT_W 64
#define CURRENT_WEAPON_RECT_H 64

#define WHOLE_HEALTH_RECT_Y (480-85)
#define WHOLE_HEALTH_RECT_W 6
#define WHOLE_HEALTH_RECT_H 50
#define WHOLE_HEALTH_RECT_X 614
#define HEALTH_RECT_COLOR (63*64*64 + 64*8)

#define GOLDEN_SCHNITT ((sqrt(5)-1)/2)

#define WHOLE_RUNNING_POWER_RECT_X 6
#define WHOLE_RUNNING_POWER_RECT_Y (480-85)
#define WHOLE_RUNNING_POWER_RECT_W 6
#define WHOLE_RUNNING_POWER_RECT_H 50

#define WHOLE_EXPERIENCE_COUNTDOWN_RECT_X 19
#define WHOLE_EXPERIENCE_COUNTDOWN_RECT_Y WHOLE_RUNNING_POWER_RECT_Y
#define WHOLE_EXPERIENCE_COUNTDOWN_RECT_W WHOLE_RUNNING_POWER_RECT_W
#define WHOLE_EXPERIENCE_COUNTDOWN_RECT_H WHOLE_RUNNING_POWER_RECT_H

#define WHOLE_FORCE_RECT_Y ( WHOLE_HEALTH_RECT_Y )
#define WHOLE_FORCE_RECT_W ( WHOLE_HEALTH_RECT_W )
#define WHOLE_FORCE_RECT_H ( WHOLE_HEALTH_RECT_H )
#define WHOLE_FORCE_RECT_X ( 640-12)
#define FORCE_RECT_COLOR ( 0 * 64 * 64 + 64 * 0 + 63 )

//--------------------
// The weapon rectangle is of course for the weapon the Tux is
// using.  When using a 2-handed weapon, the 'shield' rectangle
// will ALSO be used to show this weapon, since the shield-hand
// is also needed to wield such a thing.
//
#define WEAPON_RECT_WIDTH 67
#define WEAPON_RECT_HEIGHT 112
#define WEAPON_RECT_X 25
#define WEAPON_RECT_Y 117

//--------------------
// The armour rectangle is where you can place some armour
// for the body of the Tux.
//
#define ARMOUR_RECT_WIDTH 66
#define ARMOUR_RECT_HEIGHT 92
#define ARMOUR_RECT_X 130
#define ARMOUR_RECT_Y 70

//--------------------
// The shield rectangle is obviously for the shields, but not only
// that:  when using a 2-handed weapon, its also good for the weapon
// again, to indicate, that no shield can be used in conjunction with
// such a 2-handed weapon.
//
#define SHIELD_RECT_WIDTH 70
#define SHIELD_RECT_HEIGHT 112
#define SHIELD_RECT_X 230
#define SHIELD_RECT_Y 116

//--------------------
// The 'drive' rectangle is actually for the footwear.
//
#define DRIVE_RECT_WIDTH 64
#define DRIVE_RECT_HEIGHT 64
#define DRIVE_RECT_X 130
#define DRIVE_RECT_Y 175

//--------------------
// Now this is the 'helmet' rectangle.
//
#define HELMET_RECT_WIDTH 64
#define HELMET_RECT_HEIGHT 64
#define HELMET_RECT_X 25
#define HELMET_RECT_Y 22

#endif
// Nothing more from here on 
