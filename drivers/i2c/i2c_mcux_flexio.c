/*
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2019-2023, NXP
 * Copyright (c) 2022 Vestas Wind Systems A/S
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT nxp_imx_flexio_i2c_master

#include <errno.h>
#include <zephyr/drivers/i2c.h>
#include <fsl_flexio_i2c_master.h>
#include <mcux_flexio/mcux_flexio.h>

#ifdef CONFIG_PINCTRL
#include <zephyr/drivers/pinctrl.h>
#endif /* CONFIG_PINCTRL */

#ifdef CONFIG_I2C_MCUX_FLEXIO_I2C_BUS_RECOVERY
#include "i2c_bitbang.h"
#include <zephyr/drivers/gpio.h>
#endif /* CONFIG_I2C_MCUX_FLEXIO_I2C_BUS_RECOVERY */

#define LOG_LEVEL CONFIG_I2C_LOG_LEVEL
#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(i2c_mcux_flexio_i2c);

#include "i2c_mcux_flexio.h"

#include "i2c-priv.h"
/* Wait for the duration of 12 bits to detect a NAK after a bus
 * address scan.  (10 appears sufficient, 20% safety factor.)
 */
#define SCAN_DELAY_US(baudrate) (12 * USEC_PER_SEC / baudrate)

struct i2c_mcux_flexio_data {
	flexio_i2c_master_handle_t handle;
	struct k_sem lock;
	struct k_sem device_sync_sem;
	status_t callback_status;
};

static void i2c_mcux_flexio_master_transfer_callback(FLEXIO_I2C_Type *base,
						flexio_i2c_master_handle_t *handle,
						status_t status, void *userData)
{
	struct i2c_mcux_flexio_data *data = userData;

	ARG_UNUSED(handle);
	ARG_UNUSED(base);

	data->callback_status = status;
	k_sem_give(&data->device_sync_sem);
}

static int i2c_mcux_flexio_configure(const struct device *dev,
				uint32_t dev_config_raw)
{
	const struct i2c_mcux_flexio_config *config = dev->config;
	const struct mcux_flexio_api *flexio_api = config->flexio_dev->api;
	struct i2c_mcux_flexio_data *data = dev->data;

	uint32_t clock_freq;
	uint32_t baudrate;
	int ret;

	if (!(I2C_MODE_CONTROLLER & dev_config_raw)) {
		return -EINVAL;
	}

	if (I2C_ADDR_10_BITS & dev_config_raw) {
		return -EINVAL;
	}

	switch (I2C_SPEED_GET(dev_config_raw)) {
	case I2C_SPEED_STANDARD:
		baudrate = KHZ(100);
		break;
	case I2C_SPEED_FAST:
		baudrate = KHZ(400);
		break;
	case I2C_SPEED_FAST_PLUS:
		baudrate = MHZ(1);
		break;
	default:
		return -EINVAL;
	}

	if (flexio_api->get_rate(config->flexio_dev, &clock_freq)) {
		return -EINVAL;
	}

	ret = k_sem_take(&data->lock, K_FOREVER);
	if (ret) {
		return ret;
	}

	FLEXIO_I2C_MasterSetBaudRate(config->flexio_i2c, baudrate, clock_freq);
	k_sem_give(&data->lock);

	return 0;
}

static uint32_t i2c_mcux_flexio_convert_flags(int msg_flags)
{
	uint32_t flags = 0U;

	if (!(msg_flags & I2C_MSG_STOP)) {
		flags |= kFLEXIO_I2C_TransferNoStopFlag;
	}

	if (msg_flags & I2C_MSG_RESTART) {
		flags |= kFLEXIO_I2C_TransferRepeatedStartFlag;
	}

	return flags;
}

