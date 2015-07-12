#
# Copyright (C) 2015 Marc Geerlings <m.geerlings@mumc.nl>
#
# This file is part of clmedview.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.
#

AC_INIT([clmedview], [0.4], [roel@roelserve.com])
AC_CONFIG_AUX_DIR([scripts])
AC_CANONICAL_HOST
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([source/main.c])
AC_PROG_CC

AM_INIT_AUTOMAKE([1.9 tar-pax foreign -Wall -Werror])
AM_SILENT_RULES([yes])
AM_PROG_CC_C_O
AM_PROG_AR([ar])

LT_INIT([dlopen win32-dll])
AC_SUBST([LIBTOOL_DEPS])

AC_HEADER_STDC
AC_CHECK_HEADERS([stdio.h])
AC_CONFIG_FILES([libcommon/Makefile
		 libconfiguration/Makefile
		 libpixeldata/Makefile
		 libviewer/Makefile
		 libmemory/Makefile
		 libio/Makefile
                 plugin/Makefile
                 plugin/src/brushes/Makefile
                 plugin/src/line-tools/Makefile
                 plugin/src/selection-tools/Makefile
		 Makefile])

case $host in
*-linux*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [true])
;;
*-mingw*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [false])
;;
*-msys*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [false])
;;
*-cygwin*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [true])
;;
*-darwin*)
AC_SUBST(OS_LINUX)
AM_CONDITIONAL([OS_LINUX], [true])
;;
*)

AC_MSG_ERROR([clMedView is not (yet) supported on $host.])
esac

AC_SUBST(ENABLE_GCOV_OPTION)
AC_ARG_ENABLE([gcov],
AC_HELP_STRING([--enable-gcov],
[enable gcov support @<:@default=no@:>@]),
AM_CONDITIONAL([ENABLE_GCOV_OPTION], [true]), AM_CONDITIONAL([ENABLE_GCOV_OPTION], [false]))

AC_SUBST(ENABLE_GPROF_OPTION)
AC_ARG_ENABLE([gprof],
AC_HELP_STRING([--enable-gprof],
[enable gprof support @<:@default=no@:>@]),
AM_CONDITIONAL([ENABLE_GPROF_OPTION], [true]), AM_CONDITIONAL([ENABLE_GPROF_OPTION], [false]))

AC_SUBST(ENABLE_MTRACE_OPTION)
AC_ARG_ENABLE([mtrace],
AC_HELP_STRING([--enable-mtrace],
[enable mtrace support @<:@default=no@:>@]),
AM_CONDITIONAL([ENABLE_MTRACE_OPTION], [true]), AM_CONDITIONAL([ENABLE_MTRACE_OPTION], [false]))

PKG_PROG_PKG_CONFIG

PKG_CHECK_MODULES([glib], [glib-2.0])
PKG_CHECK_MODULES([gtk], [gtk+-3.0])
PKG_CHECK_MODULES([clutter], [clutter-1.0])
PKG_CHECK_MODULES([cluttergtk], [clutter-gtk-1.0])
PKG_CHECK_MODULES([zlib], [zlib])
PKG_CHECK_MODULES([uuid], [uuid])

AC_OUTPUT
