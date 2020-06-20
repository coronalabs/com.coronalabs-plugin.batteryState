// ----------------------------------------------------------------------------
// 
// plugin_batteryState.h
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#ifndef _PLUGIN_BATTERY_MAC_H__
#define _PLUGIN_BATTERY_MAC_H__

#include "CoronaLua.h"
#include "CoronaMacros.h"
#include "shared/plugin_batteryState.h"

namespace Corona
{

	class plugin_battery_mac : public plugin_batteryState
	{
	public:

		enum BATTERY_STATE
		{
			UNKNOWN = 0,
			CHARGING,
			FULL,
			UNPLUGGED
		};
		
		plugin_battery_mac(lua_State *L);
		virtual ~plugin_battery_mac();

		virtual float getLevel() const override;
		virtual bool getIsError() const override;
		virtual const char* getStatus() const override;
		
		virtual void onEnterFrame(lua_State *L) override;
		virtual int	init(lua_State *L);

	private:

		void getBatteryState(int& level, BATTERY_STATE& state, bool& error);

		int fLevel;
		BATTERY_STATE fState;
		bool fError;
	};

} // namespace Corona

#endif // _PLUGIN_BATTERY_STATE_H__
