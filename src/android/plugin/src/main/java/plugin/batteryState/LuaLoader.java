//
//  LuaLoader.java
//  TemplateApp
//
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

// This corresponds to the name of the Lua library,
// e.g. [Lua] require "plugin.library"
package plugin.batteryState;

import android.util.Log;
import android.webkit.WebView;
import android.webkit.WebViewClient;

import com.ansca.corona.CoronaActivity;
import com.ansca.corona.CoronaEnvironment;
import com.ansca.corona.CoronaLua;
import com.ansca.corona.CoronaRuntime;
import com.ansca.corona.CoronaRuntimeListener;
import com.ansca.corona.CoronaRuntimeTask;
import com.ansca.corona.CoronaRuntimeTaskDispatcher;
import com.naef.jnlua.JavaFunction;
import com.naef.jnlua.LuaState;
import com.naef.jnlua.NamedJavaFunction;

import android.content.Intent;
import android.content.IntentFilter;
import android.content.Context;
import android.app.IntentService;
import android.content.BroadcastReceiver;
import android.os.BatteryManager;

@SuppressWarnings("WeakerAccess")
public class LuaLoader implements JavaFunction, CoronaRuntimeListener {

	private BroadcastReceiver fBroadcastReceiver;

	/** Lua registry ID to the Lua function to be called when the ad request finishes. */
	private int fListener;

	/** This corresponds to the event name, e.g. [Lua] event.name */
	private static final String EVENT_NAME = "batteryStateEvent";

	@SuppressWarnings("unused")
	public LuaLoader() {

		fBroadcastReceiver = null;

		// Initialize member variables.
		fListener = CoronaLua.REFNIL;

		// Set up this plugin to listen for Corona runtime events to be received by methods
		// onLoaded(), onStarted(), onSuspended(), onResumed(), and onExiting().
		CoronaEnvironment.addRuntimeListener(this);
	}

	@Override
	public int invoke(LuaState L) {
		// Register this plugin into Lua with the following functions.
		NamedJavaFunction[] luaFunctions = new NamedJavaFunction[] {
			new initWrapper(),
			new getStateWrapper(),
		};
		String libName = L.toString( 1 );
		L.register(libName, luaFunctions);

		// Returning 1 indicates that the Lua require() function will return the above Lua library.
		return 1;
	}

	@Override
	public void onLoaded(CoronaRuntime runtime) {
	}

	@Override
	public void onStarted(CoronaRuntime runtime) {
	}

	@Override
	public void onSuspended(CoronaRuntime runtime) {
	}

	@Override
	public void onResumed(CoronaRuntime runtime) {
	}

	@Override
	public void onExiting(CoronaRuntime runtime) {

		clear(runtime.getLuaState());

	}

	private void clear(LuaState L) {

		CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity != null && fBroadcastReceiver != null) {
			activity.unregisterReceiver(fBroadcastReceiver);
			fBroadcastReceiver = null;
		}

		if (fListener != CoronaLua.REFNIL) {
			// Remove the Lua listener reference.
			CoronaLua.deleteRef(L, fListener);
			fListener = CoronaLua.REFNIL;
		}
	}

	@SuppressWarnings("unused")
	public void dispatchEvent(final String message, final LuaState luaState) {

		CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			return;
		}

		CoronaRuntimeTaskDispatcher rt = new CoronaRuntimeTaskDispatcher(luaState);
		if (rt == null) {
			return;
		}

		rt.send( new CoronaRuntimeTask() {
			@Override
			public void executeUsing(CoronaRuntime runtime)
			{
				LuaState L = runtime.getLuaState();

				CoronaLua.newEvent( L, EVENT_NAME );
                pushLevel( L );
                pushState( L );
				pushIsError (L );

				try
				{
					CoronaLua.dispatchEvent( L, fListener, 0 );
				}
				catch (Exception ignored)
				{
				}
			}
		} );
	}

	@SuppressWarnings("unused")
	private class initWrapper implements NamedJavaFunction {

		@Override
		public String getName() {
			return "init";
		}

		@Override
		public int invoke(LuaState L) {
			return init(L);
		}
	}

	@SuppressWarnings({"WeakerAccess", "SameReturnValue"})
	public int init(LuaState L) {

		CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity == null) {
			return 0;
		}

		// remome old listener & unregister old reciever
		clear(L);

		int listenerIndex = 1;
		if ( CoronaLua.isListener( L, listenerIndex, EVENT_NAME ) ) {

			fListener = CoronaLua.newRef(L, listenerIndex);

			fBroadcastReceiver = new BroadcastReceiver() {
				@Override
				public void onReceive(Context context, Intent battery) {
					dispatchEvent(EVENT_NAME, L);
				}
			};

			activity.registerReceiver(fBroadcastReceiver, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
		}

		return 0;
	}

	@SuppressWarnings("unused")
	private class getStateWrapper implements NamedJavaFunction {

		@Override
		public String getName() {
			return "getState";
		}

		@Override
		public int invoke(LuaState L) {
			return getState(L);
		}
	}

    private void pushLevel(LuaState L) {
        CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
        if (activity != null) {
			// battery will contain the last-broadcast ACTION_BATTERY_CHANGED Intent, and its extras will contain the battery information
			Intent battery = activity.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));

			int level = battery.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
			int scale = battery.getIntExtra(BatteryManager.EXTRA_SCALE, -1);
			float batteryPct = level / (float) scale;

			L.pushNumber(batteryPct);
			L.setField(-2, "level");
		}
    }


	private void pushState(LuaState L) {
		CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity != null) {
			// battery will contain the last-broadcast ACTION_BATTERY_CHANGED Intent, and its extras will contain the battery information
			Intent battery = activity.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));

			int status = battery.getIntExtra(BatteryManager.EXTRA_STATUS, -1);
			int plugged = battery.getIntExtra(BatteryManager.EXTRA_PLUGGED, -1);
			boolean isPlugged = plugged == BatteryManager.BATTERY_PLUGGED_AC || plugged == BatteryManager.BATTERY_PLUGGED_USB;

			if (isPlugged == false) {
				L.pushString("unplugged");
			} else if (status == BatteryManager.BATTERY_STATUS_CHARGING) {
				L.pushString("charging");
			} else if (status == BatteryManager.BATTERY_STATUS_FULL) {
				L.pushString("full");
			} else {
				L.pushString("unknown");
			}
			L.setField(-2, "state");
		}
	}

	private void pushIsError(LuaState L) {
		boolean error = true;
		CoronaActivity activity = CoronaEnvironment.getCoronaActivity();
		if (activity != null) {
			// battery will contain the last-broadcast ACTION_BATTERY_CHANGED Intent, and its extras will contain the battery information
			Intent battery = activity.registerReceiver(null, new IntentFilter(Intent.ACTION_BATTERY_CHANGED));
			int level = battery.getIntExtra(BatteryManager.EXTRA_LEVEL, -1);
			error = level == -1;
		}
		L.pushBoolean(error);
		L.setField(-2, "isError");
	}

    @SuppressWarnings({"WeakerAccess", "SameReturnValue"})
	public int getState(LuaState L) {

		L.newTable(0, 3); // 3 entries
		pushLevel(L);
		pushState(L);
		pushIsError(L);

		return 1;
	}

}
