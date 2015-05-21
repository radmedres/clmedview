#include "plugin-brush-interface.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// A marker for unused arguments.
#ifdef __GNUC__
#define UNUSED __attribute__((__unused__))
#else
#define UNUSED
#endif

// Including a C file is the intended behavior here. The functions in
// plugin-interface should be compiled into the shared library to avoid
// linking or runtime complexities.
#include "plugin-interface.c"


void v_PixelData_handleUINT8 (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                              Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action);

void v_PixelData_handleINT16 (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                              Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action);

void v_PixelData_handleFLOAT32 (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                                Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action);


void
CLM_Plugin_Metadata (char **name, unsigned char **icon, int *version, PluginType *type)
{
  *name = calloc (1, 12);
  memcpy (*name, "sobel-brush", 11);
  *version = 1;
  *type = PLUGIN_TYPE_BRUSH;

  // The icon can be exported with GIMP as "C source file".
  *icon = calloc (1, 16 * 16 * 4 + 2);
  memcpy (*icon,
    "YYY\377YYY\377YYY\377YYY\377YYY\377YYY\377YYY\377YYY\377YYY\377YYY\377YY"
    "Y\377YYY\377YYY\377YYY\377YYY\377YYY\377WWW\377WWW\274XXX\200XXX\200XXX\200"
    "XXX\200XXX\200XXX\200XXX\200XXX\200XXX\200XXX\200XXX\200XXX\200WWW\274WW"
    "W\377UUU\377VVV\206\200\200\200\2\377\377\377\0\377\377\377\0\377\377\377"
    "\0fff\5\377\377\377\0\377\377\377\0\377\377\377\0SSS(\377\377\377\0III\7"
    "\377\377\377\0TTTyUUU\377RRR\377QQQ\215UUU'\377\377\377\0\377\377\377\0\377"
    "\377\377\0TTTC\377\377\377\0\377\377\377\0]]]\13RRRg\200\200\200\2RRR>@@"
    "@\4RRR\211RRR\377PPP\377QQQ\215QQQXLLL\33\377\377\377\0UUU\30PPP\220\377"
    "\377\377\0\377\377\377\0QQQLPPP\245RRR8PPP}NNN.QQQ\215PPP\377NNN\377NNN\215"
    "NNN\212NNNRMMM\24NNN|NNN\327UUU\17UUU\14NNN\220NNN\343NNN\200OOO\274NNN_"
    "NNN\215NNN\377LLL\377LLL\215LLL\274LLL\211MMM\264LLL\307LLL\371LLLrLLLWL"
    "LL\320LLL\377MMM\334LLL\353LLL\220LLL\215LLL\377JJJ\377JJJ\215JJJ\355JJJ"
    "\301JJJ\360JJJ\377JJJ\377JJJ\336JJJ\252JJJ\377JJJ\377JJJ\377JJJ\376JJJ\307"
    "JJJ\215JJJ\377GGG\377GGG\215GGG\377GGG\376GGG\377GGG\377GGG\377GGG\377GG"
    "G\364GGG\377GGG\377GGG\377GGG\377GGG\375GGG\215GGG\377EEE\377EEE\215EEE\377"
    "EEE\377EEE\377EEE\377EEE\377EEE\377EEE\377EEE\377EEE\377EEE\377EEE\377EE"
    "E\377EEE\215EEE\377CCC\377CCC\206CCC\250CCC\250CCC\250CCC\250CCC\250CCC\250"
    "CCC\250CCC\250CCC\250CCC\250CCC\250CCC\250CCC\206CCC\377AAA\377BBBx\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0BBBxAAA\377???\377@@@x\377\377\377\0\377\377\377\0>>>_<<<&"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0???UAAA/\377\377"
    "\377\0\377\377\377\0@@@x???\377===\377>>>x>>>\204===\270===\345===\312=="
    "=\270===\270===\270===\270===\340===\316===\270===~>>>x===\377:::\377:::"
    "\274999\216;;;\224;;;\224;;;\224;;;\224;;;\224;;;\224;;;\224;;;\224;;;\224"
    ";;;\224:::\215:::\274:::\377888\377888\377888\377888\377888\377888\37788"
    "8\377888\377888\377888\377888\377888\377888\377888\377888\377888\377",
    16 * 16 * 4 + 1);
}


/* GIMP RGBA C-Source image dump (sobel.c) */