static int i2c_mcux_flexio_transfer(const struct device *dev, struct i2c_msg *msgs,
			       uint8_t num_msgs, uint16_t addr)
{
	const struct i2c_mcux_flexio_config *config = dev->config;
	struct i2c_mcux_flexio_data *data = dev->data;
	flexio_i2c_master_transfer_t transfer;
	status_t status;
	int ret = 0;

	memset(&transfer, 0, sizeof(transfer));
	ret = k_sem_take(&data->lock, K_FOREVER);
	if (ret) {
		return ret;
	}

	/* Iterate over all the messages */
	for (int i = 0; i < num_msgs; i++) {
		if (I2C_MSG_ADDR_10_BITS & msgs->flags) {
			ret = -ENOTSUP;
			break;
		}

		/* Initialize the transfer descriptor */
		transfer.flags = i2c_mcux_flexio_convert_flags(msgs->flags);

		/* Prevent the controller to send a start condition between
		 * messages, except if explicitly requested.
		 */
		if (i != 0 && !(msgs->flags & I2C_MSG_RESTART)) {
			transfer.flags |= kFLEXIO_I2C_TransferNoStartFlag;
		}

		transfer.slaveAddress = addr;
		transfer.direction = (msgs->flags & I2C_MSG_READ)
			? kFLEXIO_I2C_Read : kFLEXIO_I2C_Write;
		transfer.subaddress = 0;
		transfer.subaddressSize = 0;
		transfer.data = msgs->buf;
		transfer.dataSize = msgs->len;

		/* Start the transfer */
		status = FLEXIO_I2C_MasterTransferNonBlocking(config->flexio_i2c,
				&data->handle, &transfer);

		/* Return an error if the transfer didn't start successfully
		 * e.g., if the bus was busy
		 */
		if (status != kStatus_Success) {
			FLEXIO_I2C_MasterTransferAbort(config->flexio_i2c, &data->handle);
			ret = -EIO;
			break;
		}

		/* Wait for the transfer to complete */
		k_sem_take(&data->device_sync_sem, K_FOREVER);

		/* Return an error if the transfer didn't complete
		 * successfully. e.g., nak, timeout, lost arbitration
		 */
		if (data->callback_status != kStatus_Success) {
			FLEXIO_I2C_MasterTransferAbort(config->flexio_i2c, &data->handle);
			ret = -EIO;
			break;
		}
		if (msgs->len == 0) {
			if (kFLEXIO_I2C_ReceiveNakFlag & FLEXIO_I2C_MasterGetStatusFlags(config->flexio_i2c)) {
				FLEXIO_I2C_MasterTransferAbort(config->flexio_i2c, &data->handle);
				ret = -EIO;
				break;
			}
		}
		/* Move to the next message */
		msgs++;
	}

	k_sem_give(&data->lock);

	return ret;
}

int i2c_mcux_flexio_isr(const struct device *dev)
{
	const struct i2c_mcux_flexio_config *config = dev->config;
	struct i2c_mcux_flexio_data *data = dev->data;

	FLEXIO_I2C_MasterTransferHandleIRQ(config->flexio_i2c, &data->handle);

	return 0;
}

void i2c_mcux_flexio_res(const struct device *dev, struct mcux_flexio_child_res *child_res)
{
	const struct i2c_mcux_flexio_config *config = dev->config;

	if (child_res != NULL && config != NULL) {
		child_res->pin = (1 << config->flexio_i2c->SDAPinIndex) |
				(1 << config->flexio_i2c->SCLPinIndex);
		child_res->num_pin = 2;

		child_res->shifter = (1 << config->flexio_i2c->shifterIndex[0]) |
					(1 << config->flexio_i2c->shifterIndex[1]);
		child_res->num_shifter = 2;
		child_res->timer = (1 << config->flexio_i2c->timerIndex[0]) |
					(1 << config->flexio_i2c->timerIndex[1]) |
					(1 << config->flexio_i2c->timerIndex[2]);
		child_res->num_timer = 3;
	} else {
		LOG_ERR("%s: Error args", __func__);
	}
}

static int i2c_mcux_flexio_init(const struct device *dev)
{
	const struct i2c_mcux_flexio_config *config = dev->config;
	const struct mcux_flexio_api *flexio_api = config->flexio_dev->api;
	struct i2c_mcux_flexio_data *data = dev->data;
	FLEXIO_I2C_Type *flexio_i2c = config->flexio_i2c;
	int err;

	flexio_i2c_master_config_t masterConfig;
	uint32_t clock_freq;

	k_sem_init(&data->lock, 1, 1);
	k_sem_init(&data->device_sync_sem, 0, K_SEM_MAX_LIMIT);

	if (!device_is_ready(config->flexio_dev)) {
		return -ENODEV;
	}

	if (flexio_api->get_rate(config->flexio_dev, &clock_freq)) {
		return -EINVAL;
	}

	/*
	 * masterConfig.enableMaster = true;
	 * masterConfig.enableInDoze = false;
	 * masterConfig.enableInDebug = true;
	 * masterConfig.enableFastAccess = false;
	 * masterConfig.baudRate_Bps = 100000U;
	 */
	FLEXIO_I2C_MasterGetDefaultConfig(&masterConfig);
	FLEXIO_I2C_MasterInit(config->flexio_i2c, &masterConfig, clock_freq);

	LOG_INF("SDAPinIndex=%d, SCLPinIndex=%d, timerIndex[2]=%d, baudRate_Bps=%d, clock_freq=%d\n",
			flexio_i2c->SDAPinIndex, flexio_i2c->SCLPinIndex,
			flexio_i2c->timerIndex[2], masterConfig.baudRate_Bps, clock_freq);

	FLEXIO_I2C_MasterTransferCreateHandle(config->flexio_i2c, &data->handle, \
					i2c_mcux_flexio_master_transfer_callback, data);
#ifdef CONFIG_PINCTRL
	err = pinctrl_apply_state(config->pincfg, PINCTRL_STATE_DEFAULT);
	if (err) {
		return err;
	}
#endif /* CONFIG_PINCTRL */

	return 0;
}

