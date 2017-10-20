//gcc --std=c99 -g3 -O0 -I./ -I../include test_oaconvert.c yuv.c grey.c oavideo.c -o test_oaconvert
//or `head -n 1 test_oaconvert.c | cut -c 3-`
// ./test_oaconvert 2>&1 | tee test_oaconvert.txt; LINE=`grep at\ line test_oaconvert.txt | sed -e 's/.* at line //g'`; head -n $LINE test_oaconvert.c | tail -n 1
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <openastro/video.h>
#include <openastro/video/formats.h>
#include "yuv.h"
#include "grey.h"


#define TARGET_LENGTH 24

uint8_t source[] = { 0x01, 0x23, 0x45, 0x67,
                     0x89, 0xab, 0xcd, 0xef,
                     0x01, 0x23, 0x45, 0x67,
                     0x89, 0xab, 0xcd, 0xef,
                     0x01, 0x23, 0x45, 0x67,
                     0x89, 0xab, 0xcd, 0xef };

uint8_t empty[] =  { 0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00,
                     0x00, 0x00, 0x00, 0x00 };

void print_array(uint8_t* buf, int len, int sep)
{
  int i;
  for ( i = 0; i < len; ++i )
  {
    if (i == sep)
      fprintf(stderr, " ... ");
    fprintf(stderr, "%02x ", *(buf+i));
  }
  fprintf(stderr, "\n");
}

void assert(int fmt, uint8_t expected[], uint8_t* target, int length, int check_nulls, int LINE)
{
  fprintf(stderr, ".");
  if (0 != memcmp(target, expected, length)) {
    fprintf(stderr, "F\n");
    fprintf(stderr, "Conversion does not match expected at line %d (fmt==%d)\n", LINE, fmt);
    fprintf(stderr, "Expected: "); print_array(expected, length, -1);
    fprintf(stderr, "Actual:   "); print_array(target, length, -1);
    exit(-1);
  }
  if (check_nulls && 0 != memcmp(empty, target+length, TARGET_LENGTH-length)) {
    fprintf(stderr, "F\n");
    fprintf(stderr, "Conversion exceeds length of expected (%d) at line %d (fmt==%d)\n", length, LINE, fmt);
    print_array(target, TARGET_LENGTH, length);
    exit(-1);
  }
}

/**
 * Test a conversion from pixel format S to pixel format T
 *
 * fmt - an index if running in a loop, to provide useful debug on failure
 * S - source pixel format
 * T - target pixel format
 * e - pointer to array of expected result
 * l - length of expected result
 * L - line at which the function (macro) was invokved
 */
void test_convert(int fmt, int S, int T, uint8_t* e, int l, int L)
{
    uint8_t* t = (uint8_t*) malloc(TARGET_LENGTH);

    // Test conversion from one buffer to another
    memset(t, 0, TARGET_LENGTH);
    oaconvert(source, t, 2, 2, S, T);
    assert(fmt, e, t, l, 1, L);

    // Test in-place conversion
    memcpy(t, source, TARGET_LENGTH);
    oaconvert(t, t, 2, 2, S, T);
    assert(fmt, e, t, l, 0, L);

    free(t);
}

#define EXPECTED(...) __VA_ARGS__
#define TEST_CONVERT(fmt, S, T, expected) \
do {\
  uint8_t e[] = expected;\
  test_convert(fmt, S, T, e, sizeof(e), __LINE__);\
} while (0);



/* TODO
#define OA_PIX_FMT_YUV444P 		22
#define OA_PIX_FMT_YUV422P 		23
#define OA_PIX_FMT_YUV420P		24
#define OA_PIX_FMT_YUV410P		25
#define OA_PIX_FMT_YUYV  		27
#define OA_PIX_FMT_UYVY  		28
#define OA_PIX_FMT_YUV420 		29
#define OA_PIX_FMT_YUV411 		30
#define OA_PIX_FMT_YUV410 		31
*/


void test_8()
{
  int fmt;
  uint8_t FORMATS_8[] = { OA_PIX_FMT_GREY8,
                          OA_PIX_FMT_BGGR8,
                          OA_PIX_FMT_RGGB8,
                          OA_PIX_FMT_GBRG8,
                          OA_PIX_FMT_GRBG8 };

  fprintf(stderr, "Running tests in %s", __FUNCTION__);

  // 8bit
  for ( fmt=0; fmt < sizeof(FORMATS_8); ++fmt ) {
    TEST_CONVERT(fmt, FORMATS_8[fmt], OA_PIX_FMT_GREY8,    EXPECTED({0x01, 0x23, 0x45, 0x67}));
    TEST_CONVERT(fmt, FORMATS_8[fmt], OA_PIX_FMT_GREY16LE, EXPECTED({0x00, 0x01, 0x00, 0x23, 0x00, 0x45, 0x00, 0x67}));
    TEST_CONVERT(fmt, FORMATS_8[fmt], OA_PIX_FMT_GREY16BE, EXPECTED({0x01, 0x00, 0x23, 0x00, 0x45, 0x00, 0x67, 0x00}));
  }

  fprintf(stderr, "\n");
}

