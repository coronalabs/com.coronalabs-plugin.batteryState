
if (!window.plugin_batteryState) window.plugin_batteryState = {};

plugin_batteryState.test1 = function(a,b,c)
{

	console.log('********** test1 called',a,b,c, this);
	this.dispatchEvent({ name: 'myevent' })
	return {'aaa':'retcode from ============test1', 'bbb':1237 };
}
plugin_batteryState.test2 = function()
{
	console.log('********** test2 called');
}

console.log('********** batteryState is loaded');