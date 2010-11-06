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

#define _mission_c

#include "system.h"

#include "defs.h"
#include "struct.h"
#include "global.h"
#include "map.h"
#include "proto.h"

/**
 * This function is responsible for making a new quest diary entry 
 * visible inside the quest browser.
 */
void mission_diary_add(const char *mis_name, const char *diarytext)
{
	int mis_num = GetMissionIndexByName(mis_name);
	int idx = 0;

	while (Me.AllMissions[mis_num].mission_diary_texts[idx] && idx < MAX_MISSION_DESCRIPTION_TEXTS)
		idx++;

	if (idx >= MAX_MISSION_DESCRIPTION_TEXTS) {
		ErrorMessage(__FUNCTION__,
			     "There is no more room to append mission diary text \"%s\" to mission name \"%s\". Doing nothing.\n",
			     PLEASE_INFORM, IS_WARNING_ONLY, diarytext, mis_name);
		return;
	}

	Mission_Status_Change_Sound();

	Me.AllMissions[mis_num].mission_description_time[idx] = Me.current_game_date;

	Me.AllMissions[mis_num].mission_diary_texts[idx] = strdup(diarytext);

	Me.quest_browser_changed = 1;
};

/*----------------------------------------------------------------------
 * This function checks, if the influencer has succeeded in his given 
 * mission.  If not it returns, if yes the EndTitle/Debriefing is
 * started.
 ----------------------------------------------------------------------*/
void CheckIfMissionIsComplete(void)
{
	int ItemCounter;
	int mis_num;
	static int CheckMissionGrid;
	int this_mission_seems_completed;
	int checked_one_criterion;

#define MIS_COMPLETE_DEBUG 1

	// We do not need to check for mission completed EVERY frame
	// It will be enough to do it now and then..., e.g. every 50th frame
	//
	CheckMissionGrid++;
	if ((CheckMissionGrid % 50) != 0)
		return;

	for (mis_num = 0; mis_num < MAX_MISSIONS_IN_GAME; mis_num++) {
		// We need not do anything, if the mission has already failed or if
		// the mission is already completed or if the mission does not exist
		// at all or if the mission was not assigned yet
		//
		if (Me.AllMissions[mis_num].MissionIsComplete == TRUE)
			continue;
		if (Me.AllMissions[mis_num].MissionWasFailed == TRUE)
			continue;
		if (Me.AllMissions[mis_num].MissionExistsAtAll != TRUE)
			continue;
		if (Me.AllMissions[mis_num].MissionWasAssigned != TRUE)
			continue;

		DebugPrintf(MIS_COMPLETE_DEBUG, "\nSomething was assigned at all..... mis_num = %d ", mis_num);

		this_mission_seems_completed = TRUE;
		checked_one_criterion = FALSE;

		// Continue if the Mission target KillOne is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].KillOne != (-1)) {
			enemy *erot;
			BROWSE_ALIVE_BOTS(erot) {
				if ((erot->marker == Me.AllMissions[mis_num].KillOne)) {
					DebugPrintf(MIS_COMPLETE_DEBUG,
						    "\nOne of the marked droids is still alive... (0x%08x at %f:%f on %d)\n", erot,
						    erot->pos.x, erot->pos.y, erot->pos.z);
					this_mission_seems_completed = FALSE;
					break;
				}

			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target fetch_item is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].fetch_item != (-1)) {

			for (ItemCounter = 0; ItemCounter < MAX_ITEMS_IN_INVENTORY; ItemCounter++) {
				if (Me.Inventory[ItemCounter].type == Me.AllMissions[mis_num].fetch_item) {
					DebugPrintf(MIS_COMPLETE_DEBUG, "\nDesired item IS PRESENT!!");
					break;
				}
			}
			if (ItemCounter >= MAX_ITEMS_IN_INVENTORY) {
				// goto CheckNextMission;
				this_mission_seems_completed = FALSE;
			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target must_clear_first_level is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].must_clear_first_level != (-1)) {
			enemy *erot;
			BROWSE_LEVEL_BOTS(erot, Me.AllMissions[mis_num].must_clear_first_level) {
				if (!is_friendly(erot->faction, FACTION_SELF)) {
					this_mission_seems_completed = FALSE;
					break;
				}
			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target must_clear_second_level is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].must_clear_second_level != (-1)) {
			enemy *erot;
			BROWSE_LEVEL_BOTS(erot, Me.AllMissions[mis_num].must_clear_second_level) {
				if (!is_friendly(erot->faction, FACTION_SELF)) {
					this_mission_seems_completed = FALSE;
					break;
				}
			}
			checked_one_criterion = TRUE;
		}

		// Continue if the Mission target MustReachLevel is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].MustReachLevel != (-1)) {
			if (Me.pos.z != Me.AllMissions[mis_num].MustReachLevel) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nLevel number does not match...");
				continue;
			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target MustReachPoint.x is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].MustReachPoint.x != (-1)) {
			if (Me.pos.x != Me.AllMissions[mis_num].MustReachPoint.x) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nX coordinate does not match...");
				continue;
			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target MustReachPoint.y is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].MustReachPoint.y != (-1)) {
			if (Me.pos.y != Me.AllMissions[mis_num].MustReachPoint.y) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nY coordinate does not match...");
				continue;
			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target MustLiveTime is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].MustLiveTime != (-1)) {
			if (Me.MissionTimeElapsed < Me.AllMissions[mis_num].MustLiveTime) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nTime Limit not yet reached...");
				continue;
			}
			checked_one_criterion = TRUE;
		}
		// Continue if the Mission target MustBeOne is given but not fulfilled
		//
		if (Me.AllMissions[mis_num].MustBeOne != (-1)) {
			if (Me.marker != Me.AllMissions[mis_num].MustBeOne) {
				DebugPrintf(MIS_COMPLETE_DEBUG, "\nYou're not yet one of the marked ones...");
				continue;
			}
			checked_one_criterion = TRUE;
		}

		// If the mission actually had criteria we checked for, and those criteria
		// are OK, the mission is finished.
		if (checked_one_criterion && this_mission_seems_completed)
			CompleteMission(Me.AllMissions[mis_num].mission_name);

	}
};				// void CheckIfMissionIsComplete

