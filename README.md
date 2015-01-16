clmedview
=========

This project is an attempt to create a cross-platform medical image viewer
specialized in drawing "regions of interests".

First off, the project aims to create a useful toolkit for displaying and
interacting with medical images. Currently, only some NIFTII file types are
implemented. Feel free to add (or request) support for other formats.

Secondly, the project aims to provide a useful graphical user interface.


Dependencies
------------
Make sure you have the following libraries, development packages and build 
tools installed:

* GCC or CLANG
* Automake
* Autoconf
* Make
* Gtk+-3.0 and GLib-2.0
* Clutter and Clutter-Gtk
* zlib
* Guile 2.0

To build the documentation you need some more programs:
* Texinfo
* Dia
* Doxygen
* Dot (for diagrams inside Doxygen output)


Build instructions for GNU/Linux
--------------------------------
When you have resolved the dependencies listed above you can build 
the program by running:
<pre>
autoreconf -i
./configure
make
</pre>

To compile with CLANG:
<pre>
autoreconf -i
./configure CC=clang
make
</pre>

Additionally you can add compiler flags:
<pre>
autoreconf -i
./configure CFLAGS="-Wall -O2 -march=native"
make
</pre>

Optionally you can generate developer documentation using Doxygen.
<pre>
make docs-doxygen
</pre>


Debugging options
-----------------

The following compiler flags enable debug messages:

* -DENABLE_DEBUG_FUNCTIONS: Print the name of a function when called.
* -DENABLE_DEBUG_EVENTS:    Print the name of an event handler when called.
* -DENABLE_DEBUG_EXTRA:     Print extra debug messages.
* -DENABLE_DEBUG_WARNING:   Print warning messages.
* -DENABLE_DEBUG_ERROR:     Print error messages.
* -DENABLE_DEBUG:           Enable all of the above.

Typically you can enable debugging by running:
<pre>
./configure CFLAGS="-DENABLE_DEBUG"
</pre>

To use mtrace as alternative to valgrind, you should define ENABLE_MTRACE:
<pre>
./configure CFLAGS="-g -DENABLE_MTRACE"
</pre>

You need to set the MALLOC_TRACE environment variable to a file:
<pre>
export MALLOC_TRACE=mtrace.out
</pre>

Running mtrace after your program has finished returns a list of memory leaks:
<pre>
mtrace clmedview mtrace.out
</pre>


Build instructions for Windows
------------------------------

WARNING: The Windows build currently doesn't work.

1. Install MSYS2 from [MSYS2](https://msys2.github.io)
2. Resolve the dependencies for this project.
3. Follow the instructions for GNU/Linux.


Build instructions for Mac OSX
------------------------------

1. Install MacPorts from [MacPorts](https://www.macports.org/)
2. Resolve the dependencies for this project.
3. Follow the instructions for GNU/Linux.


License
-------

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.
