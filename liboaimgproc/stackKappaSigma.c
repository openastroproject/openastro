/*****************************************************************************
 *
 * stackKappaSigma.c -- kappa sigma stacking method
 *
 * Copyright 2019 James Fidell (james@openastroproject.org)
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
#include <openastro/imgproc.h>

#include <stdlib.h>
#include <math.h>


int
oaStackKappaSigma8 ( void** frameArray, unsigned int numFrames, void* target,
		unsigned int length, double kappa )
{
	uint8_t**	frames = ( uint8_t** ) frameArray;
	uint8_t*	tgt = target;
  unsigned int i, j;
	double total, mean, sigma, delta, min, max;
	unsigned int finalMean, numSamples;

	for ( i = 0; i < length; i++ ) {
		mean = 0;
		total = 0;
		for ( j = 0; j < numFrames; j++ ) {
			total += frames[j][i];
		}
		mean = total / numFrames;
		sigma = 0;
		for ( j = 0; j < numFrames; j++ ) {
			delta = frames[j][i] - mean;
			sigma += delta * delta;
		}
		sigma /= ( numFrames - 1 );
		sigma = sqrt ( sigma );
		min = mean - ( kappa * sigma );
		max = mean + ( kappa * sigma );
		finalMean = numSamples = 0;
		for ( j = 0; j < numFrames; j++ ) {
			if ( frames[j][i] >= min && frames[j][i] <= max ) {
				finalMean += frames[j][i];
				numSamples++;
			}
		}
		if ( numSamples ) {
			*tgt++ = finalMean / numSamples;
		} else {
			fprintf ( stderr, "no samples are between %f and %f\n", min, max );
			*tgt++ = 0;
		}
	}

  return 0;
}
