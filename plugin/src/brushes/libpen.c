#include "plugin-brush-interface.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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

void
CLM_Plugin_Metadata (char **name, unsigned char **icon, int *version, PluginType *type)
{
  *name = calloc (1, 13);
  memcpy (*name, "pencil-brush", 12);
  *version = 1;
  *type = PLUGIN_TYPE_BRUSH;
  *icon = calloc (1, 16 * 16 * 4 + 2);
  memcpy (*icon,
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0]]]!YYY\320YYY\225UUU\14\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0XXX\40WWW\316ZZZ\"YYY9VVV\303UUU\30"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0RRR\37UUU\326TTT\333"
    "TTT\206@@@\4UUU\33UUU\253\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0RRR\34SSS\325"
    "RRR\377RRR\377RRR\374RRR\203PPP#RRR\311\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0LLL\33PPP\325"
    "PPP\377PPP\377PPP\377PPP\377PPP\331PPP\314OOO\35\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0NNN\32NNN\324N"
    "NN\377NNN\377NNN\377NNN\377NNN\377NNN\324OOO\35\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0GGG\31LLL\323L"
    "LL\377LLL\377LLL\377LLL\377LLL\377KKK\325MMM\36\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0NNN\27III\323I"
    "II\377III\377III\377III\377III\377III\325DDD\36\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0FFF\26GGG\323G"
    "GG\377GGG\377GGG\377GGG\377GGG\377HHH\325DDD\36\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0III\25EEE\322E"
    "EE\377EEE\377EEE\377EEE\377EEE\377EEE\326HHH\40\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0JJJ\21CCC\345C"
    "CC\377CCC\377CCC\377CCC\377CCC\377CCC\326HHH\40\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "@@@g@@@\215@@@\344AAA\374AAA\377AAA\377AAA\327HHH\40\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0>>>\214\377\377\377\0FFF\11>>>\344???\377???\330>>>!\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0<<<(<<<b\377\377\377\0\377\377\377\0<<"
    "<s<<<\353666!\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0:::\305"
    ":::^:::N:::\204:::t;;;\37\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0""888\364888\3138887\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0",
    16 * 16 * 4 + 1);
}


void
CLM_Plugin_Brush_Apply (UNUSED PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                        Coordinate ts_Point, unsigned int ui32_BrushScale, unsigned int ui32_DrawValue,
                        PixelAction te_Action)
{
  int i32_LowerValue;
  int i32_UpperValue;

  if (ui32_BrushScale > 1)
  {
    ui32_BrushScale = ui32_BrushScale / 2;
    i32_LowerValue = -1 * ui32_BrushScale;
    i32_UpperValue = ui32_BrushScale;
  }
  else
  {
    CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_Point, ui32_DrawValue, te_Action);
    return;
  }

  // Center the brush position.
  ts_Point.y = ts_Point.y - ui32_BrushScale - 1;

  int i32_Counter, i32_RowCounter = 1;
  for (i32_Counter = i32_LowerValue; i32_Counter < i32_UpperValue; i32_Counter++)
  {
    int i32_ColumnCounter;
    for (i32_ColumnCounter = i32_LowerValue; i32_ColumnCounter < i32_UpperValue; i32_ColumnCounter++)
    {
      Coordinate ts_DrawPoint;
      ts_DrawPoint.x = ts_Point.x + i32_ColumnCounter;
      ts_DrawPoint.y = ts_Point.y + i32_RowCounter;

      CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_DrawPoint, ui32_DrawValue, te_Action);
    }

    i32_RowCounter++;
  }
}