/**
 * This function assigns a new mission to the player, which means 
 * that the status of the mission in the mission array is changed and
 * perhaps the mission log activated.
 */
void AssignMission(const char *name)
{
	int MissNum = GetMissionIndexByName(name);

	Mission_Status_Change_Sound();
	Me.AllMissions[MissNum].MissionWasAssigned = TRUE;

	if (Me.AllMissions[MissNum].assignment_lua_code)
		run_lua(Me.AllMissions[MissNum].assignment_lua_code);

	Me.quest_browser_changed = 1;
};

/**
 * This function marks a mission as complete
 */
void CompleteMission(const char *name)
{
	int MissNum = GetMissionIndexByName(name);

	Mission_Status_Change_Sound();
	Me.AllMissions[MissNum].MissionIsComplete = TRUE;

	if (Me.AllMissions[MissNum].completion_lua_code)
		run_lua(Me.AllMissions[MissNum].completion_lua_code);

	Me.quest_browser_changed = 1;
}

/**
 * At the start of every new game, the mission info (i.e. which missions
 * are already assigned, completed, failed, available and such) should
 * be reset to default state, so that no zombie mission entries can appear. 
 */
void clear_tux_mission_info()
{
	int MissionTargetIndex;
	int diary_entry_nr;

	// At first we clear out all existing mission entries, so that no 'zombies' remain
	// when the game is restarted and (perhaps less) new missions are loaded.
	//
	for (MissionTargetIndex = 0; MissionTargetIndex < MAX_MISSIONS_IN_GAME; MissionTargetIndex++) {
		Me.AllMissions[MissionTargetIndex].MissionExistsAtAll = FALSE;
		Me.AllMissions[MissionTargetIndex].MissionIsComplete = FALSE;
		Me.AllMissions[MissionTargetIndex].MissionWasFailed = FALSE;
		Me.AllMissions[MissionTargetIndex].MissionWasAssigned = FALSE;

		if (Me.AllMissions[MissionTargetIndex].mission_name) {
			free(Me.AllMissions[MissionTargetIndex].mission_name);
			Me.AllMissions[MissionTargetIndex].mission_name = NULL;
		}

		for (diary_entry_nr = 0; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS; diary_entry_nr++) {
			if (Me.AllMissions[MissionTargetIndex].mission_diary_texts[diary_entry_nr]) {
				free(Me.AllMissions[MissionTargetIndex].mission_diary_texts[diary_entry_nr]);
				Me.AllMissions[MissionTargetIndex].mission_diary_texts[diary_entry_nr] = NULL;
			}

			Me.AllMissions[MissionTargetIndex].mission_description_time[diary_entry_nr] = 0;
		}
	}

};				// void clear_tux_mission_info ( )