void test_10()
{
  int fmt;
  uint8_t FORMATS_10P[]  = { //OA_PIX_FMT_BGGR10P,
                             //OA_PIX_FMT_RGGB10P,
                             //OA_PIX_FMT_GBRG10P,
                             OA_PIX_FMT_GRBG10P };

  uint8_t FORMATS_10[]   = { //OA_PIX_FMT_BGGR10,
                             //OA_PIX_FMT_RGGB10,
                             //OA_PIX_FMT_GBRG10,
                             OA_PIX_FMT_GRBG10 };

  fprintf(stderr, "Running tests in %s", __FUNCTION__);

  // 10bit packed
  for ( fmt=0; fmt < sizeof(FORMATS_10P); ++fmt ) {
    TEST_CONVERT(fmt, FORMATS_10P[fmt], OA_PIX_FMT_GREY8,    EXPECTED({0x01, 0x23, 0x45, 0x67}));
    TEST_CONVERT(fmt, FORMATS_10P[fmt], OA_PIX_FMT_GREY16LE, EXPECTED({0x40, 0x01, 0x80, 0x23, 0x00, 0x45, 0x80, 0x67}));
    TEST_CONVERT(fmt, FORMATS_10P[fmt], OA_PIX_FMT_GREY16BE, EXPECTED({0x01, 0x40, 0x23, 0x80, 0x45, 0x00, 0x67, 0x80}));
  }

  // 10bit
  for ( fmt=0; fmt < sizeof(FORMATS_10); ++fmt ) {
    TEST_CONVERT(fmt, FORMATS_10[fmt], OA_PIX_FMT_GREY8,    EXPECTED({0xc0, 0xd1, 0xe2, 0xf3}));
    TEST_CONVERT(fmt, FORMATS_10[fmt], OA_PIX_FMT_GREY16LE, EXPECTED({0x40, 0xc0, 0x40, 0xd1, 0x40, 0xe2, 0x40, 0xf3}));
    TEST_CONVERT(fmt, FORMATS_10[fmt], OA_PIX_FMT_GREY16BE, EXPECTED({0xc0, 0x40, 0xd1, 0x40, 0xe2, 0x40, 0xf3, 0x40}));
  }

  fprintf(stderr, "\n");
}

void test_16 ()
{
  int fmt;
  uint8_t FORMATS_16LE[] = { OA_PIX_FMT_GREY16LE,
                             OA_PIX_FMT_BGGR16LE,
                             OA_PIX_FMT_RGGB16LE,
                             OA_PIX_FMT_GBRG16LE,
                             OA_PIX_FMT_GRBG16LE };

  uint8_t FORMATS_16BE[] = { OA_PIX_FMT_GREY16BE,
                             OA_PIX_FMT_BGGR16BE,
                             OA_PIX_FMT_RGGB16BE,
                             OA_PIX_FMT_GBRG16BE,
                             OA_PIX_FMT_GRBG16BE };

  fprintf(stderr, "Running tests in %s", __FUNCTION__);

  // 16bit LE
  for ( fmt=0; fmt < sizeof(FORMATS_16LE); ++fmt ) {
    TEST_CONVERT(fmt, FORMATS_16LE[fmt], OA_PIX_FMT_GREY8,    EXPECTED({0x23, 0x67, 0xab, 0xef}));
    TEST_CONVERT(fmt, FORMATS_16LE[fmt], OA_PIX_FMT_GREY16LE, EXPECTED({0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}));
    TEST_CONVERT(fmt, FORMATS_16LE[fmt], OA_PIX_FMT_GREY16BE, EXPECTED({0x23, 0x01, 0x67, 0x45, 0xab, 0x89, 0xef, 0xcd}));
  }

  // 16bit BE
  for ( fmt=0; fmt < sizeof(FORMATS_16BE); ++fmt ) {
    TEST_CONVERT(fmt, FORMATS_16BE[fmt], OA_PIX_FMT_GREY8,    EXPECTED({0x01, 0x45, 0x89, 0xcd}));
    TEST_CONVERT(fmt, FORMATS_16BE[fmt], OA_PIX_FMT_GREY16LE, EXPECTED({0x23, 0x01, 0x67, 0x45, 0xab, 0x89, 0xef, 0xcd}));
    TEST_CONVERT(fmt, FORMATS_16BE[fmt], OA_PIX_FMT_GREY16BE, EXPECTED({0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef}));
  }

  fprintf(stderr, "\n");
}


