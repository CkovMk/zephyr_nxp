/*
 * Copyright (c) 2023, STRIM, ALC
 *
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef ZEPHYR_DRIVERS_I2C_I2C_MCUX_FLEXIO_H_
#define ZEPHYR_DRIVERS_I2C_I2C_MCUX_FLEXIO_H_

#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/clock_control.h>
#include <fsl_flexio_i2c_master.h>


struct i2c_mcux_flexio_config {
	FLEXIO_I2C_Type *flexio_i2c;
	const struct device *flexio_dev;
#ifdef CONFIG_PINCTRL
	const struct pinctrl_dev_config *pincfg;
#endif /* CONFIG_PINCTRL */
};


int i2c_mcux_flexio_isr(const struct device *dev);
void i2c_mcux_flexio_res(const struct device *dev, struct mcux_flexio_child_res *child_res);


#endif /* ZEPHYR_DRIVERS_I2C_I2C_MCUX_FLEXIO_H_ */
