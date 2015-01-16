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

SUBDIRS                 = . plugin
ACLOCAL_AMFLAGS         = -I m4
AUTOMAKE_OPTIONS 	= subdir-objects
AM_CFLAGS 		= $(gtk_CFLAGS) $(glib_CFLAGS) $(zlib_CFLAGS) \
                          $(guile_CFLAGS) $(clutter_CFLAGS) $(vte_CFLAGS) \
                          $(cluttergtk_CFLAGS) -Iinclude/ \
                          -Ilib/lib-common/include/ -Iplugin/
bin_PROGRAMS 		= clmedview

INCLUDE_SOURCES 	= include/lib-pixeldata.h \
			  include/lib-pixeldata-plugin.h \
			  include/lib-viewer.h \
			  include/lib-common.h \
			  include/lib-memory.h \
			  include/lib-common-history.h \
			  include/lib-memory-io.h \
			  include/lib-memory-patient.h \
			  include/lib-memory-study.h \
			  include/lib-memory-serie.h \
			  include/lib-memory-slice.h \
			  include/lib-memory-quaternion.h \
			  include/lib-memory-tree.h \
			  include/lib-common-list.h \
			  include/lib-common-tree.h \
			  include/lib-common-debug.h \
			  include/lib-common-unused.h \
			  include/lib-grel-shell.h \
			  include/lib-grel-interpreter.h \
			  include/lib-configuration.h

PLUGIN_FILES		= plugin/Makefile.am \
			  plugin/plugin-interface.h \
			  plugin/plugin-interface.c \
			  plugin/src/brushes/Makefile.am \
			  plugin/src/brushes/plugin-brush-interface.h \
			  plugin/src/brushes/libpen.c \
			  plugin/src/brushes/libfill.c \
			  plugin/src/brushes/libsobel.c \
			  plugin/src/line-tools/Makefile.am \
			  plugin/src/line-tools/plugin-line-interface.h \
			  plugin/src/line-tools/libpolygon.c \
			  plugin/src/selection-tools/Makefile.am \
			  plugin/src/selection-tools/plugin-select-interface.h \
			  plugin/src/selection-tools/librectangle.c

GREL_FILES              = grel-scripts/init.scm

LUT_FILES               = luts/*.lut

DOXYGEN_FILES           = doc/Doxyfile doc/custom.css doc/footer.html \
                          doc/doxygen/html/background.png \
                          doc/images/component-diagram.dia \
                          doc/images/grel-mechanism.dia \
                          doc/images/grel-sequence-diagram.dia \
                          doc/clmedview.texi

LIB_MEMORY_SOURCES	= lib/lib-memory/include/lib-io-niftii.h \
			  lib/lib-memory/include/nifti1.h \
			  lib/lib-memory/include/lib-io-niftii.h \
			  lib/lib-memory/lib-memory-tree.c \
			  lib/lib-memory/lib-memory-patient.c \
			  lib/lib-memory/lib-memory-study.c \
			  lib/lib-memory/lib-memory-serie.c \
			  lib/lib-memory/lib-memory-slice.c \
			  lib/lib-memory/lib-memory-quaternion.c \
			  lib/lib-memory/lib-io-niftii.c \
			  lib/lib-memory/lib-memory-io.c

LIB_COMMON_SOURCES	= lib/lib-common/lib-common-list.c \
			  lib/lib-common/lib-common-tree.c \
			  lib/lib-common/lib-common-history.c \
			  lib/lib-common/lib-common-debug.c

LIB_PIXELDATA_SOURCES	= lib/lib-pixeldata/lib-pixeldata.c \
			  lib/lib-pixeldata/lib-pixeldata-plugin.c

LIB_VIEWER_SOURCES	= lib/lib-viewer/lib-viewer.c

LIB_GREL_SOURCES        = lib/lib-grel/lib-grel-shell.c \
                          lib/lib-grel/lib-grel-interpreter.c

LIB_CONFIGURATION_SOURCES = lib/lib-configuration/lib-configuration.c

clmedview_SOURCES	= source/main.c \
			  source/gui/mainwindow.c \
			  source/gui/mainwindow.h \
			  $(INCLUDE_SOURCES) $(LIB_MEMORY_SOURCES) \
			  $(LIB_COMMON_SOURCES) $(LIB_PIXELDATA_SOURCES) \
			  $(LIB_VIEWER_SOURCES) $(LIB_GREL_SOURCES) \
			  $(LIB_CONFIGURATION_SOURCES)

clmedview_LDFLAGS       = -fdata-sections -ffunction-sections -Wl,--gc-sections
clmedview_LDADD         = $(gtk_LIBS) $(glib_LIBS) $(zlib_LIBS) $(guile_LIBS) \
                          $(clutter_LIBS) $(cluttergtk_LIBS) $(vte_LIBS) -lm -ldl

dist_data_DATA          = $(PLUGIN_FILES) $(GREL_FILES) $(DOXYGEN_FILES) $(LUT_FILES)

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

if OS_LINUX
clmedview_LDADD	       += 
else
clmedview_LDADD	       += -lshell32
clmedview_LDFLAGS      += -mwindows -static
endif

if ENABLE_GCOV_OPTION
AM_CFLAGS	       += -fprofile-arcs -ftest-coverage
endif

if ENABLE_GPROF_OPTION
AM_CFLAGS              += -p
endif

if ENABLE_MTRACE_OPTION
AM_CFLAGS              += -DENABLE_MTRACE
endif

clean-local: docs-clean plugins-clean

plugins:
	@cd plugin/; make;

plugins-clean:
	@cd plugin/; make clean

gcov-clean:
	@rm -rf *.gcno *.gcda */*.gcno */*.gcda */*/*.gcno */*/*.gcda

docs-preprocess-images:
	@cd doc/images/; dia component-diagram.dia -e component-diagram.eps; \
	ps2pdf -dEPSCrop component-diagram.eps component-diagram.pdf
	@cd doc/images/; dia grel-sequence-diagram.dia -e grel-sequence-diagram.eps; \
	ps2pdf -dEPSCrop grel-sequence-diagram.eps grel-sequence-diagram.pdf
	@cd doc/images/; dia grel-mechanism.dia -e grel-mechanism.eps; \
	ps2pdf -dEPSCrop grel-mechanism.eps grel-mechanism.pdf

docs: docs-preprocess-images
	@texi2pdf -q doc/clmedview.texi -o doc/clmedview.pdf
	@$(RM) *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr
	@$(RM) doc/images/*.pdf doc/images/*.eps

docs-doxygen:
	@cd doc/; doxygen

docs-clean:
	-@mv doc/doxygen/html/background.png .;
	-@$(RM) -rf doc/doxygen/;
	-@mkdir -p doc/doxygen/html/;
	-@mv background.png doc/doxygen/html/;

tests:
	@echo "Compiling tests.."
	@echo ""
	@cd test/; make > /dev/null
	@cd test/; bin/lib-memory-history
	@echo ""
	@cd test/; bin/lib-common-list
	@echo ""
	@cd test/; bin/lib-common-tree
	@echo ""

.PHONY: gcov-clean docs-doxygen docs-clean plugins plugins-clean tests
