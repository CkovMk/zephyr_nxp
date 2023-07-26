/*
 * Copyright (c) 2023, STRIM, ALC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_DRIVERS_I2C_MCUX_FLEXIO_H_
#define ZEPHYR_DRIVERS_I2C_MCUX_FLEXIO_H_

#include <zephyr/drivers/i2c.h>


struct mcux_flexio_api {
	void (*interrupt_enable)(const struct device *flexio_dev);
	void (*interrupt_disable)(const struct device *flexio_dev);
	void (*lock)(const struct device *flexio_dev);
	void (*unlock)(const struct device *flexio_dev);
	int (*get_rate)(const struct device *flexio_dev, uint32_t *rate);
};

struct mcux_flexio_child_res {
	uint8_t shifter;
	uint8_t num_shifter;
	uint8_t timer;
	uint8_t num_timer;
	uint32_t pin;
	uint32_t num_pin;
};


#endif /* ZEPHYR_DRIVERS_I2C_MCUX_FLEXIO_H_ */
