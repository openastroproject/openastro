/*****************************************************************************
 *
 * oaser.c -- main SER library entrypoint
 *
 * Copyright 2013,2014,2016,2019 James Fidell (james@openastroproject.org)
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
#include <openastro/SER.h>

#include <time.h>
#include <sys/time.h>
#include <fcntl.h>


static void    _oaSERInitMicrosoftTimestamp();
static int64_t _oaSERGetMicrosoftTimestamp ( const char*, int );
static void    _oaSER32BitToLittleEndian ( int32_t, uint8_t* );
static void    _oaSER64BitToLittleEndian ( int64_t, uint8_t* );
static void    _oaSERnToLittleEndian ( int64_t, uint8_t*, uint8_t );

const int64_t  epochTicks          = 621355968000000000LL;
const int64_t  ticksPerSecond      = 10000000;
const int64_t  ticksPerMicrosecond = 10;
static int64_t _offsetFromGMT      = 0;

#define TIMESTAMP_BLOCK_COUNT 1024
#define FRAME_COUNT_POSN      38

#if !HAVE_CREAT64
#define creat64 creat
#endif
#if !HAVE_LSEEK64
#define lseek64 lseek
#endif

int
oaSEROpen ( const char* filename, oaSERContext* context )
{
  int fd;

  if (( fd = creat64 ( filename, 0644 )) < 0 ) {
    return fd;
  }
  context->SERfd = fd;
  context->frames = 0;
  context->bufferSize = TIMESTAMP_BLOCK_COUNT * 8;
  if (!( context->timestampBuffer = malloc ( context->bufferSize ))) {
    return -1;
  }
  context->nextTimestamp = context->timestampBuffer;
  context->framesLeft = 1024;
  return 0;
}


int
oaSERWriteHeader ( oaSERContext* context, oaSERHeader* userHeader )
{
  char          buffer[OA_SER_MAX_STRING_LEN+1];
  int64_t       now;
  int           bitPlanes = 1;
  oaSERHeader   header;

  // See the comments relating to the LittleEndian field for why we do
  // this
  memcpy ( &header, userHeader, sizeof ( oaSERHeader ));

  header.version = 3;
  if ( header.ColorID == OA_SER_RGB || header.ColorID == OA_SER_BGR ) {
    bitPlanes = 3;
  }

  context->frameSize = header.PixelDepth / 8 * header.ImageWidth *
      header.ImageHeight * bitPlanes;
  if (!( context->transformBuffer = malloc ( context->frameSize ))) {
    return -1;
  }
  context->shiftBits = ( header.PixelDepth < 8 ) ?
      ( 8 - header.PixelDepth ) : 0;
  context->pixelDepth = header.PixelDepth;

  ( void ) snprintf ( header.FileID, 15, "LUCAM-RECORDER" );
  if ( write ( context->SERfd, header.FileID, 14 ) != 14 ) {
    return -1;
  }

  _oaSER32BitToLittleEndian ( header.LuID, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  _oaSER32BitToLittleEndian ( header.ColorID, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  // It seems that the sense of this flag is actually inverted -- it's
  // zero when the image byte order is little-endian, non-zero otherwise.
  // For this reason we work on a copy of the header structure, so it can
  // be corrected here without messing up the user's view of the world

  header.LittleEndian = userHeader->LittleEndian ? 0 : 1;
  _oaSER32BitToLittleEndian ( header.LittleEndian, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  _oaSER32BitToLittleEndian ( header.ImageWidth, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  _oaSER32BitToLittleEndian ( header.ImageHeight, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  _oaSER32BitToLittleEndian ( header.PixelDepth, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  header.FrameCount = 0;
  _oaSER32BitToLittleEndian ( header.FrameCount, ( uint8_t* )buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }

  bzero ( buffer, OA_SER_MAX_STRING_LEN+1 );
  ( void ) snprintf ( buffer, OA_SER_MAX_STRING_LEN+1, "%s",
      header.Observer );
  if ( write ( context->SERfd, buffer, OA_SER_MAX_STRING_LEN ) !=
      OA_SER_MAX_STRING_LEN ) {
    return -1;
  }

  bzero ( buffer, OA_SER_MAX_STRING_LEN+1 );
  ( void ) snprintf ( buffer, OA_SER_MAX_STRING_LEN+1, "%s",
      header.Instrument );
  if ( write ( context->SERfd, buffer, OA_SER_MAX_STRING_LEN ) !=
      OA_SER_MAX_STRING_LEN ) {
    return -1;
  }

  bzero ( buffer, OA_SER_MAX_STRING_LEN+1 );
  ( void ) snprintf ( buffer, OA_SER_MAX_STRING_LEN+1, "%s",
      header.Telescope );
  if ( write ( context->SERfd, buffer, OA_SER_MAX_STRING_LEN ) !=
      OA_SER_MAX_STRING_LEN ) {
    return -1;
  }

  _oaSERInitMicrosoftTimestamp();
  now = _oaSERGetMicrosoftTimestamp ( 0, 0 );
  _oaSER64BitToLittleEndian ( now, ( uint8_t* ) buffer );
  if ( write ( context->SERfd, buffer, 8 ) != 8 ) {
    return -1;
  }

  now = _oaSERGetMicrosoftTimestamp ( 0, 1 );
  _oaSER64BitToLittleEndian ( now, ( uint8_t* ) buffer );
  if ( write ( context->SERfd, buffer, 8 ) != 8 ) {
    return -1;
  }

  return 0;
}


int
oaSERWriteFrame ( oaSERContext* context, void* frame, const char* timestampStr )
{
  uint8_t  buffer[8];
  void*    writeableData = frame;
  uint8_t* s;
  uint8_t* t;
  unsigned int i, ret2;
  int      ret;

  if ( timestampStr && *timestampStr ) {
    int64_t now = _oaSERGetMicrosoftTimestamp ( timestampStr, 0 );
    _oaSER64BitToLittleEndian ( now, buffer );
  } else {
    int64_t now = _oaSERGetMicrosoftTimestamp ( 0, 0 );
    _oaSER64BitToLittleEndian ( now, buffer );
  }

  if ( context->pixelDepth < 8 ) {
    t = context->transformBuffer;
    s = frame;
    for ( i = 0; i < context->frameSize; i++ ) {
      *t++ = *s << context->shiftBits;
    }
    writeableData = context->transformBuffer;
  }

  if (( ret = write ( context->SERfd, writeableData,
      context->frameSize )) < 0 ) {
    return -1;
  }
  ret2 = ( unsigned int ) ret;
  if ( ret2 != context->frameSize ) {
    return -1;
  }
  context->frames++;

  bcopy ( buffer, context->nextTimestamp, sizeof ( int64_t ));
  context->nextTimestamp++;
  if ( !--context->framesLeft ) {
    context->bufferSize += TIMESTAMP_BLOCK_COUNT * sizeof ( int64_t );
    if (!( context->timestampBuffer = realloc ( context->timestampBuffer,
        context->bufferSize ))) {
      return -1;
    }
    context->nextTimestamp = context->timestampBuffer + context->frames;
    context->framesLeft = TIMESTAMP_BLOCK_COUNT;
  }
  return 0;
}


int
oaSERWriteTrailer ( oaSERContext* context )
{
  uint32_t bufferSize;
  uint8_t  buffer[4];
  int      ret;
  uint32_t ret2;

  bufferSize = context->frames * sizeof ( int64_t );
  if (( ret = write ( context->SERfd, context->timestampBuffer,
      bufferSize )) < 0 ) {
    return -1;
  }
  ret2 = ( uint32_t ) ret;
  if ( ret2 != bufferSize ) {
    return -1;
  }
  if ( !lseek64 ( context->SERfd, FRAME_COUNT_POSN, SEEK_SET )) {
    return -1;
  }
  _oaSER32BitToLittleEndian ( context->frames, buffer );
  if ( write ( context->SERfd, buffer, 4 ) != 4 ) {
    return -1;
  }
  return 0;
}


int
oaSERClose ( oaSERContext* context )
{
  close ( context->SERfd );
  free ( context->timestampBuffer );
  free ( context->transformBuffer );
  context->SERfd = -1;
  context->timestampBuffer = 0;
  context->transformBuffer = 0;
  return 0;
}


static void
_oaSERInitMicrosoftTimestamp()
{
  struct tm      *tmp;
  time_t         now;

  now = time(0);
  tmp = localtime ( &now );
  _offsetFromGMT = tmp->tm_gmtoff * ticksPerSecond;
}


static int64_t
_oaSERGetMicrosoftTimestamp ( const char* timestamp, int utc )
{
  int64_t        stamp;
  struct timeval tv;
  struct tm      tm;
  int            millisecs;

  if ( timestamp ) {
    // timestamp is CCYY-MM-DDThh:mm:ss.sss
    ( void ) sscanf ( timestamp, "%d-%d-%dT%d:%d:%d.%d", &tm.tm_year,
        &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min, &tm.tm_sec,
        &millisecs );
    tm.tm_mon--;
    tm.tm_year -= 1900;
    tm.tm_isdst = tm.tm_wday = tm.tm_yday = -1;
    tv.tv_sec = timegm ( &tm );
    tv.tv_usec = millisecs * 1000;
  } else {
    gettimeofday ( &tv, 0 );
  }
  stamp = tv.tv_sec * ticksPerSecond + tv.tv_usec * ticksPerMicrosecond +
      epochTicks;
  if ( !utc ) {
    stamp += _offsetFromGMT;
  }

  return stamp;
}


static void
_oaSER32BitToLittleEndian ( int32_t val, uint8_t* buf )
{
  _oaSERnToLittleEndian ( val, buf, 4 );
}


static void
_oaSER64BitToLittleEndian ( int64_t val, uint8_t* buf )
{
  _oaSERnToLittleEndian ( val, buf, 8 );
}


static void
_oaSERnToLittleEndian ( int64_t val, uint8_t* buf, uint8_t len )
{
  while ( len-- > 0 ) {
    *buf++ = val & 0xff;
    val >>= 8;
  }
}
