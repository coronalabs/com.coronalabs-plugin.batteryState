// ----------------------------------------------------------------------------
// 
// plugin_batteryState win32
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#ifndef _PLUGIN_BATTERY_WIN32_H__
#define _PLUGIN_BATTERY_WIN32_H__

#include <Windows.h>
#include <Powrprof.h>

#include "CoronaLua.h"
#include "CoronaMacros.h"
//#include "asyncQueue.h"
#include "plugin_batteryState.h"

namespace Corona
{

	struct plugin_battery_win32 : public plugin_batteryState
	{

		plugin_battery_win32(lua_State *L);
		virtual ~plugin_battery_win32();

		virtual float getLevel() const override;
		virtual bool getIsError() const override;
		virtual const char* getStatus() const override;

		virtual void onEnterFrame(lua_State *L);
		virtual int	init(lua_State *L);
		virtual int getState(lua_State *L);

	private:

		SYSTEM_POWER_STATUS fStatus;
	};

} // namespace Corona

#endif // _PLUGIN_BATTERY_WIN32_H__
