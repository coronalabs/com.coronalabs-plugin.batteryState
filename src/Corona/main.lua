local battery = require "plugin.batteryState"

display.setDefault( "anchorX", 0 )
display.setDefault( "background", 0.4 )

local title = display.newText( "Battery Status:", 0, 80, nil, 20)
local level = display.newText( "", 20, 120, nil, 20)
local state = display.newText( "", 20, 150, nil, 20)
local iserror = display.newText( "", 20, 180, nil, 20)

local function displayState( event )
	level.text = "Level: " .. tostring(event.level)
	state.text = "State: " .. tostring(event.state)
	iserror.text = "isError: " .. tostring(event.isError)
end

local function listener( event )
	displayState(event)
	print("battery event" .. require("json").prettify(event) )
end

battery.init( listener )
local powerState = battery.getState()
displayState(powerState)

