/*
 * Copyright (c) 2023, NXP
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef ZEPHYR_DRIVERS_VIDEO_VIDEO_MCUX_ISI_H_
#define ZEPHYR_DRIVERS_VIDEO_VIDEO_MCUX_ISI_H_

#include <fsl_isi_v2.h>

#define ISI_MAX_ACTIVE_BUF 2U

/* Map for the fourcc pixelformat to ISI format. */
struct isi_output_format
{
	uint32_t fourcc;
	isi_output_format_t isi_format;
	uint8_t bpp;
};

struct isi_output_format isi_output_formats[] = {
	{
		.fourcc		= VIDEO_PIX_FMT_RGB24,
		.isi_format	= kISI_OutputRGB888,
		.bpp = 24,
	},
	{
		.fourcc		= VIDEO_PIX_FMT_RGB565,
 		.isi_format	= kISI_OutputRGB565,
		.bpp = 16,
	},
	{
		.fourcc		= VIDEO_PIX_FMT_YUYV,
		.isi_format	= kISI_OutputYUV422_1P8P,
		.bpp = 16,
	},
	{
		.fourcc         = VIDEO_PIX_FMT_RGB32,
		.isi_format     = kISI_OutputARGB8888,
		.bpp = 32,
	}
};

/* ISI input/camera config */
typedef struct _isi_input_config
{
	uint32_t pixelformat;
	uint32_t width;
	uint32_t height;
	uint8_t bpp;
} isi_input_config_t;

static const isi_input_config_t isi_input_parallel =
{
	.pixelformat = VIDEO_PIX_FMT_UYVY,
	.width = 1280,
	.height = 800,
	.bpp = 16,
};

static const isi_input_config_t isi_input_mipi =
{
	.pixelformat = VIDEO_PIX_FMT_UYVY,
	.width = 1280,
	.height = 800,
	.bpp = 16,
};

static const isi_csc_config_t csc_yuv2rgb = {
	.mode = kISI_CscYCbCr2RGB,
	.A1   = 1.164f,
	.A2   = 0.0f,
	.A3   = 1.596f,
	.B1   = 1.164f,
	.B2   = -0.392f,
	.B3   = -0.813f,
	.C1   = 1.164f,
	.C2   = 2.017f,
	.C3   = 0.0f,
	.D1   = -16,
	.D2   = -128,
	.D3   = -128,
};

static const isi_csc_config_t csc_rgb2yuv = {
	.mode = kISI_CscRGB2YCbCr,
	.A1   = 0.257f,
	.A2   = 0.504f,
	.A3   = 0.098f,
	.B1   = -0.148f,
	.B2   = -0.291f,
	.B3   = 0.439f,
	.C1   = 0.439f,
	.C2   = -0.368f,
	.C3   = -0.071f,
	.D1   = 16,
	.D2   = 128,
	.D3   = 128,
};

#endif /* ZEPHYR_DRIVERS_VIDEO_VIDEO_MCUX_ISI_H_ */
