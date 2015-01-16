AUTOMAKE_OPTIONS 	= subdir-objects
SUBDIRS                 = src/brushes/ src/line-tools/ src/selection-tools/

if OS_LINUX
LIBRARY_EXTENSION       = so
else
LIBRARY_EXTENSION       = dll
endif

all-local:
	@mkdir -p brushes/ line-tools/ selection-tools/;
	@cp src/brushes/.libs/lib*.$(LIBRARY_EXTENSION) brushes/.
	@cp src/line-tools/.libs/lib*.$(LIBRARY_EXTENSION) line-tools/.
	@cp src/selection-tools/.libs/lib*.$(LIBRARY_EXTENSION) selection-tools/.

clean-local:
	@rm -rf brushes/*.$(LIBRARY_EXTENSION) line-tools/*.$(LIBRARY_EXTENSION) selection-tools/*.$(LIBRARY_EXTENSION)
