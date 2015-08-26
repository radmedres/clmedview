AUTOMAKE_OPTIONS 	= subdir-objects
AM_CFLAGS               = -Iinclude/                                           \
                          -I../libcommon/include/                              \
                          -I../libpixeldata/include/                           \
                          -I../libmemory/include/

lib_LTLIBRARIES         = empty.la                                             \
                          fill.la                                              \
                          pen.la                                               \
                          sobel.la                                             \
                          polygon.la

empty_la_LDFLAGS        = -module -no-undefined -avoid-version
empty_la_SOURCES        = src/empty.c src/plugin-interface.c

fill_la_LDFLAGS         = -module -no-undefined -avoid-version
fill_la_SOURCES         = src/fill.c src/plugin-interface.c

pen_la_LDFLAGS          = -module -no-undefined -avoid-version
pen_la_SOURCES          = src/pen.c src/plugin-interface.c

sobel_la_LDFLAGS        = -module -no-undefined -avoid-version
sobel_la_SOURCES        = src/sobel.c src/plugin-interface.c

polygon_la_LDFLAGS      = -module -no-undefined -avoid-version
polygon_la_SOURCES      = src/polygon.c src/plugin-interface.c ../libcommon/src/libcommon-list.c

all-local: $(lib_LTLIBRARIES)
	@$(RM) pen.so
	@cp .libs/pen.so .
	@$(RM) empty.so
	@cp .libs/empty.so .
	@$(RM) fill.so
	@cp .libs/fill.so .
	@$(RM) sobel.so
	@cp .libs/sobel.so .
	@$(RM) polygon.so
	@cp .libs/polygon.so .

clean-local:
	@$(RM) pen.so
	@$(RM) empty.so
	@$(RM) fill.so
	@$(RM) sobel.so
	@$(RM) polygon.so
