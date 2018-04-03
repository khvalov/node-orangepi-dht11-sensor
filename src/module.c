#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <node_api.h>

#define MAX_TIMINGS 85
#define nullptr ((void*)0)
static int data[5] = { 0, 0, 0, 0, 0 };
int dht_pin = -1;

void read_dht_data(void)
{
    uint8_t laststate    = HIGH;
    uint8_t counter      = 0;
    uint8_t j            = 0, i;
 
    data[0] = data[1] = data[2] = data[3] = data[4] = 0;
	
	if (dht_pin == -1) {
		printf( "Invalid pin number\n" );
		exit(1);
    }
 
    if ( wiringPiSetup() == -1 )
        exit( 1 );
 
    /* pull pin down for 18 milliseconds */
    pinMode( dht_pin, OUTPUT );
    digitalWrite( dht_pin, LOW );
    delay( 25 );
 
    /* prepare to read the pin */
    pinMode( dht_pin, INPUT );
    /* detect change and read data */
    for ( i = 0; i < MAX_TIMINGS; i++ )
    {
        counter = 0;
        while ( digitalRead( dht_pin ) == laststate )
        {
            counter++;
            delayMicroseconds( 2 );
            if ( counter == 255 )
            {
                break;
            }
        }
        laststate = digitalRead( dht_pin );

        if ( counter == 255 )
            break;
 
        /* ignore first 3 transitions */
        if ( (i >= 4) && (i % 2 == 0) )
        {
            /* shove each bit into the storage bytes */
            data[j / 8] <<= 1;
            if ( counter > 16 )
                data[j / 8] |= 1;
            j++;
        }
    }
 
    /*
     * check we read 40 bits (8bit x 5 ) + verify checksum in the last byte
     * print it out if data is good
     */
 
    if ( (j >= 40) &&
         (data[4] == ( (data[0] + data[1] + data[2] + data[3]) & 0xFF) ) )
    {
        float h = (float)((data[0] << 8) + data[1]) / 10;
        if ( h > 100 )
        {
            h = data[0];    // for DHT11
        }
        float c = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
        if ( c > 125 )
        {
            c = data[2];    // for DHT11
        }
        if ( data[2] & 0x80 )
        {
            c = -c;
        }
		data[2]=c;
        //float f = c * 1.8f + 32;
    }
}

napi_value mySetPin(napi_env env, napi_callback_info info) {
  napi_status status;

  size_t argc = 1;
  napi_value args[2];
  status = napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }

  if (argc < 1) {
    napi_throw_type_error(env, nullptr, "Wrong number of arguments");
    return nullptr;
  }

  napi_valuetype valuetype0;
  status = napi_typeof(env, args[0], &valuetype0);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }

  if (valuetype0 != napi_number) {
    napi_throw_type_error(env, nullptr, "Wrong arguments");
    return nullptr;
  }

  int value0;
  status = napi_get_value_int32(env, args[0], &value0);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }
  
  dht_pin=value0;
  read_dht_data(); 
	
  return;
}

napi_value myGetTemp(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value myNumber;

  status = napi_create_int32(env, data[2], &myNumber);

  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value");
  }

  return myNumber;
}

napi_value myGetHumidity(napi_env env, napi_callback_info info) {
  napi_status status;
  napi_value myNumber2;

  status = napi_create_int32(env, data[0], &myNumber2);

  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to create return value");
  }

  return myNumber2;
}


napi_value Init(napi_env env, napi_value exports) {
  napi_status status;
  napi_value fn;
  
   
  status = napi_create_function(env, NULL, 0, mySetPin, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }

  status = napi_set_named_property(env, exports, "setPin", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
  
  

  status = napi_create_function(env, NULL, 0, myGetTemp, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }

  status = napi_set_named_property(env, exports, "getTemp", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
  
  
  status = napi_create_function(env, NULL, 0, myGetHumidity, NULL, &fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to wrap native function");
  }

  status = napi_set_named_property(env, exports, "getHumidity", fn);
  if (status != napi_ok) {
    napi_throw_error(env, NULL, "Unable to populate exports");
  }
 

  return exports;
}


NAPI_MODULE(NODE_GYP_MODULE_NAME, Init)