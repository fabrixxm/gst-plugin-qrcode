# Note: plugindir is set in configure

plugin_LTLIBRARIES = libgstqrcodedec.la


## Plugin 1

# sources used to compile this plug-in
libgstqrcodedec_la_SOURCES = gstqrcodedec.c gstqrcodedec.h

# compiler and linker flags used to compile this plugin, set in configure.ac
libgstqrcodedec_la_CFLAGS = $(GST_CFLAGS)
libgstqrcodedec_la_LIBADD = $(GST_LIBS) -lquirc
libgstqrcodedec_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
libgstqrcodedec_la_LIBTOOLFLAGS = --tag=disable-static

### Plugin 2 (audio filter example)
#
## sources used to compile this plug-in
#libgstaudiofilterexample_la_SOURCES = gstaudiofilter.c
#
## compiler and linker flags used to compile this plugin, set in configure.ac
#libgstaudiofilterexample_la_CFLAGS = $(GST_CFLAGS)
#libgstaudiofilterexample_la_LIBADD = $(GST_LIBS)
#libgstaudiofilterexample_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS)
#libgstaudiofilterexample_la_LIBTOOLFLAGS = --tag=disable-static

# headers we need but don't want installed
noinst_HEADERS = gstqrcodedec.h
