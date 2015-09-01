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

ACLOCAL_AMFLAGS         = -I m4
AUTOMAKE_OPTIONS 	= subdir-objects
SUBDIRS                 = libcommon/                                           \
                          libconfiguration/                                    \
                          libmemory/                                           \
                          libpixeldata/                                        \
                          libviewer/                                           \
                          libio/                                               \
                          libhistogram/                                        \
                          plugin/                                              \
                          .

AM_CFLAGS               = $(gtk_CFLAGS)                                        \
                          $(glib_CFLAGS)                                       \
                          $(clutter_CFLAGS)                                    \
                          $(cluttergtk_CFLAGS)                                 \
                          $(cairo_CFLAGS)                                      \
                          $(uuid_CFLAGS)                                       \
                          $(zlib_CFLAGS)                                       \
                          -Ilibcommon/include/                                 \
                          -Ilibconfiguration/include/                          \
                          -Ilibpixeldata/include/                              \
                          -Ilibviewer/include/                                 \
                          -Ilibmemory/include/                                 \
                          -Ilibio/include/                                     \
                          -Ilibhistogram/include/                              \
                          -Iplugin/include/

EXTERNAL_LIBS           = $(gtk_LIBS)                                          \
                          $(glib_LIBS)                                         \
                          $(zlib_LIBS)                                         \
                          $(uuid_LIBS)                                         \
                          $(clutter_LIBS)                                      \
                          $(cluttergtk_LIBS)                                   \
                          $(cairo_LIBS)

COMMON_LIBS             = libcommon/libcommon-algebra.la                       \
                          libcommon/libcommon-debug.la                         \
                          libcommon/libcommon-history.la                       \
                          libcommon/libcommon-list.la                          \
                          libcommon/libcommon-tree.la

CONFIGURATION_LIBS      = libconfiguration/libconfiguration.la

PIXELDATA_LIBS          = libpixeldata/libpixeldata.la                         \
			  libpixeldata/libpixeldata-plugin.la

VIEWER_LIBS             = libviewer/libviewer.la

MEMORY_LIBS             = libmemory/libmemory-patient.la                       \
                          libmemory/libmemory-serie.la                         \
                          libmemory/libmemory-slice.la                         \
                          libmemory/libmemory-study.la                         \
                          libmemory/libmemory-tree.la                          \
			  libmemory/libmemory-io.la

IO_LIBS                 = libio/libio-dicom.la                                 \
                          libio/libio-nifti.la

HISTOGRAM_LIBS          = libhistogram/libhistogram.la

LUT_FILES               = luts/*.lut

DOXYGEN_FILES           = doc/Doxyfile doc/custom.css doc/footer.html          \
                          doc/doxygen/html/background.png                      \
                          doc/images/component-diagram.dia                     \
                          doc/clmedview.texi

clmedview_SOURCES	= source/main.c                                        \
                          source/gui/mainwindow.c                              \
                          source/gui/mainwindow.h

clmedview_LDFLAGS       = -static-libtool-libs
clmedview_LDADD         = $(EXTERNAL_LIBS)                                     \
                          $(COMMON_LIBS)                                       \
                          $(CONFIGURATION_LIBS)                                \
                          $(PIXELDATA_LIBS)                                    \
                          $(MEMORY_LIBS)                                       \
                          $(IO_LIBS)                                           \
                          $(VIEWER_LIBS)                                       \
                          $(HISTOGRAM_LIBS)                                    \
                          -lm -ldl

dist_data_DATA          = $(PLUGIN_FILES) $(DOXYGEN_FILES) $(LUT_FILES)

bin_PROGRAMS 		= clmedview

CLEAN_TARGET            = docs-clean

# Be silent unless VERBOSE is defined.
$(VERBOSE).SILENT:

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool

if OS_LINUX
clmedview_LDADD	       += 
else
clmedview_LDADD	       += -lshell32
clmedview_LDFLAGS      += -mwindows -static
endif

if ENABLE_CREF_OPTION
clmedview_LDFLAGS      += -Xlinker -Map=$(bin_PROGRAMS).map -Xlinker --cref
CLEAN_TARGET           += map-clean
endif

if ENABLE_GCOV_OPTION
AM_CFLAGS	       += -fprofile-arcs -ftest-coverage
CLEAN_TARGET           += gcov-clean
endif

if ENABLE_GPROF_OPTION
AM_CFLAGS              += -p
endif

if ENABLE_MTRACE_OPTION
AM_CFLAGS              += -DENABLE_MTRACE
endif

clean-local: $(CLEAN_TARGET)

cref-graph:
	$(SH) scripts/cref-graph.sh $(bin_PROGRAMS).map $(bin_PROGRAMS).dot

gcov-clean:
	@$(RM) -rf *.gcno *.gcda */*.gcno */*.gcda */*/*.gcno */*/*.gcda

map-clean:
	@$(RM) -f $(bin_PROGRAMS).map

docs-preprocess-images:
	@cd doc/images/; dia component-diagram.dia -e component-diagram.eps;   \
	ps2pdf -dEPSCrop component-diagram.eps component-diagram.pdf;          \
	dia grel-mechanism.dia -e grel-mechanism.eps;                          \
	ps2pdf -dEPSCrop grel-mechanism.eps grel-mechanism.pdf;                \
	dia grel-sequence-diagram.dia -e grel-sequence-diagram.eps;            \
	ps2pdf -dEPSCrop grel-sequence-diagram.eps grel-sequence-diagram.pdf

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
	-@$(RM) -rf doc/*.aux doc/*.cp doc/*.fn doc/*.ky doc/*.log doc/*.pg doc/*.toc doc/*.tp doc/*.vr

.PHONY: gcov-clean docs-doxygen docs-clean
