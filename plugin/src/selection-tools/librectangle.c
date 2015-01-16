#include "plugin-select-interface.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>

#include "lib-common-unused.h"

// Including a C file is the intended behavior here. The functions in
// plugin-interface should be compiled into the shared library to avoid
// linking or runtime complexities.
#include "plugin-interface.c"

void
CLM_Plugin_Metadata (char **name, unsigned char **icon, int *version, PluginType *type)
{
  *name = calloc (1, 25);
  memcpy (*name, "selection-tool-rectangle", 24);
  *version = 1;
  *type = PLUGIN_TYPE_SELECTION;

  // The icon can be exported with GIMP as "C source file".
  *icon = calloc (1, 16 * 16 * 4 + 2);
  memcpy (*icon,
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0ZZZ\325[[[\243ZZZ\216\377\377\377\0ZZZc[[[\230[[[\230[[[*[[[*[[[\230"
    "[[[\230ZZZc\377\377\377\0ZZZ\216[[[\243ZZZ\325VVV\377VVV\253VVV\216\377\377"
    "\377\0UUUcVVV\230VVV\230UUU*UUU*VVV\230VVV\230UUUc\377\377\377\0VVV\216V"
    "VV\253VVV\377SSS\377UUU0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0UUU0SSS\377PPPPUUU\17\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0UUU\17PPPPMMMxQQQ\26\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0QQQ\26MMMxJJJ\377"
    "JJJ0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0JJJ0JJJ\377GGG\377EEE0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "EEE0GGG\377DDDxFFF\26\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0FFF\26DDDx@@@PDDD\17\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0DDD\17@@@P===\377:::0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0:::0===\377:::\377::"
    ":\253999\216\377\377\377\0;;;c;;;\230;;;\230===*===*;;;\230;;;\230;;;c\377"
    "\377\377\0""999\216:::\253:::\377777\325777\243888\216\377\377\377\0""66"
    "6c777\230777\230777*777*777\230777\230666c\377\377\377\0""888\216777\243"
    "777\325\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377"
    "\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377"
    "\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377"
    "\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0\377\377\377\0"
    "\377\377\377\0", 16 * 16 * 4 + 1);
}

void
CLM_Plugin_Selection_Apply (UNUSED PixelData *ps_Original, PixelData *ps_Mask, PixelData *ps_Selection,
                            Coordinate ts_Start, Coordinate ts_Current, unsigned int ui32_DrawValue,
                            PixelAction te_Action)
{
  assert (ps_Original != NULL);
  assert (ps_Mask != NULL);

  Slice *selection_slice = PIXELDATA_ACTIVE_SLICE (ps_Mask);
  assert (selection_slice != NULL);

  {
    int i32_RowCount, i32_ColumnCount;
    Coordinate ts_Position;

    for (i32_RowCount = 0; i32_RowCount < selection_slice->matrix.y; i32_RowCount++)
    {
      for (i32_ColumnCount = 0; i32_ColumnCount < selection_slice->matrix.x; i32_ColumnCount++)
      {
        ts_Position.x = i32_ColumnCount;
        ts_Position.y = i32_RowCount;

        CLM_Plugin_DrawPixelAtPoint (ps_Mask, ps_Selection, ts_Position, ui32_DrawValue, ACTION_ERASE);
      }
    }
  }

  Coordinate ts_TopLeft;
  ts_TopLeft.x = (ts_Start.x > ts_Current.x) ? ts_Current.x : ts_Start.x;
  ts_TopLeft.y = (ts_Start.y > ts_Current.y) ? ts_Current.y : ts_Start.y;

  Coordinate ts_DownRight;
  ts_DownRight.x = (ts_Start.x > ts_Current.x) ? ts_Start.x : ts_Current.x;
  ts_DownRight.y = (ts_Start.y > ts_Current.y) ? ts_Start.y : ts_Current.y;

  float f_TopLeftY = ts_TopLeft.y;

  while (ts_TopLeft.x <= ts_DownRight.x)
  {
    while (ts_TopLeft.y <= ts_DownRight.y)
    {
      CLM_Plugin_DrawPixelAtPoint (ps_Mask, NULL, ts_TopLeft, ui32_DrawValue, te_Action);
      ts_TopLeft.y += 1;
    }

    CLM_Plugin_DrawPixelAtPoint (ps_Mask, NULL, ts_TopLeft, ui32_DrawValue, te_Action);
    ts_TopLeft.x += 1;

    // Restore original Y position.
    ts_TopLeft.y = f_TopLeftY;
  }
}
