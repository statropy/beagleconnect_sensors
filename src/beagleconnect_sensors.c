/*
 * Copyright (c) 2020 Statropy Software LLC
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_LEVEL CONFIG_LOG_DEFAULT_LEVEL
#include <logging/log.h>
LOG_MODULE_REGISTER(beagleconnect_sensors, LOG_LEVEL);

#include <stdio.h>
#include <zephyr.h>
#include <device.h>
#include <drivers/sensor.h>
#include <drivers/gpio.h>
#include <drivers/flash.h>

#define LED0_NODE DT_ALIAS(led0)
#define LED0	DT_GPIO_LABEL(LED0_NODE, gpios)
#define PIN0	DT_GPIO_PIN(LED0_NODE, gpios)
#define FLAGS0	DT_GPIO_FLAGS(LED0_NODE, gpios)

#define LED1_NODE DT_ALIAS(led1)
#define LED1	DT_GPIO_LABEL(LED1_NODE, gpios)
#define PIN1	DT_GPIO_PIN(LED1_NODE, gpios)
#define FLAGS1	DT_GPIO_FLAGS(LED1_NODE, gpios)

#define LED2_NODE DT_ALIAS(led2)
#define LED2	DT_GPIO_LABEL(LED2_NODE, gpios)
#define PIN2	DT_GPIO_PIN(LED2_NODE, gpios)
#define FLAGS2	DT_GPIO_FLAGS(LED2_NODE, gpios)

#define LED3_NODE DT_ALIAS(led3)
#define LED3	DT_GPIO_LABEL(LED3_NODE, gpios)
#define PIN3	DT_GPIO_PIN(LED3_NODE, gpios)
#define FLAGS3	DT_GPIO_FLAGS(LED3_NODE, gpios)

#define SW0_NODE	DT_ALIAS(sw0)
#define SW0_GPIO_LABEL	DT_GPIO_LABEL(SW0_NODE, gpios)
#define SW0_GPIO_PIN	DT_GPIO_PIN(SW0_NODE, gpios)
#define SW0_GPIO_FLAGS	(GPIO_INPUT | DT_GPIO_FLAGS(SW0_NODE, gpios))

#define FLASH_DEVICE DT_LABEL(DT_INST(0, jedec_spi_nor))

static struct gpio_callback button_cb_data;
static const struct device *dev_led1;

static void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	if(dev_led1) {
		gpio_pin_toggle(dev_led1, PIN1);
	}
}

static void display_light(const struct device *sensor)
{
	if(sensor == NULL) return;
	struct sensor_value light;
	int rc = sensor_sample_fetch(sensor);

	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_LIGHT,
					&light);
	}
	if (rc < 0) {
		printf("ERROR: OPT3001 Update failed: %d\n", rc);
	} else {
		printf("Light: %f\n", sensor_value_to_double(&light));
	}
}

static void display_temp_rh(const struct device *sensor)
{
	if(sensor == NULL) return;
	struct sensor_value temp, humidity;

	int rc = sensor_sample_fetch(sensor);
	if (rc == 0) {
		sensor_channel_get(sensor, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	}
	if (rc == 0) {
		sensor_channel_get(sensor, SENSOR_CHAN_HUMIDITY, &humidity);
	}

	if (rc < 0) {
		printf("ERROR: HDC2010 Update failed: %d\n", rc);
	} else {
		printf("Temp = %d.%06d C, RH = %d.%06d %%\n",
			temp.val1, temp.val2, humidity.val1, humidity.val2);
	}
}

static void display_accel(const struct device *sensor)
{
	if(sensor == NULL) return;
	struct sensor_value accel[3];
	int rc = sensor_sample_fetch(sensor);

	if (rc == -EBADMSG) {
		// /* Sample overrun.  Ignore in polled mode. */
		// if (IS_ENABLED(CONFIG_LIS2DH_TRIGGER)) {
		// 	overrun = "[OVERRUN] ";
		// }
		rc = 0;
	}
	if (rc == 0) {
		rc = sensor_channel_get(sensor,
					SENSOR_CHAN_ACCEL_XYZ,
					accel);
	}
	if (rc < 0) {
		printf("ERROR: LIS2DH12 Update failed: %d\n", rc);
	} else {
		printf("x %f , y %f , z %f\n",
			sensor_value_to_double(&accel[0]),
			sensor_value_to_double(&accel[1]),
			sensor_value_to_double(&accel[2]));
	}
}

