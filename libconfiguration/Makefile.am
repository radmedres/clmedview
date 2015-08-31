AUTOMAKE_OPTIONS              = subdir-objects
AM_CFLAGS                     = -Iinclude/                              \
                                -I../libcommon/include/			\
                                -I../libmemory/include/

lib_LTLIBRARIES               = libconfiguration.la

libconfiguration_la_LDFLAGS  = -module -no-undefined -avoid-version
libconfiguration_la_SOURCES  = src/libconfiguration.c