static const struct i2c_driver_api i2c_mcux_flexio_driver_api = {
	.configure = i2c_mcux_flexio_configure,
	.transfer = i2c_mcux_flexio_transfer,
#ifdef CONFIG_I2C_CALLBACK
	.transfer_cb = i2c_mcux_flexio_transfer_cb,
#endif
};

#ifdef CONFIG_PINCTRL
#define I2C_MCUX_FLEXIO_I2C_PINCTRL_DEFINE(n) PINCTRL_DT_INST_DEFINE(n);
#define I2C_MCUX_FLEXIO_I2C_PINCTRL_INIT(n) .pincfg = PINCTRL_DT_INST_DEV_CONFIG_GET(n),
#else
#define I2C_MCUX_FLEXIO_I2C_PINCTRL_DEFINE(n)
#define I2C_MCUX_FLEXIO_I2C_PINCTRL_INIT(n)
#endif /* CONFIG_PINCTRL */

#if CONFIG_I2C_MCUX_FLEXIO_I2C_BUS_RECOVERY
#define I2C_MCUX_FLEXIO_I2C_SCL_INIT(n) .scl = GPIO_DT_SPEC_INST_GET_OR(n, scl_gpios, {0}),
#define I2C_MCUX_FLEXIO_I2C_SDA_INIT(n) .sda = GPIO_DT_SPEC_INST_GET_OR(n, sda_gpios, {0}),
#else
#define I2C_MCUX_FLEXIO_I2C_SCL_INIT(n)
#define I2C_MCUX_FLEXIO_I2C_SDA_INIT(n)
#endif /* CONFIG_I2C_MCUX_FLEXIO_I2C_BUS_RECOVERY */

#define I2C_MCUX_FLEXIO_I2C_INIT(n)						\
	I2C_MCUX_FLEXIO_I2C_PINCTRL_DEFINE(n)				\
									\
	static FLEXIO_I2C_Type flexio_i2c_##n = {			\
		.flexioBase = (FLEXIO_Type *) DT_REG_ADDR(DT_INST_PARENT(n)), \
		.SDAPinIndex  = DT_INST_PROP(n, sda_pin),		\
		.SCLPinIndex  = DT_INST_PROP(n, scl_pin),		\
		.shifterIndex = DT_INST_PROP(n, shifters),		\
		.timerIndex   = DT_INST_PROP(n, timers),		\
	};								\
									\
	static const struct i2c_mcux_flexio_config i2c_mcux_flexio_config_##n = {	\
		.flexio_i2c = &flexio_i2c_##n,				\
		.flexio_dev = DEVICE_DT_GET(DT_INST_PARENT(n)),		\
		I2C_MCUX_FLEXIO_I2C_PINCTRL_INIT(n)			\
	};								\
									\
	static struct i2c_mcux_flexio_data i2c_mcux_flexio_data_##n;		\
									\
	I2C_DEVICE_DT_INST_DEFINE(n, i2c_mcux_flexio_init, NULL,		\
			    &i2c_mcux_flexio_data_##n,			\
			    &i2c_mcux_flexio_config_##n, POST_KERNEL,	\
			    CONFIG_MCUX_FLEXIO_CHILD_INIT_PRIORITY,			\
			    &i2c_mcux_flexio_driver_api);			\
									\

DT_INST_FOREACH_STATUS_OKAY(I2C_MCUX_FLEXIO_I2C_INIT)
