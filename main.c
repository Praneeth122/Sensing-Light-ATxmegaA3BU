#include <stdio.h>

#include <board.h>
#include <led.h>

#include <sensors/sensor.h>


// supply missing C-library constants

#if defined(XMEGA) && defined(__GNUC__)
enum {EXIT_SUCCESS, EXIT_FAILURE};
#endif


// Application configuration constants

#define	LIGHT_SAMPLE_RATE	(10)	// light sensor sample rate (Hz)
#define	PROX_SAMPLE_RATE	(10)	// proximity sensor sample rate (Hz)

#define SET_PROX_THRESHOLD  (true)	// true = manually set proximity threshold
#define	PROX_THRESHOLD		(100)	// manual prox sensor threshold for "near"

#define SET_PROX_CURRENT    (true)	// true = manually set proximity current
#define	PROX_CURRENT_mA     (50)	// current for proximity sensor LEDs (mA)

#define PRINT_BANNER        (true)  // true = print sensor config information
#define SCALED_DATA         (true)  // true = convert sensor data to std. units

// Hardware environment constants

#define ACTIVITY_LED    (LED1)      // which LED to blink to show activity

static const char * const prox_labels[4] = {
	// strings to display based on proximity values
	"none  ",		// PROXIMITY_NONE
	"FAR   ",		// PROXIMITY_FAR
	"MEDIUM",		// PROXIMITY_MEDIUM
	"NEAR  " 		// PROXIMITY_NEAR
};

int main(void)
{
	sensor_t light_dev;     // light sensor device descriptor
	sensor_t prox_dev;      // proximity sensor device descriptor


	/* Initialize the board (Xplained UC3 or XMEGA & Xplained Sensor boards)
	 * I/O pin mappings and any other configurable resources selected in
	 * the build configuration.
	 */
	sensor_platform_init();

	// Attach descriptors to the defined sensor devices.

	sensor_attach(&light_dev, SENSOR_TYPE_LIGHT,     0, 0);
	sensor_attach(&prox_dev,  SENSOR_TYPE_PROXIMITY, 0, 0);

	if (light_dev.err || prox_dev.err) {
		puts ("\rSensor initialization error.");
		mdelay (5000);
		return EXIT_FAILURE;
	}


	// Print sensor information

	if (PRINT_BANNER) {

		static const char * const banner_format =
			"%s\r\nID = 0x%02x ver. 0x%02x\r\n"
			"Bandwidth = %d Hz  Range = +/- %d\r\n\n";
	
		uint32_t id;
		uint8_t  version;
		int16_t  freq, range;
	
		sensor_device_id(&light_dev, &id, &version);
		sensor_get_bandwidth(&light_dev, &freq);
		sensor_get_range(&light_dev, &range);
	
		printf(banner_format, light_dev.drv->caps.name,
			(unsigned) id, (unsigned) version, freq, range);
	
		sensor_device_id(&prox_dev, &id, &version);
		sensor_get_bandwidth(&prox_dev, &freq);
		sensor_get_range(&prox_dev, &range);
	
		printf(banner_format, prox_dev.drv->caps.name,
			(unsigned) id, (unsigned) version, freq, range);
	
	
		mdelay(500);
	}

	// Set sample intervals

	if (sensor_set_sample_rate(&light_dev, LIGHT_SAMPLE_RATE) != true) {
		printf("Error setting light sensor sample rate.\r\n");
		}

	if (sensor_set_sample_rate(&prox_dev, PROX_SAMPLE_RATE) != true) {	
		printf("Error setting proximity sensor sample rate.\r\n");
		}

	sensor_set_channel(&prox_dev, SENSOR_CHANNEL_ALL);	// select all channels

#if (SET_PROX_THRESHOLD == true)
	// Manually  set proximity threshold values for each channel
	//   Otherwise, sensor will use values previously stored in nvram.
	sensor_set_threshold(&prox_dev, SENSOR_THRESHOLD_NEAR_PROXIMITY, 
		PROX_THRESHOLD);
#endif

#if (SET_PROX_CURRENT == true)
	// Manually set LED current value for each channel
	//   Otherwise, sensor will use default values
	sensor_set_current(&prox_dev, PROX_CURRENT_mA);
#endif


	// Initialize sensor data descriptors for scaled vs. raw data.

	static sensor_data_t light_data = {.scaled = SCALED_DATA};
	static sensor_data_t prox_data  = {.scaled = SCALED_DATA};


	// Enter main loop

	while (true) {              // loop forever

		LED_Toggle(ACTIVITY_LED);

		// Read sensor values

		sensor_get_light(&light_dev, &light_data);
		sensor_get_proximity(&prox_dev,  &prox_data);


		// Print sensor values 

		if (SCALED_DATA) {

			printf("light = [%5d]\r\n", (int16_t) light_data.light.value);

			printf("prox  = 1:%s 2:%s 3:%s\r\n",
				prox_labels[prox_data.proximity.value[0]],
				prox_labels[prox_data.proximity.value[1]],
				prox_labels[prox_data.proximity.value[2]]);

		} else {

			printf("light = [%5d]\r\n", (int16_t) light_data.light.value);

			printf("prox = [%.5x, %.5x, %.5x]\r\n", 
				(int16_t) prox_data.proximity.value[0],
				(int16_t) prox_data.proximity.value[1], 
				(int16_t) prox_data.proximity.value[2]);
		}


		mdelay(500);
	}


	return EXIT_SUCCESS;
}