void v_PixelData_SobelEdgeDetection (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                                     Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action)
{
  Slice *slice = PIXELDATA_ACTIVE_SLICE (ps_Original);
  assert (slice != NULL);

  // Boundary check for a 10x10 local image.
  if ((ts_Point.x > (slice->matrix.x - 5)) || (ts_Point.x < 5)
      || (ts_Point.y > (slice->matrix.y - 5)) || (ts_Point.y < 5))
  {
    return;
  }

  switch (ps_Original->serie->data_type)
  {
    case MEMORY_TYPE_UINT8   : v_PixelData_handleUINT8(ps_Original, ps_Mask, ps_Selection, ts_Point, ui32_DrawValue, te_Action); break;

    case MEMORY_TYPE_INT16   : v_PixelData_handleINT16(ps_Original, ps_Mask, ps_Selection, ts_Point, ui32_DrawValue, te_Action); break;
    case MEMORY_TYPE_INT32   : break;
    case MEMORY_TYPE_UINT16  : break;
    case MEMORY_TYPE_UINT32  : break;
    case MEMORY_TYPE_FLOAT32 : v_PixelData_handleFLOAT32(ps_Original, ps_Mask, ps_Selection, ts_Point, ui32_DrawValue, te_Action); break;
    case MEMORY_TYPE_FLOAT64 : break;
    default                  : assert (NULL != NULL); break;
  }
}


void
CLM_Plugin_Brush_Apply (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                        Coordinate ts_Point, UNUSED unsigned int ui32_BrushScale, unsigned int ui32_DrawValue,
                        PixelAction te_Action)
{
  // Filter out invalid requests.
  if (ps_Original == NULL || ps_Mask == NULL) return;

  // Apply the sobel edge detection algorithm.
  v_PixelData_SobelEdgeDetection (ps_Original, ps_Mask, ps_Selection, ts_Point, ui32_DrawValue, te_Action);
}


