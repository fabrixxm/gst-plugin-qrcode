# Note: plugindir is set in configure

##############################################################################
# TODO: change libgstplugin.la to something else, e.g. libmysomething.la     #
##############################################################################
plugin_LTLIBRARIES = libgstqrcodedec.la

##############################################################################
# TODO: for the next set of variables, name the prefix if you named the .la, #
#  e.g. libmysomething.la => libmysomething_la_SOURCES                       #
#                            libmysomething_la_CFLAGS                        #
#                            libmysomething_la_LIBADD                        #
#                            libmysomething_la_LDFLAGS                       #
##############################################################################

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
