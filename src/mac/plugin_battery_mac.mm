// ----------------------------------------------------------------------------
// 
// plugin_battery_mac.cpp
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
// 
// ----------------------------------------------------------------------------

#include <stdlib.h>
#include <assert.h>

#import <Foundation/Foundation.h>
#import <IOKit/ps/IOPowerSources.h>
#import <IOKit/ps/IOPSKeys.h>

#include "plugin_battery_mac.h"

namespace Corona
{

	plugin_battery_mac::plugin_battery_mac(lua_State *L)
		: plugin_batteryState(L)
		, fLevel(-1)
		, fState(UNKNOWN)
		, fError(true)
	{
	}

	plugin_battery_mac::~plugin_battery_mac()
	{
	}
	
	float plugin_battery_mac::getLevel() const
	{
		return fLevel;
	}
	
	bool plugin_battery_mac::getIsError() const
	{
		return fError;
	}
	
	const char* plugin_battery_mac::getStatus() const
	{
		switch (fState)
		{
			case BATTERY_STATE::UNPLUGGED:
				return "unplugged";
			case BATTERY_STATE::CHARGING:
				return "charging";
			case BATTERY_STATE::FULL:
				return "full";
			default:
				return "unknown";
		}
		return "unknown";
	}
	
	void plugin_battery_mac::getBatteryState(int& level, BATTERY_STATE& state, bool& error)
	{
		level = -1;
		state = UNKNOWN;

		CFTypeRef blob = IOPSCopyPowerSourcesInfo();
		CFArrayRef sources = IOPSCopyPowerSourcesList(blob);
		
		CFDictionaryRef pSource = NULL;
		const void *psValue;
		
		if (CFArrayGetCount(sources) == 0)
		{
			error = true;
			return;	// Could not retrieve battery information.  System may not have a battery.
		}
		
		int index = 0;		// hack
		pSource = IOPSGetPowerSourceDescription(blob, CFArrayGetValueAtIndex(sources, index));
		psValue = (CFStringRef)CFDictionaryGetValue(pSource, CFSTR(kIOPSNameKey));
		
		int isCharging = 0;
		psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSIsChargingKey));
		CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &isCharging);
		
		int curCapacity = 0;
		int maxCapacity = 0;
		psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSCurrentCapacityKey));
		CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &curCapacity);
		psValue = CFDictionaryGetValue(pSource, CFSTR(kIOPSMaxCapacityKey));
		CFNumberGetValue((CFNumberRef)psValue, kCFNumberSInt32Type, &maxCapacity);
		
		level = (float)curCapacity / (float)maxCapacity;
		
		state = UNPLUGGED;
		if (isCharging == 1)
		{
			state = curCapacity == maxCapacity ? FULL : CHARGING;
		}

		error = false;
	}

	int	plugin_battery_mac::init(lua_State *L)
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
	
	void plugin_battery_mac::onEnterFrame(lua_State *L)
	{
		int level;
		BATTERY_STATE state;
		bool error;
		getBatteryState(level, state, error);
		
		if (level == fLevel && state == fState && error == fError)
		{
			return;
		}
		
		fLevel = level;
		fState = state;
		fError = error;
		
		if (fListener != NULL)
		{
			// Create event and add message to it
			CoronaLuaNewEvent(fL, kEvent);
			pushState(fL);
			
			// Dispatch event to library's listener
			CoronaLuaDispatchEvent(fL, fListener, 0);
		}
	}

	// factory
	plugin_batteryState* create_battery_plugin(lua_State *L)
	{
		return new plugin_battery_mac(L);
	}


} // namespace Corona

