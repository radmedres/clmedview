AUTOMAKE_OPTIONS                = subdir-objects
AM_CFLAGS                       = -Iinclude/                                   \
                                  -I../libmemory/include/                      \
                                  -I../libcommon/include/                      \
                                  -I../libconfiguration/include/               \
                                  -I../plugin/

lib_LTLIBRARIES                 = libpixeldata.la libpixeldata-plugin.la 

libpixeldata_la_LDFLAGS         = -module -no-undefined -avoid-version
libpixeldata_la_SOURCES         = src/libpixeldata.c

libpixeldata_plugin_la_LDFLAGS  = -module -no-undefined -avoid-version
libpixeldata_plugin_la_SOURCES  = src/libpixeldata-plugin.c
