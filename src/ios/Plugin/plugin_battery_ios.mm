//
//  plugin_battery_ios.mm
//  TemplateApp
//
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
//

#import <UIKit/UIKit.h>

#include <CoronaRuntime.h>
#include "plugin_battery_ios.h"

@implementation Battery

- (void)batteryLevelChanged:(NSNotification *)notification
{
	fLibrary->batteryStateChanged();
}

- (void)batteryStateChanged:(NSNotification *)notification
{
	fLibrary->batteryStateChanged();
}

-(id)initWithLibrary: (Corona::plugin_battery_ios*) library;
{
	self = [super init];
	
	// keep
	fLibrary = library;
	
	// Register for battery level and state change notifications.
	[[NSNotificationCenter defaultCenter] addObserver:self
																					 selector:@selector(batteryLevelChanged:)
																							 name:UIDeviceBatteryLevelDidChangeNotification object:nil];
	
	[[NSNotificationCenter defaultCenter] addObserver:self
																					 selector:@selector(batteryStateChanged:)
																							 name:UIDeviceBatteryStateDidChangeNotification object:nil];
	
	// The UI will be updated as a result of the first UIDeviceBatteryStateDidChangeNotification notification.
	// Note that enabling monitoring only triggers a UIDeviceBatteryStateDidChangeNotification;
	// a UIDeviceBatteryLevelDidChangeNotification is not sent.
	[UIDevice currentDevice].batteryMonitoringEnabled = YES;
	
	return self;
	
}

-(void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceBatteryLevelDidChangeNotification object:nil];
	[[NSNotificationCenter defaultCenter] removeObserver:self name:UIDeviceBatteryStateDidChangeNotification object:nil];
	
	[super dealloc];
}

@end

//
// plugin_battery_ios
//

namespace Corona
{
	
	plugin_battery_ios::plugin_battery_ios(lua_State* L)
	: plugin_batteryState(L)
	,	fBattery( NULL )
	{
		fBattery = [[Battery alloc] initWithLibrary: this];
	}
	
	plugin_battery_ios::~plugin_battery_ios()
	{
		[fBattery release];
	}
	
	void plugin_battery_ios::batteryStateChanged()
	{
		if (fListener != nil)
		{
			// Create event and add message to it
			CoronaLuaNewEvent( fL, kEvent );
			pushState(fL);
			
			// Dispatch event to library's listener
			CoronaLuaDispatchEvent( fL, fListener, 0 );
		}
	}
	
	bool plugin_battery_ios::Initialize( CoronaLuaRef listener )
	{
		[UIDevice currentDevice].batteryMonitoringEnabled = listener != nil;
		return plugin_batteryState::Initialize(listener);
	}
	
	float plugin_battery_ios::getLevel() const
	{
		return [UIDevice currentDevice].batteryLevel;
	}
	
	bool plugin_battery_ios::getIsError() const
	{
		return ([UIDevice currentDevice].batteryState) == UIDeviceBatteryStateUnknown;
	}
	
	const char* plugin_battery_ios::getStatus() const
	{
		switch ([UIDevice currentDevice].batteryState)
		{
			case UIDeviceBatteryStateUnplugged:
				return "unplugged";
			case UIDeviceBatteryStateCharging:
				return "charging";
			case UIDeviceBatteryStateFull:
				return "full";
			default:
				return "unknown";
		}
		return "unknown";
	}
	
	// factory
	plugin_batteryState* create_battery_plugin(lua_State *L)
	{
		return new plugin_battery_ios(L);
	}
	
}

