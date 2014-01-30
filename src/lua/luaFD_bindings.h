/*
 *
 *  Copyright (c) 2013 Samuel Degrande
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
 * \file luaFD_bindings.h
 * \brief This file contains the declarations needed to create Lua bindings of
 *  some C structs.
 *
 */

/// \defgroup luaFD_bindings Lua bindings
///
/// Some doc on how to write and use a binding - TO BE WRITTEN

#ifndef _luafd_bindings_h
#define _luafd_bindings_h

#include "luaFD.h"
#include "lauxlib.h"

extern int luaFD_tux_init(lua_State *);
extern int luaFD_tux_get_instance(lua_State *);

/**
 * List of binding initializers (functions creating class-type metatables)
 */
const luaL_Reg luaFD_initializers[] = {
	{ "FDtux", luaFD_tux_init },
	{ NULL, NULL }
};

/**
 * List of functions available in the 'FDrpg' library (instance getters)
 */
const luaL_Reg luaFD_instances[] = {
		{ "get_tux", luaFD_tux_get_instance },
		{ NULL, NULL }
};

#endif /* _luafd_bindings_h */