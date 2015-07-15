AUTOMAKE_OPTIONS              = subdir-objects
AM_CFLAGS                     = -Iinclude/                                     \
                                -I../libmemory/include/                        \
                                -I../libcommon/include/                        \
                                $(cairo_CFLAGS)

lib_LTLIBRARIES               = libhistogram.la

libhistogram_la_LDFLAGS       = -module -no-undefined -avoid-version
libhistogram_la_SOURCES       = src/libhistogram.c
