/*****************************************************************************
 *
 * stack.c -- main stacking entrypoints
 *
 * Copyright 2019, 2021 James Fidell (james@openastroproject.org)
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

#include <openastro/errno.h>
#include <openastro/util.h>
#include <openastro/imgproc.h>
#include <openastro/video/formats.h>

#include "imgstack.h"


int
oaStackSum ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length, unsigned int frameFormat )
{
	int numBits, littleEndian, fullColour;

	if ( oaFrameFormats[ frameFormat ].planar ) {
		oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
				frameFormat );
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	numBits = oaFrameFormats[ frameFormat ].bitsPerPixel;
	fullColour = oaFrameFormats[ frameFormat ].fullColour;
	littleEndian = oaFrameFormats[ frameFormat ].littleEndian;

	if ( numBits == 8 || ( numBits == 24 && fullColour )) {
		return oaStackSum8 ( frameArray, numFrames, target, length );
	}

	if ( numBits <= 16 || ( numBits == 48 && fullColour )) {
		if ( littleEndian ) {
			return oaStackSum16LE ( frameArray, numFrames, target, length );
		}
		return oaStackSum16BE ( frameArray, numFrames, target, length );
	}

	oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
			frameFormat );
	return -OA_ERR_UNSUPPORTED_FORMAT;
}


int
oaStackMean ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length, unsigned int frameFormat )
{
	int numBits, littleEndian, fullColour;

	if ( oaFrameFormats[ frameFormat ].planar ) {
		oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
				frameFormat );
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	numBits = oaFrameFormats[ frameFormat ].bitsPerPixel;
	fullColour = oaFrameFormats[ frameFormat ].fullColour;
	littleEndian = oaFrameFormats[ frameFormat ].littleEndian;

	if ( numBits == 8 || ( numBits == 24 && fullColour )) {
		return oaStackMean8 ( frameArray, numFrames, target, length );
	}

	if ( numBits <= 16 || ( numBits == 48 && fullColour )) {
		if ( littleEndian ) {
			return oaStackMean16LE ( frameArray, numFrames, target, length );
		}
		return oaStackMean16BE ( frameArray, numFrames, target, length );
	}

	oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
			frameFormat );
	return -OA_ERR_UNSUPPORTED_FORMAT;
}


int
oaStackMedian ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length, unsigned int frameFormat )
{
	int numBits, littleEndian, fullColour;

	if ( oaFrameFormats[ frameFormat ].planar ) {
		oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
				frameFormat );
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	numBits = oaFrameFormats[ frameFormat ].bitsPerPixel;
	fullColour = oaFrameFormats[ frameFormat ].fullColour;
	littleEndian = oaFrameFormats[ frameFormat ].littleEndian;

	if ( numBits == 8 || ( numBits == 24 && fullColour )) {
		return oaStackMedian8 ( frameArray, numFrames, target, length );
	}

	if ( numBits <= 16 || ( numBits == 48 && fullColour )) {
		if ( littleEndian ) {
			return oaStackMedian16LE ( frameArray, numFrames, target, length );
		}
		return oaStackMedian16BE ( frameArray, numFrames, target, length );
	}

	oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
			frameFormat );
	return -OA_ERR_UNSUPPORTED_FORMAT;
}


int
oaStackMaximum ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length, unsigned int frameFormat )
{
	int numBits, littleEndian, fullColour;

	if ( oaFrameFormats[ frameFormat ].planar ) {
		oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
				frameFormat );
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	numBits = oaFrameFormats[ frameFormat ].bitsPerPixel;
	fullColour = oaFrameFormats[ frameFormat ].fullColour;
	littleEndian = oaFrameFormats[ frameFormat ].littleEndian;

	if ( numBits == 8 || ( numBits == 24 && fullColour )) {
		return oaStackMaximum8 ( frameArray, numFrames, target, length );
	}

	if ( numBits <= 16 || ( numBits == 48 && fullColour )) {
		if ( littleEndian ) {
			return oaStackMaximum16LE ( frameArray, numFrames, target, length );
		}
		return oaStackMaximum16BE ( frameArray, numFrames, target, length );
	}

	oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
			frameFormat );
	return -OA_ERR_UNSUPPORTED_FORMAT;
}


int
oaStackKappaSigma ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length, double kappa, unsigned int frameFormat )
{
	int numBits, littleEndian, fullColour;

	if ( oaFrameFormats[ frameFormat ].planar ) {
		oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
				frameFormat );
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	numBits = oaFrameFormats[ frameFormat ].bitsPerPixel;
	fullColour = oaFrameFormats[ frameFormat ].fullColour;
	littleEndian = oaFrameFormats[ frameFormat ].littleEndian;

	if ( numBits == 8 || ( numBits == 24 && fullColour )) {
		return oaStackKappaSigma8 ( frameArray, numFrames, target, length, kappa );
	}

	if ( numBits <= 16 || ( numBits == 48 && fullColour )) {
		if ( littleEndian ) {
			return oaStackKappaSigma16LE ( frameArray, numFrames, target, length,
					kappa );
		}
		return oaStackKappaSigma16BE ( frameArray, numFrames, target, length,
				kappa );
	}

	oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
			frameFormat );
	return -OA_ERR_UNSUPPORTED_FORMAT;
}


int
oaStackMedianKappaSigma ( void** frameArray, unsigned int numFrames,
		void* target, unsigned int length, double kappa, unsigned int frameFormat )
{
	int numBits, littleEndian, fullColour;

	if ( oaFrameFormats[ frameFormat ].planar ) {
		oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
				frameFormat );
		return -OA_ERR_UNSUPPORTED_FORMAT;
	}

	numBits = oaFrameFormats[ frameFormat ].bitsPerPixel;
	fullColour = oaFrameFormats[ frameFormat ].fullColour;
	littleEndian = oaFrameFormats[ frameFormat ].littleEndian;

	if ( numBits == 8 || ( numBits == 24 && fullColour )) {
		return oaStackMedianKappaSigma8 ( frameArray, numFrames, target, length,
				kappa );
	}

	if ( numBits <= 16 || ( numBits == 48 && fullColour )) {
		if ( littleEndian ) {
			return oaStackMedianKappaSigma16LE ( frameArray, numFrames, target,
					length, kappa );
		}
		return oaStackMedianKappaSigma16BE ( frameArray, numFrames, target, length,
				kappa );
	}

	oaLogError ( OA_LOG_IMGPROC, "Unable to stack frame format %d",
			frameFormat );
	return -OA_ERR_UNSUPPORTED_FORMAT;
}