void
v_PixelData_handleUINT8 (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                         Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action)
{
  Slice *slice = PIXELDATA_ACTIVE_SLICE (ps_Original);
  assert (slice != NULL);

  // Boundary check for a 10x10 local image.
  if ((ts_Point.x > (slice->matrix.x - 5)) || (ts_Point.x < 5)
      || (ts_Point.y > (slice->matrix.y - 5)) || (ts_Point.y < 5))
  {
    return;
  }

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;

  int i32_tempValue;

  /* DECLARATIONS FOR INT16 */
  unsigned char u8_tempImage[10][10];
  unsigned char u8_tempImageSmooth[10][10];

  unsigned char u8_GradientX[10][10];
  unsigned char u8_GradientY[10][10];

  unsigned char u8_Gradient[10][10];

  unsigned int u32_AvarageValue = 0;



  // Initialize the data to zero.
  memset (&u8_tempImage,       0, 100 * sizeof (unsigned char));
  memset (&u8_tempImageSmooth, 0, 100 * sizeof (unsigned char));
  memset (&u8_GradientX,       0, 100 * sizeof (unsigned char));
  memset (&u8_GradientY,       0, 100 * sizeof (unsigned char));
  memset (&u8_Gradient,        0, 100 * sizeof (unsigned char));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  u32_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (ts_Point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = ts_Point.x + X_Cnt;

      CLM_Plugin_GetPixelAtPoint (ps_Original, ts_BlobCount, &(u8_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      u32_AvarageValue += u8_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  u32_AvarageValue = u32_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      i32_tempValue =
        // first row kernel
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +
        (int)(u8_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +

        // second row kernel
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (int)(u8_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (int)(u8_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (int)(u8_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (int)(u8_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        u8_tempImageSmooth[Y_Cnt][X_Cnt] = (short int)(i32_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/


  unsigned char u8_MaxValue = 0;

  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      u8_GradientX[Y_Cnt][X_Cnt] =
        (u8_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * u8_tempImageSmooth[Y_Cnt][X_Cnt + 1] + u8_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (u8_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * u8_tempImageSmooth[Y_Cnt][X_Cnt + 1] + u8_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      u8_GradientY[Y_Cnt][X_Cnt] =
        (u8_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * u8_tempImageSmooth[Y_Cnt - 1][X_Cnt] + u8_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (u8_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * u8_tempImageSmooth[Y_Cnt + 1][X_Cnt] + u8_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      u8_Gradient[Y_Cnt][X_Cnt] = sqrt (u8_GradientX[Y_Cnt][X_Cnt] * u8_GradientX[Y_Cnt][X_Cnt] +
                                     u8_GradientY[Y_Cnt][X_Cnt] * u8_GradientY[Y_Cnt][X_Cnt]);

      if (u8_Gradient[Y_Cnt][X_Cnt] > u8_MaxValue)
      {
        u8_MaxValue = u8_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (u8_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      u8_Gradient[Y_Cnt][X_Cnt] = (u8_Gradient[Y_Cnt][X_Cnt] * 255) / u8_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (ts_Point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = ts_Point.x + X_Cnt;

      if (u8_tempImage[Y_Cnt+5][X_Cnt+5] > u32_AvarageValue)
      {
        CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_BlobCount, ui32_DrawValue, te_Action);
      }
      /* DO NOT ERASE AUTOMATICALLY
         --------------------------
      else
      {
        short int *pi16_SelectionPosition = ((short int *)*ppv_SelectionDataCounter + ts_BlobCount.y + ts_BlobCount.x);
        short int *pi16_ImagePosition = ((short int *)*ppv_ImageDataCounter + ts_BlobCount.y + ts_BlobCount.x);
        *pi16_ImagePosition = 0 & *pi16_SelectionPosition;
      }
      */
    }
  }
}

void
v_PixelData_handleINT16 (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                         Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action)
{
  Slice *slice = PIXELDATA_ACTIVE_SLICE (ps_Original);
  assert (slice != NULL);

  // Boundary check for a 10x10 local image.
  if ((ts_Point.x > (slice->matrix.x - 5)) || (ts_Point.x < 5)
      || (ts_Point.y > (slice->matrix.y - 5)) || (ts_Point.y < 5))
  {
    return;
  }

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;

  int i32_tempValue;

  /* DECLARATIONS FOR INT16 */
  short int i16_tempImage[10][10];
  short int i16_tempImageSmooth[10][10];

  short int i16_GradientX[10][10];
  short int i16_GradientY[10][10];

  short int i16_Gradient[10][10];

  int i32_AvarageValue = 0;



  // Initialize the data to zero.
  memset (&i16_tempImage,       0, 100 * sizeof (short int));
  memset (&i16_tempImageSmooth, 0, 100 * sizeof (short int));
  memset (&i16_GradientX,       0, 100 * sizeof (short int));
  memset (&i16_GradientY,       0, 100 * sizeof (short int));
  memset (&i16_Gradient,        0, 100 * sizeof (short int));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  i32_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (ts_Point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = ts_Point.x + X_Cnt;

      CLM_Plugin_GetPixelAtPoint (ps_Original, ts_BlobCount, &(i16_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      i32_AvarageValue += i16_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  i32_AvarageValue = i32_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      i32_tempValue =
        // first row kernel
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +
        (int)(i16_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +

        // second row kernel
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (int)(i16_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (int)(i16_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (int)(i16_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (int)(i16_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        i16_tempImageSmooth[Y_Cnt][X_Cnt] = (short int)(i32_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/


  short int i16_MaxValue = 0;

  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      i16_GradientX[Y_Cnt][X_Cnt] =
        (i16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * i16_tempImageSmooth[Y_Cnt][X_Cnt + 1] + i16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (i16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * i16_tempImageSmooth[Y_Cnt][X_Cnt + 1] + i16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      i16_GradientY[Y_Cnt][X_Cnt] =
        (i16_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * i16_tempImageSmooth[Y_Cnt - 1][X_Cnt] + i16_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (i16_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * i16_tempImageSmooth[Y_Cnt + 1][X_Cnt] + i16_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      i16_Gradient[Y_Cnt][X_Cnt] = sqrt (i16_GradientX[Y_Cnt][X_Cnt] * i16_GradientX[Y_Cnt][X_Cnt] +
                                     i16_GradientY[Y_Cnt][X_Cnt] * i16_GradientY[Y_Cnt][X_Cnt]);

      if (i16_Gradient[Y_Cnt][X_Cnt] > i16_MaxValue)
      {
        i16_MaxValue = i16_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (i16_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      i16_Gradient[Y_Cnt][X_Cnt] = (i16_Gradient[Y_Cnt][X_Cnt] * 255) / i16_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (ts_Point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = ts_Point.x + X_Cnt;

      if (i16_tempImage[Y_Cnt+5][X_Cnt+5] > i32_AvarageValue)
      {
        CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_BlobCount, ui32_DrawValue, te_Action);
      }
      /* DO NOT ERASE AUTOMATICALLY
         --------------------------
      else
      {
        short int *pi16_SelectionPosition = ((short int *)*ppv_SelectionDataCounter + ts_BlobCount.y + ts_BlobCount.x);
        short int *pi16_ImagePosition = ((short int *)*ppv_ImageDataCounter + ts_BlobCount.y + ts_BlobCount.x);
        *pi16_ImagePosition = 0 & *pi16_SelectionPosition;
      }
      */
    }
  }
}


void
v_PixelData_handleFLOAT32 (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                              Coordinate ts_Point, unsigned char ui32_DrawValue, PixelAction te_Action)
{
  Slice *slice = PIXELDATA_ACTIVE_SLICE (ps_Original);
  assert (slice != NULL);

  // Boundary check for a 10x10 local image.
  if ((ts_Point.x > (slice->matrix.x - 5)) || (ts_Point.x < 5)
      || (ts_Point.y > (slice->matrix.y - 5)) || (ts_Point.y < 5))
  {
    return;
  }

  int X_Cnt, Y_Cnt;
  Coordinate ts_BlobCount;

  double d_tempValue;

  /* DECLARATIONS FOR INT16 */
  float f_tempImage[10][10];
  float f_tempImageSmooth[10][10];

  float f_GradientX[10][10];
  float f_GradientY[10][10];

  float f_Gradient[10][10];

  float f_AvarageValue = 0;

  // Initialize the data to zero.
  memset (&f_tempImage,       0, 100 * sizeof (float));
  memset (&f_tempImageSmooth, 0, 100 * sizeof (float));
  memset (&f_GradientX,       0, 100 * sizeof (float));
  memset (&f_GradientY,       0, 100 * sizeof (float));
  memset (&f_Gradient,        0, 100 * sizeof (float));

  /*--------------------------------------------------------------------------.
   | CREATE LOCAL 10x10 IMAGE                                                 |
   '--------------------------------------------------------------------------*/
  f_AvarageValue = 0;

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (ts_Point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = ts_Point.x + X_Cnt;

      CLM_Plugin_GetPixelAtPoint (ps_Original, ts_BlobCount, &(f_tempImage[Y_Cnt + 5][X_Cnt + 5]));
      f_AvarageValue += f_tempImage[Y_Cnt + 5][X_Cnt + 5];
    }
  }

  f_AvarageValue = f_AvarageValue / 100;

  /*--------------------------------------------------------------------------.
   | SMOOTH WITH GAUSIAN FILTER                                               |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 5 |12 | 15 | 12 | 5 | * 1/159 * IMAGE                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 4 | 9 | 12 |  9 | 4 |                                                  |
   | |~~~+~~~+~~~~+~~~~+~~~|                                                  |
   | | 2 | 4 |  5 |  4 | 2 |                                                  |
   | ~~~~~~~~~~~~~~~~~~~~~~~                                                  |
   '--------------------------------------------------------------------------*/

  for (Y_Cnt = 2; Y_Cnt < 8; Y_Cnt++)
  {
    for (X_Cnt = 2; X_Cnt < 8; X_Cnt++)
    {
      d_tempValue =
        // first row kernel
        (int)(f_tempImage[Y_Cnt-2][X_Cnt-2]) *  2 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt-1]) *  4 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt  ]) *  5 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt+1]) *  4 +
        (int)(f_tempImage[Y_Cnt-2][X_Cnt+2]) *  2 +

        // second row kernel
        (int)(f_tempImage[Y_Cnt-1][X_Cnt-2]) *  4 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt-1]) *  9 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt  ]) * 12 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt+1]) *  9 +
        (int)(f_tempImage[Y_Cnt-1][X_Cnt+2]) *  4 +

        // third row kernel
        (int)(f_tempImage[Y_Cnt  ][X_Cnt-2]) *  5 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt-1]) * 12 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt  ]) * 15 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt+1]) * 12 +
        (int)(f_tempImage[Y_Cnt  ][X_Cnt+2]) *  5 +

        // fourth row kernel
        (int)(f_tempImage[Y_Cnt+1][X_Cnt-2]) *  4 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt-1]) *  9 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt  ]) * 12 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt+1]) *  9 +
        (int)(f_tempImage[Y_Cnt+1][X_Cnt+2]) *  4 +

        // fifth row kernel
        (int)(f_tempImage[Y_Cnt+2][X_Cnt-2]) *  2 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt-1]) *  4 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt  ]) *  5 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt+1]) *  4 +
        (int)(f_tempImage[Y_Cnt+2][X_Cnt+2]) *  2;

        f_tempImageSmooth[Y_Cnt][X_Cnt] = (float)(d_tempValue / 159);

    }
  }


  /*

    ~~~~~~~~~~~~~
    |-1 | 0 | 1 |
    |~~~+~~~+~~~|
    |-2 | 0 | 2 |
    |~~~+~~~+~~~|
    |-1 | 0 | 1 |
    ~~~~~~~~~~~~~
    X i16_Gradient kernel

    ~~~~~~~~~~~~~
    |-1 |-2 |-1 |
    |~~~+~~~+~~~|
    | 0 | 0 | 0 |
    |~~~+~~~+~~~|
    | 1 | 2 | 1 |
    ~~~~~~~~~~~~~

*/


  float f_MaxValue = 0;

  for (Y_Cnt = 1; Y_Cnt < 9; Y_Cnt++)
  {
    for (X_Cnt = 1; X_Cnt < 9; X_Cnt++)
    {
      f_GradientX[Y_Cnt][X_Cnt] =
        (f_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * f_tempImageSmooth[Y_Cnt][X_Cnt + 1] + f_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]) -
        (f_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1] + 2 * f_tempImageSmooth[Y_Cnt][X_Cnt + 1] + f_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      f_GradientY[Y_Cnt][X_Cnt] =
        (f_tempImageSmooth[Y_Cnt - 1][X_Cnt - 1] + 2 * f_tempImageSmooth[Y_Cnt - 1][X_Cnt] + f_tempImageSmooth[Y_Cnt - 1][X_Cnt + 1]) -
        (f_tempImageSmooth[Y_Cnt + 1][X_Cnt - 1] + 2 * f_tempImageSmooth[Y_Cnt + 1][X_Cnt] + f_tempImageSmooth[Y_Cnt + 1][X_Cnt + 1]);

      f_Gradient[Y_Cnt][X_Cnt] = sqrt (f_GradientX[Y_Cnt][X_Cnt] * f_GradientX[Y_Cnt][X_Cnt] +
                                       f_GradientY[Y_Cnt][X_Cnt] * f_GradientY[Y_Cnt][X_Cnt]);

      if (f_Gradient[Y_Cnt][X_Cnt] > f_MaxValue)
      {
        f_MaxValue = f_Gradient[Y_Cnt][X_Cnt];
      }
    }
  }

  // When the maximum value is 0, don't continue.
  // A minimum threshold could be set here.
  if (f_MaxValue == 0) return;

  // NORMALIZE

  for (Y_Cnt = 0; Y_Cnt < 10; Y_Cnt++)
  {
    for (X_Cnt = 0; X_Cnt < 10; X_Cnt++)
    {
      f_Gradient[Y_Cnt][X_Cnt] = (f_Gradient[Y_Cnt][X_Cnt] * 255) / f_MaxValue;
    }
  }

  for (Y_Cnt = -5; Y_Cnt < 5; Y_Cnt++)
  {
    ts_BlobCount.y = (ts_Point.y + Y_Cnt);

    for (X_Cnt = -5; X_Cnt < 5; X_Cnt++)
    {
      ts_BlobCount.x = ts_Point.x + X_Cnt;

      if (f_tempImage[Y_Cnt+5][X_Cnt+5] > f_AvarageValue)
      {
        CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_BlobCount, ui32_DrawValue, te_Action);
      }
      /* DO NOT ERASE AUTOMATICALLY
         --------------------------
      else
      {
        short int *pi16_SelectionPosition = ((short int *)*ppv_SelectionDataCounter + ts_BlobCount.y + ts_BlobCount.x);
        short int *pi16_ImagePosition = ((short int *)*ppv_ImageDataCounter + ts_BlobCount.y + ts_BlobCount.x);
        *pi16_ImagePosition = 0 & *pi16_SelectionPosition;
      }
      */
    }
  }
}
