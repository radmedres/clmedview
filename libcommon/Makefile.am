AUTOMAKE_OPTIONS              = subdir-objects
AM_CFLAGS                     = -Iinclude/

lib_LTLIBRARIES               = libcommon-algebra.la    \
                                libcommon-debug.la      \
                                libcommon-history.la    \
                                libcommon-list.la       \
                                libcommon-tree.la

libcommon_algebra_la_LDFLAGS  = -module -no-undefined -avoid-version
libcommon_algebra_la_SOURCES  = src/libcommon-algebra.c

libcommon_debug_la_LDFLAGS    = -module -no-undefined -avoid-version
libcommon_debug_la_SOURCES    = src/libcommon-debug.c

libcommon_history_la_LDFLAGS  = -module -no-undefined -avoid-version
libcommon_history_la_SOURCES  = src/libcommon-history.c

libcommon_list_la_LDFLAGS     = -module -no-undefined -avoid-version
libcommon_list_la_SOURCES     = src/libcommon-list.c

libcommon_tree_la_LDFLAGS     = -module -no-undefined -avoid-version
libcommon_tree_la_SOURCES     = src/libcommon-tree.c
