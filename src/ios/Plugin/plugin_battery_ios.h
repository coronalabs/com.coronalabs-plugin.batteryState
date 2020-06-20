//
//  BatteryLibrary.h
//  TemplateApp
//
// Copyright (c) 2017 Corona Labs Inc. All rights reserved.
//

#ifndef _BatteryLibrary_H__
#define _BatteryLibrary_H__

#import <UIKit/UIKit.h>

#include <CoronaLua.h>
#include <CoronaMacros.h>
#include "shared/plugin_batteryState.h"

namespace Corona
{
	class plugin_battery_ios;
}

@interface Battery : NSObject
{
	@private
	Corona::plugin_battery_ios* fLibrary;
}

-(void)batteryStateChanged:(NSNotification*)note;

@end

namespace Corona
{
	
class plugin_battery_ios : public plugin_batteryState
{
	
public:

	plugin_battery_ios(lua_State* L);
	virtual ~plugin_battery_ios();
	
	void batteryLevelChanged();
	void batteryStateChanged();

	virtual float getLevel() const override;
	virtual bool getIsError() const override;
	virtual const char* getStatus() const override;
	virtual bool Initialize(CoronaLuaRef listener) override;
	
	
private:

	
	//virtual void onEnterFrame(lua_State *L) override;
	Battery *fBattery;
};
	
}

#endif // _BatteryLibrary_H__
