WHAT IT IS
==========

A GStreamer plugin to detect qrcode in frames, based on [quirc](https://github.com/dlbeer/quirc/)

qrcodedec is a video filter that decodes qrcode found in frame
and emit "qrcode" signal with decoded string.

From version 0.2, the element generate messages named `qrcode`.
The structure containes these fields:

`GstClockTime timestamp`: the timestamp of the buffer that triggered the message.
`gchar symbol`: the deteted bar code data.

Video buffer is passed untouched to the src pad.

It supports only raw RGB video at 320x240px


Compile
-------

    $ ./autogen.sh
    $ ./configure
    $ make
    $ make install


Compile devel
-------------

    $ ./autogen.sh
    $ ./configure --prefix=$HOME
    $ make
    $ make install

Debug run
---------

    $ GST_PLUGIN_PATH=$HOME/.gstreamer-1.0/ gst-launch-1.0 --gst-debug=qrcodedec:9 v4l2src ! videoconvert ! qrcodedec ! videoconvert ! gtksink

