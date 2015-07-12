AUTOMAKE_OPTIONS              = subdir-objects
AM_CFLAGS                     = -Iinclude/                                     \
                                -I../plugin/                                   \
                                -I../libcommon/include/                        \
                                -I../libmemory/include/                        \
                                -I../libpixeldata/include/                     \
                                $(glib_CFLAGS)                                 \
                                $(clutter_CFLAGS)                              \
                                $(cluttergtk_CFLAGS)

lib_LTLIBRARIES               = libviewer.la

libviewer_la_LDFLAGS          = -module -no-undefined -avoid-version
libviewer_la_SOURCES          = src/libviewer.c
