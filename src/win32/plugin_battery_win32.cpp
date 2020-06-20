// ----------------------------------------------------------------------------
// 
// plugin_batteryState
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <assert.h>

#include "CoronaLibrary.h"
#include "CoronaLua.h"

#include "plugin_battery_win32.h"

namespace Corona
{

	plugin_battery_win32::plugin_battery_win32(lua_State *L)
		: plugin_batteryState(L)
	{
		fStatus = {};
	}

	plugin_battery_win32::~plugin_battery_win32()
	{
	}

	int	plugin_battery_win32::init(lua_State *L)
	{
		int rc = plugin_batteryState::init(L);
		if (fListener == NULL)
		{
			detachOnEnterFrame(L);
		}
		else
		{
			attachOnEnterFrame(L);
		}
		return rc;
	}

	int plugin_battery_win32::getState(lua_State *L)
	{
		GetSystemPowerStatus(&fStatus);
		return plugin_batteryState::getState(L);
	}


	float plugin_battery_win32::getLevel() const
	{
		float level = -1;
		const BYTE& b = fStatus.BatteryLifePercent;

		// BatteryLifePercent = percentage of full battery charge remaining. This member can be a value in the range 0 to 100, or 255 if status is unknown
		if (b >= 0 && b <= 100)
		{
			level = b / 100.0f;
		}
		return level;
	}

	bool plugin_battery_win32::getIsError() const
	{
		DWORD rc = GetLastError();
		return rc != 0;
	}

	const char* plugin_battery_win32::getStatus() const
	{
		const BYTE& f = fStatus.BatteryFlag;

		if (f & 0x08)
		{
			return "charging";
		}
		else
		if (fStatus.BatteryLifePercent == 100)
		{
			return "full";
		}
		else
		if (fStatus.ACLineStatus != 1)
		{
			return "unplugged";
		}
		return "unknown";
	}

	void	 plugin_battery_win32::onEnterFrame(lua_State *L)
	{
		SYSTEM_POWER_STATUS status = {};
		BOOL success = GetSystemPowerStatus(&status);
		if (memcmp(&status, &fStatus, sizeof(SYSTEM_POWER_STATUS)) == 0)
		{
			return;
		}

		fStatus = status;

		if (fListener != NULL)
		{
			// Create event and add message to it
			CoronaLuaNewEvent(fL, kEvent);
			pushState(fL);

			// Dispatch event to library's listener
			CoronaLuaDispatchEvent(fL, fListener, 0);
		}
	}

	// Factory.
	plugin_batteryState* create_battery_plugin(lua_State *L)
	{
		return new plugin_battery_win32(L);
	}

}

