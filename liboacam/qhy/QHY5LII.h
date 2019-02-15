/*****************************************************************************
 *
 * QHY5LII.h -- header for QHY5LII-specific control
 *
 * Copyright 2014,2015,2017 James Fidell (james@openastroproject.org)
 *
 * License:
 *
 * This file is part of the Open Astro Project.
 *
 * The Open Astro Project is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * The Open Astro Project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Open Astro Project.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 *****************************************************************************/

#ifndef OA_QHY5LII_H
#define OA_QHY5LII_H

extern int		_QHY5LIIInitCamera ( oaCamera* );
extern void*		oacamQHY5LIIcontroller ( void* );
extern void		oaQHY5LIISetAllControls ( oaCamera* );

#define QHY5LII_IMAGE_WIDTH		1280
#define QHY5LII_IMAGE_HEIGHT		960
#define QHY5LII_DEFAULT_EXPOSURE	20
#define QHY5LII_DEFAULT_SPEED		0
#define QHY5LII_DEFAULT_USBTRAFFIC	30
#define QHY5LII_EOF_LEN			5

#define QHY5LII_MONO_GAIN_MIN		0
#define QHY5LII_MONO_GAIN_MAX		796
#define QHY5LII_COLOUR_GAIN_MIN         0
#define QHY5LII_COLOUR_GAIN_MAX         398

#define	MT9M034_Y_ADDR_START			0x3002
#define	MT9M034_X_ADDR_START			0x3004
#define	MT9M034_Y_ADDR_END			0x3006
#define	MT9M034_X_ADDR_END			0x3008
#define	MT9M034_FRAME_LENGTH_LINES		0x300A
#define	MT9M034_LINE_LENGTH_PCK			0x300C
#define MT9M034_COARSE_INTEGRATION_TIME		0x3012
#define MT9M034_FINE_INT_TIME			0x3014
#define	MT9M034_COARSE_INT_TIME_CB		0x3016
#define MT9M034_FINE_INT_TIME_CB		0x3018
#define MT9M034_RESET_REGISTER			0x301A
#define MT9M034_DATA_PEDESTAL			0x301E
#define MT9M034_VT_SYS_CLK_DIV			0x302A
#define MT9M034_VT_PIX_CLK_DIV			0x302C
#define MT9M034_PRE_PLL_CLK_DIV			0x302E
#define MT9M034_PLL_MULTIPLIER			0x3030
#define MT9M034_DIGITAL_BINNING			0x3032
#define	MT9M034_READ_MODE			0x3040
#define MT9M034_DARK_CONTROL			0x3044
#define MT9M034_GREEN1_GAIN			0x3056
#define MT9M034_BLUE_GAIN			0x3058
#define MT9M034_RED_GAIN			0x305A
#define MT9M034_GREEN2_GAIN			0x305C
#define MT9M034_GLOBAL_GAIN			0x305E
#define MT9M034_EMBEDDED_DATA_CTRL		0x3064
#define MT9M034_TEST_PATTERN			0x3070
#define MT9M034_TEST_RAW_MODE			0x307A
#define MT9M034_MODE_CTRL			0x3082
#define MT9M034_SEQ_DATA_PORT			0x3086
#define MT9M034_SEQ_CTRL_PORT			0x3088
#define MT9M034_ERS_PROG_START_ADDR		0x309E
#define	MT9M034_X_ODD_INC			0x30A2
#define	MT9M034_Y_ODD_INC			0x30A6
#define MT9M034_DIGITAL_TEST			0x30B0
#define MT9M034_TEMPERATURE			0x30B2
#define MT9M034_TEMPERATURE_CONTROL		0x30B4
#define MT9M034_DIGITAL_CTRL			0x30BA
#define MT9M034_GREEN1_GAIN_CB			0x30BC
#define MT9M034_BLUE_GAIN_CB			0x30BE
#define MT9M034_RED_GAIN_CB			0x30C0
#define MT9M034_GREEN2_GAIN_CB			0x30C2
#define MT9M034_GLOBAL_GAIN_CB			0x30C4
#define MT9M034_TEMPERATURE_CALIB_1		0x30C6
#define MT9M034_TEMPERATURE_CALIB_2		0x30C8
#define MT9M034_COLUMN_CORRECTION		0x30D4
#define MT9M034_ADC_BITS_2_3			0x30E0
#define MT9M034_ADC_BITS_4_5			0x30E2
#define MT9M034_ADC_BITS_6_7			0x30E4
#define MT9M034_ADC_CONFIG1			0x30E6
#define MT9M034_ADC_CONFIG2			0x30E8
#define MT9M034_AE_CTRL_REG			0x3100
#define MT9M034_AE_LUMA_TARGET_REG		0x3102
#define MT9M034_AE_HIST_TARGET_REG		0x3104
#define MT9M034_AE_DCG_EXPOSURE_HIGH_REG	0x3112
#define MT9M034_AE_DCG_EXPOSURE_LOW_REG		0x3114
#define MT9M034_AE_DCG_GAIN_FACTOR_REG		0x3116
#define MT9M034_AE_DCG_GAIN_FACTOR_INV_REG	0x3118
#define MT9M034_AE_MAX_EXPOSURE_REG		0x311C
#define MT9M034_AE_MIN_EXPOSURE_REG		0x311E
#define MT9M034_AE_ALPHA_V1_REG			0x3126
#define MT9M034_HDR_COMP			0x31D0
#define MT9M034_ANALOG_REG			0x3ED6
#define MT9M034_DAC_LD_10_11			0x3ED6
#define MT9M034_DAC_LD_12_13			0x3ED8
#define MT9M034_DAC_LD_14_15			0x3EDA
#define MT9M034_DAC_LD_16_17			0x3EDC
#define MT9M034_DAC_LD_18_19			0x3EDE
#define MT9M034_DAC_LD_20_21			0x3EE0
#define MT9M034_DAC_LD_22_23			0x3EE2
#define MT9M034_DAC_LD_24_25			0x3EE4
#define MT9M034_DAC_LD_26_27			0x3EE6

#define MT9M034_HOR_AND_VER_BIN			0x0022
#define MT9M034_HOR_BIN				0x0011
#define MT9M034_DISABLE_BINNING			0x0000

#define MT9M034_RESET				0x00D9
#define MT9M034_STREAM_OFF			0x00D8
#define MT9M034_STREAM_ON			0x00DC

#endif	/* OA_QHY5LII_H */
