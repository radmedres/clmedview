#include "plugin-brush-interface.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lib-common-unused.h"

// Including a C file is the intended behavior here. The functions in
// plugin-interface should be compiled into the shared library to avoid
// linking or runtime complexities.
#include "plugin-interface.c"

void
CLM_Plugin_Metadata (char **name, unsigned char **icon, int *version, PluginType *type)
{
  *name = calloc (1, 13);
  memcpy (*name, "fill-brush", 12);
  *version = 1;
  *type = PLUGIN_TYPE_BRUSH;
  *icon = calloc (1, 16 * 16 * 4 + 2);
  memcpy (*icon,
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0WWW&ZZZ\346YYYV\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0YYYPZZZ\377ZZZ\210\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0YYY<WWW[XXX\377X"
    "XX\210\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0UUU<VVV\363UUU`VVV\377VVV\210UUU\25\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0UUU<UUU\363UUU\377UUU`UUU\377TTT\210UUU\277S"
    "SS(\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "UUU$TTTFTTT^SSS\363SSS\377SSS\377RRR`SSS\377SSS\210SSS\330SSS\347SSS(\377"
    "\377\377\0\377\377\377\0\200\200\200\2RRR\230RRR\377RRR\377RRR\377RRR\377"
    "RRR\377RRR\377RRR]RRR\377RRR\206QQQ\330RRR\377RRR\347SSS(\377\377\377\0Q"
    "QQ<PPP\377PPP\377PPP\377PPP\377PPP\377PPP\377PPP\377QQQLQQQ\216RRR;PPP\365"
    "PPP\377PPP\377OOO\347NNN'OOO~OOO\377NNNlNNN\352OOO\377OOO\377OOO\377OOO\377"
    "OOO\362OOO\253OOO\343OOO\377OOO\377OOO\377OOO\377OOO\261MMM\300MMM\352UU"
    "U\3KKK,MMM\351MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MMM\377MM"
    "M\377MMM\377MMM\337LLL\364LLL\257\377\377\377\0\377\377\377\0MMM+LLL\351"
    "LLL\377LLL\377LLL\377LLL\377LLL\377LLL\377LLL\377LLL\377LLL\377LLLrJJJ\370"
    "JJJ\242\377\377\377\0\377\377\377\0\377\377\377\0III*JJJ\350JJJ\377JJJ\377"
    "JJJ\377JJJ\377JJJ\377JJJ\377JJJ\377III\217\377\377\377\0HHH\370III\227\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0III*III\350HHH\377HH"
    "H\377HHH\377HHH\377HHH\377GGG\217\377\377\377\0\377\377\377\0GGG\370GGG\215"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0DD"
    "D)GGG\347GGG\377GGG\377GGG\377GGG\217\377\377\377\0\377\377\377\0\377\377"
    "\377\0EEE\370EEE\201\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0HHH'EEE\265EEE\345FFFu\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0DDD\277EEEC\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0", 16 * 16 * 4 + 1);
}


void
CLM_Plugin_Brush_Apply (UNUSED PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                        Coordinate ts_Point, UNUSED unsigned int ui32_BrushScale,
                        unsigned int ui32_DrawValue, PixelAction te_Action)
{
  assert (ps_Original != NULL);
  assert (ps_Mask != NULL);

  Slice *mask_slice = PIXELDATA_ACTIVE_SLICE (ps_Mask);
  assert (mask_slice != NULL);

  if (ts_Point.x > mask_slice->matrix.i16_x || ts_Point.y > mask_slice->matrix.i16_y) return;

  int i32_CntArray;
  int i32_CntKernel;

  short int i16_Value;

  int i32_lengthArrayToRead = 1;

  Coordinate *p_ArrayToReadFrom = calloc (1, PIXELDATA_ACTIVE_SLICE (ps_Original)->matrix.i16_x *
                                             PIXELDATA_ACTIVE_SLICE (ps_Original)->matrix.i16_y *
                                             sizeof (Coordinate));
  Coordinate *p_PointInArray = NULL;

  Coordinate *ps_Kernel = calloc (1, 4 * sizeof (Coordinate));

  Coordinate ts_PixelPoint;

  p_ArrayToReadFrom[0].x = ts_Point.x;
  p_ArrayToReadFrom[0].y = ts_Point.y;


  i32_lengthArrayToRead=1;
  for(i32_CntArray=0;i32_CntArray<i32_lengthArrayToRead; i32_CntArray++)
  {
    p_PointInArray = &p_ArrayToReadFrom[i32_CntArray];

    ps_Kernel[0].x = p_PointInArray->x;
    ps_Kernel[0].y = p_PointInArray->y + 1;

    ps_Kernel[1].x = p_PointInArray->x - 1;
    ps_Kernel[1].y = p_PointInArray->y;

    ps_Kernel[2].x = p_PointInArray->x + 1;
    ps_Kernel[2].y = p_PointInArray->y;

    ps_Kernel[3].x = p_PointInArray->x;
    ps_Kernel[3].y = p_PointInArray->y - 1;

    for(i32_CntKernel=0; i32_CntKernel<4; i32_CntKernel++)
    {
      ts_PixelPoint = ps_Kernel[i32_CntKernel];

      if ((ts_PixelPoint.x >= 0) && (ts_PixelPoint.x < PIXELDATA_ACTIVE_SLICE (ps_Mask)->matrix.i16_x) &&
          (ts_PixelPoint.y >= 0) && (ts_PixelPoint.y < PIXELDATA_ACTIVE_SLICE (ps_Mask)->matrix.i16_y))
      {
        if (CLM_Plugin_GetPixelAtPoint (ps_Mask, ts_PixelPoint, &i16_Value) && i16_Value == 0)
        {
          CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_PixelPoint , ui32_DrawValue, te_Action);

          p_ArrayToReadFrom[i32_lengthArrayToRead] = ts_PixelPoint;

          i32_lengthArrayToRead++;
        }
      }
    }
  }

  free (p_ArrayToReadFrom);
  p_ArrayToReadFrom = NULL;

  free (ps_Kernel);
  ps_Kernel = NULL;
}