void test_rgb24()
{
  fprintf(stderr, "Running tests in %s", __FUNCTION__);

  // RGB24
  TEST_CONVERT(0, OA_PIX_FMT_RGB24, OA_PIX_FMT_GREY8,    EXPECTED({0x1e, 0x84, 0xd6, 0x40}));
  TEST_CONVERT(0, OA_PIX_FMT_RGB24, OA_PIX_FMT_GREY16LE, EXPECTED({0x39, 0x1e, 0x39, 0x84, 0x96, 0xd6, 0x39, 0x40}));
  TEST_CONVERT(0, OA_PIX_FMT_RGB24, OA_PIX_FMT_GREY16BE, EXPECTED({0x1e, 0x39, 0x84, 0x39, 0xd6, 0x96, 0x40, 0x39}));

  // BGR24
  TEST_CONVERT(0, OA_PIX_FMT_BGR24, OA_PIX_FMT_GREY8,    EXPECTED({0x27, 0x8d, 0xb9, 0x49}));
  TEST_CONVERT(0, OA_PIX_FMT_BGR24, OA_PIX_FMT_GREY16LE, EXPECTED({0xc6, 0x27, 0xc6, 0x8d, 0xf2, 0xb9, 0xc6, 0x49}));
  TEST_CONVERT(0, OA_PIX_FMT_BGR24, OA_PIX_FMT_GREY16BE, EXPECTED({0x27, 0xc6, 0x8d, 0xc6, 0xb9, 0xf2, 0x49, 0xc6}));

  fprintf(stderr, "\n");
}

void test_rgb48()
{
  fprintf(stderr, "Running tests in %s", __FUNCTION__);

  // RGB48
  TEST_CONVERT(0, OA_PIX_FMT_RGB48LE, OA_PIX_FMT_GREY8,    EXPECTED({0x5d, 0x53, 0xd2, 0xa1}));
  TEST_CONVERT(0, OA_PIX_FMT_RGB48LE, OA_PIX_FMT_GREY16LE, EXPECTED({0xaf, 0x5d, 0x78, 0x53, 0x80, 0xd2, 0xf3, 0xa1}));
  TEST_CONVERT(0, OA_PIX_FMT_RGB48LE, OA_PIX_FMT_GREY16BE, EXPECTED({0x5d, 0xaf, 0x53, 0x78, 0xd2, 0x80, 0xa1, 0xf3}));

  TEST_CONVERT(0, OA_PIX_FMT_RGB48BE, OA_PIX_FMT_GREY8,    EXPECTED({0x3b, 0x31, 0xb0, 0x80}));
  TEST_CONVERT(0, OA_PIX_FMT_RGB48BE, OA_PIX_FMT_GREY16LE, EXPECTED({0xd1, 0x3b, 0x9a, 0x31, 0xa2, 0xb0, 0x15, 0x80}));
  TEST_CONVERT(0, OA_PIX_FMT_RGB48BE, OA_PIX_FMT_GREY16BE, EXPECTED({0x3b, 0xd1, 0x31, 0x9a, 0xb0, 0xa2, 0x80, 0x15}));

  // BGR48
  TEST_CONVERT(0, OA_PIX_FMT_BGR48LE, OA_PIX_FMT_GREY8,    EXPECTED({0x70, 0x40, 0xbf, 0xb5}));
  TEST_CONVERT(0, OA_PIX_FMT_BGR48LE, OA_PIX_FMT_GREY16LE, EXPECTED({0xda, 0x70, 0x4d, 0x40, 0x55, 0xbf, 0x1e, 0xb5}));
  TEST_CONVERT(0, OA_PIX_FMT_BGR48LE, OA_PIX_FMT_GREY16BE, EXPECTED({0x70, 0xda, 0x40, 0x4d, 0xbf, 0x55, 0xb5, 0x1e}));

  TEST_CONVERT(0, OA_PIX_FMT_BGR48BE, OA_PIX_FMT_GREY8,    EXPECTED({0x4e, 0x1e, 0x9d, 0x93}));
  TEST_CONVERT(0, OA_PIX_FMT_BGR48BE, OA_PIX_FMT_GREY16LE, EXPECTED({0xfc, 0x4e, 0x6f, 0x1e, 0x77, 0x9d, 0x40, 0x93}));
  TEST_CONVERT(0, OA_PIX_FMT_BGR48BE, OA_PIX_FMT_GREY16BE, EXPECTED({0x4e, 0xfc, 0x1e, 0x6f, 0x9d, 0x77, 0x93, 0x40}));

  fprintf(stderr, "\n");
}

int main()
{
  test_8();
  test_10();
  test_16();
  test_rgb24();
  test_rgb48();
}

