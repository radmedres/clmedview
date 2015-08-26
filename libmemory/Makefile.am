AUTOMAKE_OPTIONS              = subdir-objects
AM_CFLAGS                     = -Iinclude/                                     \
                                -I../libcommon/include/                        \
                                -I../libio/include/

lib_LTLIBRARIES               = libmemory-patient.la                           \
                                libmemory-serie.la                             \
                                libmemory-slice.la                             \
                                libmemory-study.la                             \
                                libmemory-tree.la                              \
				libmemory-io.la

libmemory_patient_la_LDFLAGS  = -module -no-undefined -avoid-version
libmemory_patient_la_SOURCES  = src/libmemory-patient.c

libmemory_serie_la_LDFLAGS    = -module -no-undefined -avoid-version
libmemory_serie_la_SOURCES    = src/libmemory-serie.c

libmemory_slice_la_LDFLAGS    = -module -no-undefined -avoid-version
libmemory_slice_la_SOURCES    = src/libmemory-slice.c

libmemory_study_la_LDFLAGS    = -module -no-undefined -avoid-version
libmemory_study_la_SOURCES    = src/libmemory-study.c

libmemory_tree_la_LDFLAGS     = -module -no-undefined -avoid-version
libmemory_tree_la_SOURCES     = src/libmemory-tree.c

libmemory_io_la_LDFLAGS       = -module -no-undefined -avoid-version
libmemory_io_la_SOURCES       = src/libmemory-io.c src/libmemory-patient.c     \
                                src/libmemory-study.c
