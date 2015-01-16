#include "plugin-line-interface.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "lib-common-unused.h"

// Including a C file is the intended behavior here. The functions in
// plugin-interface should be compiled into the shared library to avoid
// linking or runtime complexities.
#include "plugin-interface.c"

#include "lib-common-debug.c"
#include "lib-common-list.c"

static Coordinate *ac_1 = NULL;
static Coordinate *ac_2 = NULL;


void
CLM_Plugin_Metadata (char **name, unsigned char **icon, int *version, PluginType *type)
{
  *name = calloc (1, 18);
  memcpy (*name, "line-tool-polygon", 17);
  *version = 1;
  *type = PLUGIN_TYPE_LINE;

  // The icon can be exported with GIMP as "C source file".
  *icon = calloc (1, 16 * 16 * 4 + 2);
  memcpy (*icon,
    "ZZZJYYY\327YYY\302ZZZ\37\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0WWW\327VVV\377VVV\377UUU\255"
    "WWW/fff\5\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0TTT\313TTT\377TTT\377TTT\377TTT\377TTT\370TTT\314TTT\232SSShUUU6```\10"
    "\377\377\377\0\377\377\377\0ZZZ\21SSS1\377\377\377\0RRR\37RRR\360QQQ\302"
    "PPP#SSSMSSS~SSS\260RRR\341RRR\377RRR\377RRR\373RRR\323SSS\252RRR\360RRR\377"
    "RRR\217\377\377\377\0PPP\340QQQ\230\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0QQQ\23PPPCQQQuPPP\246PPP\351PPP\377PPP\377"
    "PPP\351\377\377\377\0OOO\340MMM\230\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0MMM+NNN\364NNN\377NNN\231\377\377\377\0LLL\340KKK\230\377\377"
    "\377\0\377\377\377\0\377\377\377\0KKK\21MMMcKKK)\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0LLL\233LLL\345\0\0\0\1\377\377\377\0III\340"
    "JJJ\230\377\377\377\0\377\377\377\0\0\0\0\1III\331III\377III\377KKK)\377"
    "\377\377\0\377\377\377\0\377\377\377\0JJJ\230III\340\377\377\377\0\377\377"
    "\377\0GGG\340FFF\230\377\377\377\0\377\377\377\0GGGpGGG\377GGG\377GGG\377"
    "HHHr\377\377\377\0\377\377\377\0\377\377\377\0FFF\230GGG\340\377\377\377"
    "\0\377\377\377\0EEE\340EEE\230III\7EEE\233EEE\377EEE\333EEE\377EEE\377FF"
    "F\347EEE0\377\377\377\0\377\377\377\0EEE\230EEE\340\377\377\377\0DDDGCCC"
    "\367DDD\333CCC\312CCC\373CCCs\377\377\377\0DDD\36EEE%CCC\326CCC\361CCCA\377"
    "\377\377\0CCC\230CCC\340\377\377\377\0AAA\337AAA\377AAA\377AAA\354AAA?\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0CCC\23AAA\307AAA\370"
    "BBBUAAA\230AAA\340\377\377\377\0???\276???\377???\377???z\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0FFF\13"
    "???\267???\375???\337???\360>>>!>>>%>>>\235<<<\203777\16\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0""333\5===\304===\377===\377===\316\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0:::\215:::"
    "\377:::\377:::\327\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0""555\35""777\270888\327777J",
    16 * 16 * 4 + 1);
}


static void
CLM_Plugin_BresenhamLine (UNUSED PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                          Coordinate ts_Start, Coordinate ts_End, unsigned int ui32_DrawValue,
                          PixelAction te_Action)
{
  /*--------------------------------------------------------------------------.
   | The following is an optimized version of the bresenham line algorithm.   |
   | It basically calculates a straight line of pixels for two given points.  |
   | For each pixel it selects, fp_BrushCallback will be called.              |
   '--------------------------------------------------------------------------*/

  int i32_DifferenceInX = abs (ts_End.x - ts_Start.x);
  int i32_DirectionOfX = (ts_Start.x < ts_End.x) ? 1 : -1;

  int i32_DifferenceInY = abs (ts_End.y - ts_Start.y);
  int i32_DirectionOfY = (ts_Start.y < ts_End.y) ? 1 : -1; 

  int i32_Error = ((i32_DifferenceInX > i32_DifferenceInY) ? i32_DifferenceInX : -i32_DifferenceInY) / 2;
  int i32_PreviousError;

  for (;;)
  {
    if (!CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_Start, ui32_DrawValue, te_Action)) break;
    if (ts_Start.x == ts_End.x && ts_Start.y == ts_End.y) break;

    i32_PreviousError = i32_Error;

    if (i32_PreviousError >- i32_DifferenceInX) { i32_Error -= i32_DifferenceInY; ts_Start.x += i32_DirectionOfX; }
    if (i32_PreviousError < i32_DifferenceInY)  { i32_Error += i32_DifferenceInX; ts_Start.y += i32_DirectionOfY; }
  }
}


void
CLM_Plugin_Line_Apply (PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                       Coordinate ts_Point, List **pll_Coordinates, unsigned int ui32_DrawValue,
                       PixelAction te_Action)
{
  assert (pll_Coordinates != NULL);

  CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_Point, ui32_DrawValue, te_Action);

  // When the coordinates have been reset, reset the internal variables too.
  if (*pll_Coordinates == NULL)
  {
    ac_1 = NULL;
    ac_2 = NULL;
  }
  
  if (ac_1 != NULL && ac_2 != NULL)
  {
    (te_Action == ACTION_ERASE)
      ? CLM_Plugin_BresenhamLine (ps_Original, ps_Mask, ps_Selection, *ac_1, *ac_2, ui32_DrawValue, ACTION_SET)
      : CLM_Plugin_BresenhamLine (ps_Original, ps_Mask, ps_Selection, *ac_1, *ac_2, ui32_DrawValue, ACTION_ERASE);
  }

  // Add the new coordinate.
  Coordinate *ts_NewCoordinate = calloc (1, sizeof (Coordinate));
  memcpy (ts_NewCoordinate, &ts_Point, sizeof (Coordinate));
  *pll_Coordinates = list_prepend (*pll_Coordinates, (void *)ts_NewCoordinate);

  List *pll_Points = *pll_Coordinates;

  // Draw all previous lines.
  List *pll_LastElement = NULL;
  while (pll_Points != NULL)
  {
    Coordinate *ts_CurrentPoint = pll_Points->data;
    assert (ts_CurrentPoint != NULL);

    if (pll_LastElement != NULL)
    {
      Coordinate *ts_LastCoordinate = pll_LastElement->data;
      CLM_Plugin_BresenhamLine (ps_Original, ps_Mask, ps_Selection, *ts_LastCoordinate,
				*ts_CurrentPoint, ui32_DrawValue, te_Action);
    }

    pll_LastElement = pll_Points;
    pll_Points = list_next (pll_Points);
  }

  ac_1 = (*pll_Coordinates)->data;
  ac_2 = pll_LastElement->data;

  CLM_Plugin_BresenhamLine (ps_Original, ps_Mask, ps_Selection, *ac_1, *ac_2,
			    ui32_DrawValue, te_Action);
}
