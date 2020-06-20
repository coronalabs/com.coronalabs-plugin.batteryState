// ----------------------------------------------------------------------------
// 
// plugin_batteryState
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

//
//	base class for battery plugin
//

#ifndef _PLUGIN_BATTERY_STATE_H__
#define _PLUGIN_BATTERY_STATE_H__

#include "CoronaLua.h"
#include "CoronaMacros.h"
//#include "asyncQueue.h"

// This corresponds to the name of the library, e.g. [Lua] require "plugin.batteryState"
// where the '.' is replaced with '_'
CORONA_EXPORT int luaopen_plugin_batteryState(lua_State *L);

namespace Corona
{

	class plugin_batteryState
	{
	public:

		plugin_batteryState(lua_State *L);
		virtual ~plugin_batteryState();

		typedef plugin_batteryState Self;
		static const char kName[];
		static const char kEvent[];

		// from Lua 
		static int open(lua_State *L);
		static int Finalizer(lua_State *L);
		static Self *ToLibrary(lua_State *L);
		static int getStateMethod(lua_State *L);
		static int initMethod(lua_State *L);
		static int ProcessFrame(lua_State *L);

	protected:
	
		virtual int getState(lua_State *L);
		virtual int init(lua_State *L);

		virtual bool Initialize(CoronaLuaRef listener);
		virtual float getLevel() const = 0;
		virtual bool getIsError() const = 0;
		virtual const char* getStatus() const = 0;
		virtual void onEnterFrame(lua_State *L) {};

		void pushState(lua_State *L);
		void attachOnEnterFrame(lua_State *L);
		void detachOnEnterFrame(lua_State *L);

		CoronaLuaRef fListener;
		lua_State* fL;
		int fCallbackRef;
	};

	// factory
	plugin_batteryState* create_battery_plugin(lua_State *L);

} // namespace Corona

#endif // _PLUGIN_BATTERY_STATE_H__
