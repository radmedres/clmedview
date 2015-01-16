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
  memcpy (*name, "empty-brush", 12);
  *version = 1;
  *type = PLUGIN_TYPE_BRUSH;
  *icon = calloc (1, 16 * 16 * 4 + 2);
  memcpy (*icon,
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0nnn\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0nqj\0\0\0\0\0\0\0\0\0TWT\20TWS%UXR\26\0\0\0\0\0\0\0\0\0\0\0\0MfM\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0YYN\0\0\0\0\0\0\0\0\0UXR;SUQ\253RTP\306TUQ"
    "\301SUQ\311SUR\264UWR_#\263(\1\0\0\0\0UUU\0UUU\0\0\0\0\0QQQ\0\0\0\0\0\0\0"
    "\0\0VWR\247TVR\356moj\304\201\202}\247\203\204\177\223\200\202|\205moj\177"
    "YZV\241UVS\225TTU+\0\0\0\0\0\0\0\0@@@\0\0\0\0\0IGL\4VWS\241^`\\\355\205\210"
    "\203\277\206\211\205\255\204\206\201\240\205\206\200\221\205\207\203\177"
    "\207\212\205k\215\220\212Vmpke[[Y\177TXT7\0\0\0\0\0\0\0\0KPF\1SUQkbd_\366"
    "}\200{\325ilg\331egb\337gkk\346_gl\345agg\316bc\\\265ikg\207\207\212\207"
    "J\203\207\203=bd_YY[V$?=7\1XXW\24]^Z\345kmh\362Z[X\364\224\224\222\377x\207"
    "\234\3776V\204\3772T\206\3773R~\377ix\214\377vwt\364RTP\314kpkb\226\224\215"
    "%jjd6QQN\11VVS]^`\\\377Y[W\371\310\310\307\377\300\312\331\377)U\222\377"
    "Eu\257\3774c\242\377&S\220\377(P\213\377\272\306\330\377\274\275\273\377"
    "OPM\355ce_k\240\236\223\24||y\6Z[X\270\\^Z\377\235\236\234\377\375\374\372"
    "\377Kn\237\377Ao\252\377w\244\323\377Gw\261\3771b\241\377\32E\203\377e\202"
    "\254\377\374\375\377\377\321\321\321\377[]Y\365]_Zjrto\10\\^[\230dfb\372"
    "\305\305\305\376\377\377\377\377$N\212\377.\\\232\377Aq\256\3775f\246\377"
    ".^\234\377\34G\204\377Hk\235\377\363\365\371\377\377\377\377\377\307\310"
    "\307\377gie\377W[Uaifb\3Y[W\272\275\275\274\377\377\377\377\377Jl\236\377"
    "\35H\207\377'T\222\377(T\221\377\"M\212\377\31D\203\377f\203\255\377\367"
    "\371\373\377\376\376\376\377\331\331\331\377npm\377VXSp\0\0\0\0QRNZuvs\377"
    "\357\356\356\377\317\331\347\377\26A\201\377\37I\206\377\40J\207\377\37I"
    "\206\377%N\211\377\277\313\336\377\377\377\377\377\334\334\333\377oqm\377"
    "UWS\211VVQ\10\0\0\0\0RPV\10UVS\230Z\\X\357\264\264\263\377\244\261\303\377"
    "2V\213\377%M\206\3770U\212\377\250\266\312\377\332\332\330\377\225\226\223"
    "\374MPK\353VYV{K7Z\2\0\0\0\0UUU\0\0\0\0\0\0\0\0\0TWTMMNJ\237PPI\276ee^\311"
    "fln\321pph\315WXQ\305GIE\263ORN\204WUS!\0\0\0\0\0\0\0\0UUU\0\0\0\0\0UUU\0"
    "`kZ\0\0\0\0\0\0\0\0\0VTO\14OQM\35OOG$LNJ\"OSN\30k\201Q\1\0\0\0\0\0\0\0\0"
    "VVS\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0", 16 * 16 * 4 + 1);
}


void
CLM_Plugin_Brush_Apply (UNUSED PixelData *ps_Original, UNUSED PixelData *ps_Mask,
                        UNUSED PixelData *ps_Selection, UNUSED Coordinate ts_Point,
                        UNUSED unsigned int ui32_BrushScale, UNUSED unsigned int ui32_DrawValue,
                        UNUSED PixelAction te_Action)
{
  // Do absolutely nothing.
  return;
}
