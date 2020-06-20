// ----------------------------------------------------------------------------
// 
// plugin_batteryState
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

//
//	base class for battery plugin
//


#include <stdlib.h>
#include <assert.h>

#include "plugin_batteryState.h"
#include "CoronaLibrary.h"
#include "CoronaLua.h"
//#include "asyncQueue.h"

namespace Corona
{

	// This corresponds to the name of the library, e.g. [Lua] require "plugin.library"
	const char plugin_batteryState::kName[] = "plugin.batteryState";

	// This corresponds to the event name, e.g. [Lua] event.name
	const char plugin_batteryState::kEvent[] = "batteryStateEvent";

	plugin_batteryState::plugin_batteryState(lua_State *L)
		: fListener(NULL)
		, fL(L)
		, fCallbackRef(0)
	{
	}

	plugin_batteryState::~plugin_batteryState()
	{
		detachOnEnterFrame(fL);
		CoronaLuaDeleteRef(fL, fListener);
	}

	int	plugin_batteryState::open(lua_State *L)
	{
		// Register __gc callback
		const char kMetatableName[] = __FILE__; // Globally unique string to prevent collision
		CoronaLuaInitializeGCMetatable(L, kMetatableName, Finalizer);

		// Functions in library
		const luaL_Reg kVTable[] =
		{
			{"init", initMethod},
			{"getState", getStateMethod},

			{NULL, NULL}
		};

		// Set library as upvalue for each library function
		plugin_batteryState *library = create_battery_plugin(L);

		// Store the library singleton in the registry so it persists
		// using kMetatableName as the unique key.
		CoronaLuaPushUserdata(L, library, kMetatableName);
		lua_pushstring(L, kMetatableName);
		lua_settable(L, LUA_REGISTRYINDEX);

		// Leave "library" on top of stack
		// Set library as upvalue for each library function
		int result = CoronaLibraryNew(L, kName, "com.coronalabs", 1, 1, kVTable, library);
		return result;
	}

	int	plugin_batteryState::Finalizer(lua_State *L)
	{
		Self *library = (Self*)CoronaLuaToUserdata(L, 1);
		library->detachOnEnterFrame(L);
		delete library;

		return 0;
	}

	// [Lua] network.init( )	
	int	plugin_batteryState::initMethod(lua_State *L)
	{
		plugin_batteryState* library = ToLibrary(L);
		return library->init(L);
	}

	// [Lua] network.getState( )
	int	plugin_batteryState::getStateMethod(lua_State *L)
	{
		plugin_batteryState* library = ToLibrary(L);
		return library->getState(L);
	}

	int	plugin_batteryState::init(lua_State *L)
	{
		// remove old listener
		CoronaLuaDeleteRef(L, fListener);
		fListener = NULL;
		
		int listenerIndex = 1;
		bool hasListener = CoronaLuaIsListener(L, listenerIndex, kEvent) == 1;
		if (hasListener)
		{
			CoronaLuaRef listener = CoronaLuaNewRef(L, listenerIndex);
			Initialize(listener);
		}
		return 0;
	}

	int	plugin_batteryState::getState(lua_State *L)
	{
		lua_newtable(L);
		pushState(L);
		return 1;
	}

	void plugin_batteryState::pushState(lua_State *L)
	{
		lua_pushnumber(L, getLevel());
		lua_setfield(L, -2, "level");

		lua_pushstring(L, getStatus());
		lua_setfield(L, -2, "state");

		lua_pushboolean(L, getIsError());
		lua_setfield(L, -2, "isError");
	}

	void 	plugin_batteryState::attachOnEnterFrame(lua_State *L)
	{
		if (fCallbackRef == 0)
		{
			// Does the equivalent of the following Lua code:
			//   Runtime:addEventListener( "enterFrame", ProcessFrame )
			// which is equivalent to:
			//   local f = Runtime.addEventListener
			//   f( Runtime, "enterFrame", ProcessFrame )
			//
			// Now invoke above from C:
			// Lua stack order (from lowest index to highest):
			//   f
			//   Runtime
			//   "enterFrame"
			//   ProcessFrame (closure)

			CoronaLuaPushRuntime(L); // push 'Runtime'
			lua_getfield(L, -1, "addEventListener"); // push 'f', i.e. Runtime.addEventListener
			lua_insert(L, -2); // swap so 'f' is below 'Runtime'
			lua_pushstring(L, "enterFrame");

			// Push ProcessFrame as closure so it has access
			lua_pushlightuserdata(L, this);
			lua_pushcclosure(L, &ProcessFrame, 1);

			lua_pushvalue(L, -1);
			fCallbackRef = luaL_ref(L, LUA_REGISTRYINDEX); // r = clo

			CoronaLuaDoCall(L, 3, 0);
		}
	}

	void 	plugin_batteryState::detachOnEnterFrame(lua_State *L)
	{
		if (fCallbackRef != 0)
		{
			//Get rid of closure
			CoronaLuaPushRuntime(L); // push 'Runtime'

			if (lua_type(L, -1) == LUA_TTABLE)
			{
				lua_getfield(L, -1, "removeEventListener"); // push 'f', i.e. Runtime.addEventListener
				lua_insert(L, -2); // swap so 'f' is below 'Runtime'
				lua_pushstring(L, "enterFrame");
				lua_rawgeti(L, LUA_REGISTRYINDEX, fCallbackRef);// pushes closure
				CoronaLuaDoCall(L, 3, 0);
				luaL_unref(L, LUA_REGISTRYINDEX, fCallbackRef);
				fCallbackRef = 0;
			}
			else
			{
				lua_pop(L, 1); // pop nil
			}
		}
	}

	int	plugin_batteryState::ProcessFrame(lua_State *L)
	{
		Self *library = ToLibrary(L);
		assert(library);

		library->onEnterFrame(L);

		return 0;
	}

	plugin_batteryState *	plugin_batteryState::ToLibrary(lua_State *L)
	{
		// library is pushed as part of the closure
		Self *library = (Self *)lua_touserdata(L, lua_upvalueindex(1));
		return library;
	}

	bool plugin_batteryState::Initialize(CoronaLuaRef listener)
	{
		// Can only initialize listener once
		bool result = (NULL == fListener);
		if (result)
		{
			fListener = listener;
		}
		return result;
	}

} // namespace Corona


CORONA_EXPORT int luaopen_plugin_batteryState(lua_State *L)
{
	return Corona::plugin_batteryState::open(L);
}
