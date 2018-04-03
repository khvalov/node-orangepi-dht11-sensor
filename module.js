const addon = require('bindings')('module')
var sleep = require('sleep');
addon.setPin(7);

while(true){
	console.log('T:'+addon.getTemp());
	console.log('H:'+addon.getHumidity());
	sleep.sleep(3);
}
