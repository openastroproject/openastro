/*****************************************************************************
 *
 * Spinformats.c -- match Spinnaker frame formats to liboavideo
 *
 * Copyright 2021
 *   James Fidell (james@openastroproject.org)
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

#include <oa_common.h>

#include <openastro/camera.h>
#include <openastro/demosaic.h>
#include <spinc/SpinnakerC.h>

#include "Spin.h"

int	_spinFormatMap[ NUM_SPIN_FORMATS ] =
{
    OA_PIX_FMT_GREY8,					// PixelFormat_Mono8
    OA_PIX_FMT_GREY16LE,			// PixelFormat_Mono16
    OA_PIX_FMT_RGB24,					// PixelFormat_RGB8Packed
    OA_PIX_FMT_GRBG8,					// PixelFormat_BayerGR8
    OA_PIX_FMT_RGGB8,					// PixelFormat_BayerRG8
    OA_PIX_FMT_GBRG8,					// PixelFormat_BayerGB8
    OA_PIX_FMT_BGGR8,					// PixelFormat_BayerBG8
    OA_PIX_FMT_GRBG16LE,			// PixelFormat_BayerGR16
    OA_PIX_FMT_RGGB16LE,			// PixelFormat_BayerRG16
    OA_PIX_FMT_GBRG16LE,			// PixelFormat_BayerGB16
    OA_PIX_FMT_BGGR16LE,			// PixelFormat_BayerBG16
    OA_PIX_FMT_GREY12P,				// PixelFormat_Mono12Packed
    OA_PIX_FMT_GRBG12,				// PixelFormat_BayerGR12Packed
    OA_PIX_FMT_RGGB12,				// PixelFormat_BayerRG12Packed
    OA_PIX_FMT_GBRG12,				// PixelFormat_BayerGB12Packed
    OA_PIX_FMT_BGGR12,				// PixelFormat_BayerBG12Packed
    OA_PIX_FMT_YUV411,				// PixelFormat_YUV411Packed
    -1,												// PixelFormat_YUV422Packed
    OA_PIX_FMT_YUV444,				// PixelFormat_YUV444Packed
    OA_PIX_FMT_GREY12P,				// PixelFormat_Mono12p
    OA_PIX_FMT_GRBG12,				// PixelFormat_BayerGR12p
    OA_PIX_FMT_RGGB12,				// PixelFormat_BayerRG12p
    OA_PIX_FMT_GBRG12,				// PixelFormat_BayerGB12p
    OA_PIX_FMT_BGGR12,				// PixelFormat_BayerBG12p
    -1,												// PixelFormat_YCbCr8
    -1,												// PixelFormat_YCbCr422_8
    -1,												// PixelFormat_YCbCr411_8
    OA_PIX_FMT_BGR24,					// PixelFormat_BGR8
    OA_PIX_FMT_BGRA,					// PixelFormat_BGRa8
    OA_PIX_FMT_GREY10P,				// PixelFormat_Mono10Packed
    OA_PIX_FMT_GRBG10,				// PixelFormat_BayerGR10Packed
    OA_PIX_FMT_RGGB10,				// PixelFormat_BayerRG10Packed
    OA_PIX_FMT_GBRG10,				// PixelFormat_BayerGB10Packed
    OA_PIX_FMT_BGGR10,				// PixelFormat_BayerBG10Packed
    OA_PIX_FMT_GREY10P,				// PixelFormat_Mono10p
    OA_PIX_FMT_GRBG10,				// PixelFormat_BayerGR10p
    OA_PIX_FMT_RGGB10,				// PixelFormat_BayerRG10p
    OA_PIX_FMT_GBRG10,				// PixelFormat_BayerGB10p
    OA_PIX_FMT_BGGR10,				// PixelFormat_BayerBG10p
    -1,												// PixelFormat_Mono1p,
    -1,												// PixelFormat_Mono2p,
    -1,												// PixelFormat_Mono4p,
    -1,												// PixelFormat_Mono8s,
    OA_PIX_FMT_GREY10_16LE,		// PixelFormat_Mono10
    OA_PIX_FMT_GREY12_16LE,		// PixelFormat_Mono12
    OA_PIX_FMT_GREY14_16LE,		// PixelFormat_Mono14
    -1,												// PixelFormat_Mono16s,
    -1,												// PixelFormat_Mono32f,
    OA_PIX_FMT_BGGR10_16LE,		// PixelFormat_BayerBG10
    OA_PIX_FMT_BGGR12_16LE,		// PixelFormat_BayerBG12
    OA_PIX_FMT_GBRG10_16BE,		// PixelFormat_BayerGB10
    OA_PIX_FMT_GBRG12_16BE,		// PixelFormat_BayerGB12
    OA_PIX_FMT_GRBG10_16BE,		// PixelFormat_BayerGR10
    OA_PIX_FMT_GRBG12_16BE,		// PixelFormat_BayerGR12
    OA_PIX_FMT_RGGB10_16LE,		// PixelFormat_BayerRG10
    OA_PIX_FMT_RGGB12_16LE,		// PixelFormat_BayerRG12
    OA_PIX_FMT_RGBA,					// PixelFormat_RGBa8
    -1,												// PixelFormat_RGBa10
    -1,												// PixelFormat_RGBa10p
    -1,												// PixelFormat_RGBa12
    -1,												// PixelFormat_RGBa12p
    -1,												// PixelFormat_RGBa14
    -1,												// PixelFormat_RGBa16
    OA_PIX_FMT_RGB24,					// PixelFormat_RGB8
    -1,												// PixelFormat_RGB8_Planar
    OA_PIX_FMT_RGB30LE,				// PixelFormat_RGB10
    -1,												// PixelFormat_RGB10_Planar
    -1,												// PixelFormat_RGB10p
    -1,												// PixelFormat_RGB10p32
    OA_PIX_FMT_RGB36LE,				// PixelFormat_RGB12
    -1,												// PixelFormat_RGB12_Planar
    -1,												// PixelFormat_RGB12p
    OA_PIX_FMT_RGB42LE,				// PixelFormat_RGB14
    OA_PIX_FMT_RGB48LE,				// PixelFormat_RGB16
    -1,												// PixelFormat_RGB16s
    -1,												// PixelFormat_RGB32f
    -1,												// PixelFormat_RGB16_Planar
    -1,												// PixelFormat_RGB565p
    -1,												// PixelFormat_BGRa10
    -1,												// PixelFormat_BGRa10p
    -1,												// PixelFormat_BGRa12
    -1,												// PixelFormat_BGRa12p
    -1,												// PixelFormat_BGRa14
    -1,												// PixelFormat_BGRa16
    -1,												// PixelFormat_RGBa32f
    -1,												// PixelFormat_BGR10
    -1,												// PixelFormat_BGR10p
    -1,												// PixelFormat_BGR12
    -1,												// PixelFormat_BGR12p
    -1,												// PixelFormat_BGR14
    OA_PIX_FMT_BGR48LE,				// PixelFormat_BGR16
    -1,												// PixelFormat_BGR565p
    -1,												// PixelFormat_R8
    -1,												// PixelFormat_R10
    -1,												// PixelFormat_R12
    -1,												// PixelFormat_R16
    -1,												// PixelFormat_G8
    -1,												// PixelFormat_G10
    -1,												// PixelFormat_G12
    -1,												// PixelFormat_G16
    -1,												// PixelFormat_B8
    -1,												// PixelFormat_B10
    -1,												// PixelFormat_B12
    -1,												// PixelFormat_B16
    -1,												// PixelFormat_Coord3D_ABC8
    -1,												// PixelFormat_Coord3D_ABC8_Planar
    -1,												// PixelFormat_Coord3D_ABC10p
    -1,												// PixelFormat_Coord3D_ABC10p_Planar
    -1,												// PixelFormat_Coord3D_ABC12p
    -1,												// PixelFormat_Coord3D_ABC12p_Planar
    -1,												// PixelFormat_Coord3D_ABC16
    -1,												// PixelFormat_Coord3D_ABC16_Planar
    -1,												// PixelFormat_Coord3D_ABC32f
    -1,												// PixelFormat_Coord3D_ABC32f_Planar
    -1,												// PixelFormat_Coord3D_AC8
    -1,												// PixelFormat_Coord3D_AC8_Planar
    -1,												// PixelFormat_Coord3D_AC10p
    -1,												// PixelFormat_Coord3D_AC10p_Planar
    -1,												// PixelFormat_Coord3D_AC12p
    -1,												// PixelFormat_Coord3D_AC12p_Planar
    -1,												// PixelFormat_Coord3D_AC16
    -1,												// PixelFormat_Coord3D_AC16_Planar
    -1,												// PixelFormat_Coord3D_AC32f
    -1,												// PixelFormat_Coord3D_AC32f_Planar
    -1,												// PixelFormat_Coord3D_A8
    -1,												// PixelFormat_Coord3D_A10p
    -1,												// PixelFormat_Coord3D_A12p
    -1,												// PixelFormat_Coord3D_A16
    -1,												// PixelFormat_Coord3D_A32f
    -1,												// PixelFormat_Coord3D_B8
    -1,												// PixelFormat_Coord3D_B10p
    -1,												// PixelFormat_Coord3D_B12p
    -1,												// PixelFormat_Coord3D_B16
    -1,												// PixelFormat_Coord3D_B32f
    -1,												// PixelFormat_Coord3D_C8
    -1,												// PixelFormat_Coord3D_C10p
    -1,												// PixelFormat_Coord3D_C12p
    -1,												// PixelFormat_Coord3D_C16
    -1,												// PixelFormat_Coord3D_C32f
    -1,												// PixelFormat_Confidence1
    -1,												// PixelFormat_Confidence1p
    -1,												// PixelFormat_Confidence8
    -1,												// PixelFormat_Confidence16
    -1,												// PixelFormat_Confidence32f
    -1,												// PixelFormat_BiColorBGRG8
    -1,												// PixelFormat_BiColorBGRG10
    -1,												// PixelFormat_BiColorBGRG10p
    -1,												// PixelFormat_BiColorBGRG12
    -1,												// PixelFormat_BiColorBGRG12p
    -1,												// PixelFormat_BiColorRGBG8
    -1,												// PixelFormat_BiColorRGBG10
    -1,												// PixelFormat_BiColorRGBG10p
    -1,												// PixelFormat_BiColorRGBG12
    -1,												// PixelFormat_BiColorRGBG12p
    -1,												// PixelFormat_SCF1WBWG8
    -1,												// PixelFormat_SCF1WBWG10
    -1,												// PixelFormat_SCF1WBWG10p
    -1,												// PixelFormat_SCF1WBWG12
    -1,												// PixelFormat_SCF1WBWG12p
    -1,												// PixelFormat_SCF1WBWG14
    -1,												// PixelFormat_SCF1WBWG16
    -1,												// PixelFormat_SCF1WGWB8
    -1,												// PixelFormat_SCF1WGWB10
    -1,												// PixelFormat_SCF1WGWB10p
    -1,												// PixelFormat_SCF1WGWB12
    -1,												// PixelFormat_SCF1WGWB12p
    -1,												// PixelFormat_SCF1WGWB14
    -1,												// PixelFormat_SCF1WGWB16
    -1,												// PixelFormat_SCF1WGWR8
    -1,												// PixelFormat_SCF1WGWR10
    -1,												// PixelFormat_SCF1WGWR10p
    -1,												// PixelFormat_SCF1WGWR12
    -1,												// PixelFormat_SCF1WGWR12p
    -1,												// PixelFormat_SCF1WGWR14
    -1,												// PixelFormat_SCF1WGWR16
    -1,												// PixelFormat_SCF1WRWG8
    -1,												// PixelFormat_SCF1WRWG10
    -1,												// PixelFormat_SCF1WRWG10p
    -1,												// PixelFormat_SCF1WRWG12
    -1,												// PixelFormat_SCF1WRWG12p
    -1,												// PixelFormat_SCF1WRWG14
    -1,												// PixelFormat_SCF1WRWG16
    -1,												// PixelFormat_YCbCr8_CbYCr
    -1,												// PixelFormat_YCbCr10_CbYCr
    -1,												// PixelFormat_YCbCr10p_CbYCr
    -1,												// PixelFormat_YCbCr12_CbYCr
    -1,												// PixelFormat_YCbCr12p_CbYCr
    -1,												// PixelFormat_YCbCr411_8_CbYYCrYY
    -1,												// PixelFormat_YCbCr422_8_CbYCrY
    -1,												// PixelFormat_YCbCr422_10
    -1,												// PixelFormat_YCbCr422_10_CbYCrY
    -1,												// PixelFormat_YCbCr422_10p
    -1,												// PixelFormat_YCbCr422_10p_CbYCrY
    -1,												// PixelFormat_YCbCr422_12
    -1,												// PixelFormat_YCbCr422_12_CbYCrY
    -1,												// PixelFormat_YCbCr422_12p
    -1,												// PixelFormat_YCbCr422_12p_CbYCrY
    -1,												// PixelFormat_YCbCr601_8_CbYCr
    -1,												// PixelFormat_YCbCr601_10_CbYCr
    -1,												// PixelFormat_YCbCr601_10p_CbYCr
    -1,												// PixelFormat_YCbCr601_12_CbYCr
    -1,												// PixelFormat_YCbCr601_12p_CbYCr
    -1,												// PixelFormat_YCbCr601_411_8_CbYYCrYY
    -1,												// PixelFormat_YCbCr601_422_8
    -1,												// PixelFormat_YCbCr601_422_8_CbYCrY
    -1,												// PixelFormat_YCbCr601_422_10
    -1,												// PixelFormat_YCbCr601_422_10_CbYCrY
    -1,												// PixelFormat_YCbCr601_422_10p
    -1,												// PixelFormat_YCbCr601_422_10p_CbYCrY
    -1,												// PixelFormat_YCbCr601_422_12
    -1,												// PixelFormat_YCbCr601_422_12_CbYCrY
    -1,												// PixelFormat_YCbCr601_422_12p
    -1,												// PixelFormat_YCbCr601_422_12p_CbYCrY
    -1,												// PixelFormat_YCbCr709_8_CbYCr
    -1,												// PixelFormat_YCbCr709_10_CbYCr
    -1,												// PixelFormat_YCbCr709_10p_CbYCr
    -1,												// PixelFormat_YCbCr709_12_CbYCr
    -1,												// PixelFormat_YCbCr709_12p_CbYCr
    -1,												// PixelFormat_YCbCr709_411_8_CbYYCrYY
    -1,												// PixelFormat_YCbCr709_422_8
    -1,												// PixelFormat_YCbCr709_422_8_CbYCrY
    -1,												// PixelFormat_YCbCr709_422_10
    -1,												// PixelFormat_YCbCr709_422_10_CbYCrY
    -1,												// PixelFormat_YCbCr709_422_10p
    -1,												// PixelFormat_YCbCr709_422_10p_CbYCrY
    -1,												// PixelFormat_YCbCr709_422_12
    -1,												// PixelFormat_YCbCr709_422_12_CbYCrY
    -1,												// PixelFormat_YCbCr709_422_12p
    -1,												// PixelFormat_YCbCr709_422_12p_CbYCrY
    OA_PIX_FMT_YUV444,				// PixelFormat_YUV8_UYV
    OA_PIX_FMT_YUV411,				// PixelFormat_YUV411_8_UYYVYY
    OA_PIX_FMT_YUYV,					// PixelFormat_YUV422_8
    -1,												// PixelFormat_YUV422_8_UYVY
    -1,												// PixelFormat_Polarized8
    -1,												// PixelFormat_Polarized10p
    -1,												// PixelFormat_Polarized12p
    -1,												// PixelFormat_Polarized16
    -1,												// PixelFormat_BayerRGPolarized8
    -1,												// PixelFormat_BayerRGPolarized10p
    -1,												// PixelFormat_BayerRGPolarized12p
    -1,												// PixelFormat_BayerRGPolarized16
    -1,												// PixelFormat_LLCMono8
    -1,												// PixelFormat_LLCBayerRG8
    -1,												// PixelFormat_JPEGMono8
    OA_PIX_FMT_JPEG8,					// PixelFormat_JPEGColor8
    -1,												// PixelFormat_Raw16
    -1,												// PixelFormat_Raw8
    -1,												// PixelFormat_R12_Jpeg
    -1,												// PixelFormat_GR12_Jpeg
    -1,												// PixelFormat_GB12_Jpeg
    -1,												// PixelFormat_B12_Jpeg
};