void main(void)
{
	const struct device *light_sensor = device_get_binding(DT_LABEL(DT_INST(0, ti_opt3001)));
	const struct device *temp_rh_sensor = device_get_binding(DT_LABEL(DT_INST(0, ti_hdc2000)));
	const struct device *accel_sensor = device_get_binding(DT_LABEL(DT_INST(0, st_lis2dh)));
	const struct device *dev_led0 = device_get_binding(LED0);
	dev_led1 = device_get_binding(LED1);
	const struct device *flash_dev = device_get_binding(FLASH_DEVICE);

	// const struct device *dev_led2 = device_get_binding(LED2);
	// const struct device *dev_led3 = device_get_binding(LED3);

	const struct device *button = device_get_binding(SW0_GPIO_LABEL);

	if (light_sensor == NULL) {
		printf("Could not get %s device\n", DT_LABEL(DT_INST(0, ti_opt3001)));
	} else {
		printf("%s Ready!\n", DT_LABEL(DT_INST(0, ti_opt3001)));
	}

	if (temp_rh_sensor == NULL) {
		printf("Could not get %s device\n", DT_LABEL(DT_INST(0, ti_hdc2000)));
	} else {
		printf("%s Ready!\n", DT_LABEL(DT_INST(0, ti_hdc2000)));
	}

	if (accel_sensor == NULL) {
		printf("Could not get %s device\n", DT_LABEL(DT_INST(0, st_lis2dh)));
	} else {
		printf("%s Ready!\n", DT_LABEL(DT_INST(0, st_lis2dh)));
	}

	if (dev_led0 == NULL || gpio_pin_configure(dev_led0, PIN0, GPIO_OUTPUT_ACTIVE | FLAGS0) < 0) {
		printf("Could not configure LED0");
	}
	if (dev_led1 == NULL || gpio_pin_configure(dev_led1, PIN1, GPIO_OUTPUT_ACTIVE | FLAGS1) < 0) {
		printf("Could not configure LED1");
	}
	// if (dev_led2 == NULL || gpio_pin_configure(dev_led2, PIN2, GPIO_OUTPUT_ACTIVE | FLAGS2) < 0) {
	// 	printf("Could not configure LED2");
	// }
	// if (dev_led3 == NULL || gpio_pin_configure(dev_led3, PIN3, GPIO_OUTPUT_ACTIVE | FLAGS3) < 0) {
	// 	printf("Could not configure LED3");
	// }

	if (button == NULL || gpio_pin_configure(button, SW0_GPIO_PIN, SW0_GPIO_FLAGS) != 0) {
		printf("Error: failed to configure %s pin %d\n",
				SW0_GPIO_LABEL, SW0_GPIO_PIN);
	}
	if (gpio_pin_interrupt_configure(button, SW0_GPIO_PIN, GPIO_INT_EDGE_TO_ACTIVE) != 0) {
		printf("Error: failed to configure interrupt on %s pin %d\n",
				SW0_GPIO_LABEL, SW0_GPIO_PIN);
	}
	gpio_init_callback(&button_cb_data, button_pressed, BIT(SW0_GPIO_PIN));
	gpio_add_callback(button, &button_cb_data);
	printk("Set up button at %s pin %d\n", SW0_GPIO_LABEL, SW0_GPIO_PIN);

	if (!flash_dev) {
		printf("SPI flash driver %s was not found!\n",
		       FLASH_DEVICE);
		return;
	}
	uint8_t flash_id[3];
	if(flash_read_jedec_id(flash_dev, flash_id) == 0) {
		printf("Flash ID: %x %x %x\n", flash_id[0], flash_id[1], flash_id[2]);
	} else {
		printf("Error: Cannot read flash ID\n");
	}

	printf("Polling at 0.5 Hz\n");
	while (true) {
		if(dev_led0) { gpio_pin_toggle(dev_led0, PIN0); }
		// if(dev_led1) { gpio_pin_toggle(dev_led1, PIN1); }
		// if(dev_led2) { gpio_pin_toggle(dev_led2, PIN2); }
		// if(dev_led3) { gpio_pin_toggle(dev_led3, PIN3); }
		
		display_light(light_sensor);
		display_temp_rh(temp_rh_sensor);
		display_accel(accel_sensor);
		printf("\n");
		k_sleep(K_MSEC(2000));
	}
}
