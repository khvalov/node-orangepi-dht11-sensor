const addon = require('node-orangepi-dht11-sensor');
var sleep = require('sleep');
addon.setPin(7);

while(true){
	console.log('T:'+addon.getTemp());
	console.log('H:'+addon.getHumidity());
	sleep.sleep(3);
}