/**
 * This function reads the mission specifications from the mission file
 * which is assumed to be loaded into memory already.
 */
void GetQuestList(char *QuestListFilename)
{
	char *EndOfMissionTargetPointer;
	int MissionTargetIndex = 0;
	char *MissionTargetPointer;
	char fpath[2048];
	char *MissionFileContents;
	char InnerPreservedLetter = 0;
	int diary_entry_nr;

#define MISSION_TARGET_SUBSECTION_START_STRING "** Start of this mission target subsection **"
#define MISSION_TARGET_SUBSECTION_END_STRING "** End of this mission target subsection **"

#define MISSION_TARGET_NAME_INITIALIZER "Mission Name=_\""

#define MISSION_TARGET_FETCH_ITEM_STRING "Mission target is to fetch item : \""
#define MISSION_TARGET_KILL_ONE_STRING "Mission target is to kill droids with marker : "
#define MISSION_TARGET_MUST_CLEAR_FIRST_LEVEL "Mission target is to kill all hostile droids this first level : "
#define MISSION_TARGET_MUST_CLEAR_SECOND_LEVEL "Mission target is to also kill all hostile droids on second level : "
#define MISSION_TARGET_MUST_REACH_LEVEL_STRING "Mission target is to reach level : "
#define MISSION_TARGET_MUST_REACH_POINT_X_STRING "Mission target is to reach X-Pos : "
#define MISSION_TARGET_MUST_REACH_POINT_Y_STRING "Mission target is to reach Y-Pos : "
#define MISSION_TARGET_MUST_LIVE_TIME_STRING "Mission target is to live for how many seconds : "
#define MISSION_TARGET_MUST_BE_ONE_STRING "Mission target is to overtake a droid with marker : "

#define MISSION_ASSIGNMENT_LUACODE_STRING "Assignment LuaCode={"
#define MISSION_COMPLETION_LUACODE_STRING "Completion LuaCode={"

	// At first we must load the quest list file given...
	//
	find_file(QuestListFilename, MAP_DIR, fpath, 0);
	MissionFileContents = ReadAndMallocAndTerminateFile(fpath, "*** END OF QUEST LIST *** LEAVE THIS TERMINATOR IN HERE ***");
	MissionTargetPointer = MissionFileContents;

	for (MissionTargetIndex = 0; MissionTargetIndex < MAX_MISSIONS_IN_GAME; MissionTargetIndex++) {
		Me.AllMissions[MissionTargetIndex].MissionExistsAtAll = FALSE;
	}

	MissionTargetIndex = 0;
	while ((MissionTargetPointer = strstr(MissionTargetPointer, MISSION_TARGET_SUBSECTION_START_STRING)) != NULL) {
		EndOfMissionTargetPointer = LocateStringInData(MissionTargetPointer, MISSION_TARGET_SUBSECTION_END_STRING);

		if (MissionTargetIndex >= MAX_MISSIONS_IN_GAME)
			ErrorMessage(__FUNCTION__, "The number of quests specified in %s exceeds MAX_MISSIONS_IN_GAME (%d).\n",
				     PLEASE_INFORM, IS_FATAL, QuestListFilename, MAX_MISSIONS_IN_GAME);

		// We need to add an inner terminator here, so that the strstr operation
		// below will know where to stop within this subsection.
		//
		InnerPreservedLetter = *EndOfMissionTargetPointer;
		*EndOfMissionTargetPointer = 0;

		Me.AllMissions[MissionTargetIndex].MissionExistsAtAll = TRUE;

		Me.AllMissions[MissionTargetIndex].mission_name = ReadAndMallocStringFromData(MissionTargetPointer, MISSION_TARGET_NAME_INITIALIZER, "\"");

		// From here on we read the details of the mission target, i.e. what the
		// influencer has to do, so that the mission can be thought of as completed
		//
		if (strstr(MissionTargetPointer, MISSION_TARGET_FETCH_ITEM_STRING)) {
			char *iname = ReadAndMallocStringFromData(MissionTargetPointer, MISSION_TARGET_FETCH_ITEM_STRING, "\"");
			Me.AllMissions[MissionTargetIndex].fetch_item = GetItemIndexByName(iname);
			free(iname);
		} else
			Me.AllMissions[MissionTargetIndex].fetch_item = -1;

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_KILL_ONE_STRING, "%d", "-1",
				    &Me.AllMissions[MissionTargetIndex].KillOne, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_CLEAR_FIRST_LEVEL, "%d", "-1", 
				    &Me.AllMissions[MissionTargetIndex].must_clear_first_level, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_CLEAR_SECOND_LEVEL, "%d", "-1",
				    &Me.AllMissions[MissionTargetIndex].must_clear_second_level, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_BE_ONE_STRING, "%d", "-1", 
				    &Me.AllMissions[MissionTargetIndex].MustBeOne, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_REACH_POINT_X_STRING, "%d", "-1",
				    &Me.AllMissions[MissionTargetIndex].MustReachPoint.x, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_REACH_POINT_Y_STRING, "%d", "-1", 
				    &Me.AllMissions[MissionTargetIndex].MustReachPoint.y, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_REACH_LEVEL_STRING, "%d", "-1", 
				    &Me.AllMissions[MissionTargetIndex].MustReachLevel, EndOfMissionTargetPointer);

		ReadValueFromStringWithDefault(MissionTargetPointer, MISSION_TARGET_MUST_LIVE_TIME_STRING, "%lf", "-1",
				    &Me.AllMissions[MissionTargetIndex].MustLiveTime, EndOfMissionTargetPointer);

		if (strstr(MissionTargetPointer, MISSION_COMPLETION_LUACODE_STRING)) {
			Me.AllMissions[MissionTargetIndex].completion_lua_code =
			    ReadAndMallocStringFromData(MissionTargetPointer, MISSION_COMPLETION_LUACODE_STRING, "}");
		} else {
			Me.AllMissions[MissionTargetIndex].completion_lua_code = NULL;
		}

		if (strstr(MissionTargetPointer, MISSION_ASSIGNMENT_LUACODE_STRING)) {
			Me.AllMissions[MissionTargetIndex].assignment_lua_code =
			    ReadAndMallocStringFromData(MissionTargetPointer, MISSION_ASSIGNMENT_LUACODE_STRING, "}");
		} else {
			Me.AllMissions[MissionTargetIndex].assignment_lua_code = NULL;
		}
		// Now it is time to read in the mission diary entries, that might
		// be displayed in the quest browser later.
		//
		for (diary_entry_nr = 0; diary_entry_nr < MAX_MISSION_DESCRIPTION_TEXTS; diary_entry_nr++) {
			Me.AllMissions[MissionTargetIndex].mission_diary_texts[diary_entry_nr] = NULL;
		}

		// Now we are done with reading in THIS one mission target
		// We need to advance the MissionTargetPointer, so that we avoid doubly
		// reading in this mission OR ONE OF THIS MISSIONS VALUES!!!!
		// 
		// And we need of course to advance the array index for mission targets too...
		//
		MissionTargetPointer = EndOfMissionTargetPointer;	// to avoid double entering the same target
		MissionTargetIndex++;	// to avoid overwriting the same entry again

		// We restore the termination character we added before, even if that
		// is maybe not really necessary...
		//
		*EndOfMissionTargetPointer = InnerPreservedLetter;

	}			// while mission target found...

	// Finally we record the number of mission targets scanned and are done with this function
	DebugPrintf(1, "\nNUMBER OF MISSION TARGETS FOUND: %d.\n", MissionTargetIndex);
	fflush(stdout);
	free(MissionFileContents);
};				// void Get_Mission_Targets( ... )

/**
 * This function returns the mission that has the specified name.
 */
int GetMissionIndexByName(const char *name)
{
	int cidx;

	for (cidx = 0; cidx < MAX_MISSIONS_IN_GAME; cidx++) {
		const char *n = Me.AllMissions[cidx].mission_name;
		if (!n)
			continue;

		if (!strcmp(n, name))
			return cidx;
	}

	ErrorMessage(__FUNCTION__, "Unable to find mission named %s\n", PLEASE_INFORM, IS_FATAL, name);
	return -1;
}

#undef _mission_c
