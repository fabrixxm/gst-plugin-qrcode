/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2018 Fabio <<user@hostname.org>>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Alternatively, the contents of this file may be used under the
 * GNU Lesser General Public License Version 2.1 (the "LGPL"), in
 * which case the following provisions apply instead of the ones
 * mentioned above:
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/**
 * SECTION:element-qrcodedec
 *
 * qrcodedec is a video filter that decodes qrcode found in frame
 * and emit "qrcode" signal with decoded string.
 *
 * From version 0.2, the element generate messages named `barcode`.
 * The structure containes these fields:
 *
 * #GstClockTime `timestamp`: the timestamp of the buffer that triggered the message.
 * gchar * `symbol`: the deteted bar code data.
 *
 * Video buffer is passed untouched to the src pad.
 *
 * It supports only raw RGB video at 320x240 px
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! qrcodedec ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gst/gst.h>

#include "gstqrcodedec.h"

GST_DEBUG_CATEGORY_STATIC (gst_qrcode_dec_debug);
#define GST_CAT_DEFAULT gst_qrcode_dec_debug

/* Filter signals and args */
enum
{
  SIGNAL_QRCODE,
  LAST_SIGNAL
};

static guint gst_qrcode_dec_signals[LAST_SIGNAL] = { 0 };

enum
{
  PROP_0,
  PROP_SILENT,
  PROP_MESSAGE
};

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (
	    "video/x-raw, "
	    "format = (string)RGB, "
	    "width = (int)320, "
	    "height = (int)240"
	  )
);

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS (	
        "video/x-raw, "
	    "format = (string)RGB, "
	    "width = (int)320, "
	    "height = (int)240"
	  )
);

#define gst_qrcode_dec_parent_class parent_class
G_DEFINE_TYPE (GstQRCodeDec, gst_qrcode_dec, GST_TYPE_ELEMENT);

static void gst_qrcode_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_qrcode_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_qrcode_dec_sink_event (GstPad * pad, GstObject * parent, GstEvent * event);
static GstFlowReturn gst_qrcode_dec_chain (GstPad * pad, GstObject * parent, GstBuffer * buf);
static GstStateChangeReturn gst_qrcode_dec_change_state (GstElement *element, GstStateChange transition);

/* GObject vmethod implementations */

/* initialize the qrcodedec's class */
static void
gst_qrcode_dec_class_init (GstQRCodeDecClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;

  gobject_class->set_property = gst_qrcode_dec_set_property;
  gobject_class->get_property = gst_qrcode_dec_get_property;

  g_object_class_install_property (gobject_class, PROP_SILENT,
      g_param_spec_boolean ("silent", "Silent", "Produce verbose output ?",
          TRUE, G_PARAM_READWRITE));


  g_object_class_install_property (gobject_class, PROP_MESSAGE,
      g_param_spec_boolean ("message", "message",
          "Post a barcode message for each detected code",
          TRUE, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));


  GType params[] = {G_TYPE_STRING};
  gst_qrcode_dec_signals[SIGNAL_QRCODE] =g_signal_newv (
    "qrcode",
    G_TYPE_FROM_CLASS (klass), 
    G_SIGNAL_RUN_LAST | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
    NULL /* closure */,
    NULL /* accumulator */,
    NULL /* accumulator data */,
    NULL /* C marshaller */,
    G_TYPE_NONE /* return_type */,
    1     /* n_params */,
    params  /* param_types */
  );

  gst_element_class_set_details_simple(gstelement_class,
    "QRCode Decoder",
    "Filter/QRCodeDec",
    "Search for QRCodes in video stream and emits decoded values",
    "Fabio <fabrixxm@kirgroup.com>");

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&src_factory));
  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sink_factory));
      
  gstelement_class->change_state = gst_qrcode_dec_change_state;
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_qrcode_dec_init (GstQRCodeDec * filter)
{
  filter->sinkpad = gst_pad_new_from_static_template (&sink_factory, "sink");
  gst_pad_set_event_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_qrcode_dec_sink_event));
  gst_pad_set_chain_function (filter->sinkpad,
                              GST_DEBUG_FUNCPTR(gst_qrcode_dec_chain));
  GST_PAD_SET_PROXY_CAPS (filter->sinkpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->sinkpad);

  filter->srcpad = gst_pad_new_from_static_template (&src_factory, "src");
  GST_PAD_SET_PROXY_CAPS (filter->srcpad);
  gst_element_add_pad (GST_ELEMENT (filter), filter->srcpad);

  filter->silent = TRUE;
  filter->message = TRUE;
  filter->qr = NULL;
  
}

static void
gst_qrcode_dec_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstQRCodeDec *filter = GST_QRCODEDEC (object);

  switch (prop_id) {
    case PROP_SILENT:
      filter->silent = g_value_get_boolean (value);
      break;
    case PROP_MESSAGE:
      filter->message = g_value_get_boolean (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_qrcode_dec_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstQRCodeDec *filter = GST_QRCODEDEC (object);

  switch (prop_id) {
    case PROP_SILENT:
      g_value_set_boolean (value, filter->silent);
      break;
    case PROP_MESSAGE:
      g_value_set_boolean (value, filter->message);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

/* GstElement vmethod implementations */

/* this function handles sink events */
static gboolean
gst_qrcode_dec_sink_event (GstPad * pad, GstObject * parent, GstEvent * event)
{
  GstQRCodeDec *filter;
  gboolean ret;

  filter = GST_QRCODEDEC (parent);

  GST_LOG_OBJECT (filter, "Received %s event: %" GST_PTR_FORMAT,
      GST_EVENT_TYPE_NAME (event), event);

  switch (GST_EVENT_TYPE (event)) {
    case GST_EVENT_CAPS:
    {
      GstCaps * caps;
      GstStructure *capstruct;
      
      gst_event_parse_caps (event, &caps);
      /* do something with the caps */
  
  
      if (filter->qr != NULL && gst_caps_get_size(caps)>0) {
        capstruct = gst_caps_get_structure(caps, 0);

        GST_LOG_OBJECT (filter, "Caps: %s", gst_structure_to_string(capstruct));
        
        gst_structure_get_int (capstruct, "width", &(filter->width));
        gst_structure_get_int (capstruct, "height", &(filter->height));
        
        GST_LOG_OBJECT (filter, "Frame size: %dx%d\n", filter->width, filter->height);
        
        if (quirc_resize(filter->qr, filter->width, filter->height) < 0)
    	    GST_ERROR_OBJECT (filter, "Failed to allocate qirc video memory");
    	  }
      break;
    }
    default:
      break;
  }
  
  /* and forward */
  ret = gst_pad_event_default (pad, parent, event);
  return ret;
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_qrcode_dec_chain (GstPad * pad, GstObject * parent, GstBuffer * buf)
{
  GstQRCodeDec *filter;

  filter = GST_QRCODEDEC (parent);

  if (filter->qr != NULL) {
    // quirc image and w/h
    uint8_t *image;
    int w, h;

    image = quirc_begin(filter->qr, &w, &h);
    
    // get memory from buffer
    guint nmem=0;
   
    GstMemory *mem = gst_buffer_peek_memory(buf,nmem);
    GstMapInfo info = GST_MAP_INFO_INIT;
    // TODO: use GST_MAP_WRITE and draw qrcode bounding box
    gst_memory_map (mem, &info,GST_MAP_READ);
    
    // grayscale copy of frame into quirc image    
    for (gsize k=0; k<info.size; k+=3 ) {
      guint r = *(info.data + k + 0);
      guint g = *(info.data + k + 1);
      guint b = *(info.data + k + 2);
      guint gray = (r + g + b) / 3;
      *image = gray;
      image++;
    }
    
    // frame end. look for qurcode.
    quirc_end(filter->qr);
    
    
    
    
    // Parse found qrcodes and send event
    int num_codes;
    int i;

    num_codes = quirc_count(filter->qr);
    for (i = 0; i < num_codes; i++) {
      struct quirc_code code;
      struct quirc_data data;
      quirc_decode_error_t err;

      quirc_extract(filter->qr, i, &code);

      /* Decoding stage */
      err = quirc_decode(&code, &data);
      if (!err) {
        GST_LOG_OBJECT (filter, "data: %s", data.payload);

        g_signal_emit(filter, gst_qrcode_dec_signals[SIGNAL_QRCODE],0, data.payload);

        if (filter->message) {
            GstMessage *m;
            GstStructure *s;

            s = gst_structure_new ("qrcode",
                "timestamp", G_TYPE_UINT64, GST_BUFFER_TIMESTAMP (buf),
                //"type", G_TYPE_STRING, data.payload,
                "symbol", G_TYPE_STRING, data.payload,
                //"quality", G_TYPE_INT, quality,
                 NULL);


            m = gst_message_new_element (GST_OBJECT (filter), s);
            gst_element_post_message (GST_ELEMENT (filter), m);

        }
      }
    }

     
      
    gst_memory_unmap (mem, &info);

  }
  
  /* just push out the incoming buffer without touching it */
  return gst_pad_push (filter->srcpad, buf);
}




/*
 * called when element change state
 */
static GstStateChangeReturn 
gst_qrcode_dec_change_state (GstElement *element, GstStateChange transition)
{
  GstStateChangeReturn ret = GST_STATE_CHANGE_SUCCESS;
  GstQRCodeDec *filter = GST_QRCODEDEC (element);


  //upward state change**
  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
        GST_LOG_OBJECT (filter, "null to ready");
        if (filter->qr != NULL) quirc_destroy(filter->qr);
        filter->qr = quirc_new();
        if (!filter->qr)
          GST_ERROR_OBJECT (filter, "couldn't create quirc element");
      break;
    default:
      break;
  }

  ret = GST_ELEMENT_CLASS (parent_class)->change_state (element, transition);
  if (ret == GST_STATE_CHANGE_FAILURE)
    return ret;

  //Downwards state change;**
  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
        GST_LOG_OBJECT (filter, "ready to null");
        if (filter->qr != NULL) {  
          quirc_destroy(filter->qr);
          filter->qr = NULL;
        }
      break;
    default:
      break;
  }

  return ret;
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
qrcodedec_init (GstPlugin * qrcodedec)
{
  /* debug category for fltering log messages
   *
   */
  GST_DEBUG_CATEGORY_INIT (gst_qrcode_dec_debug, "qrcodedec",
      0, "QRCode decoder");

  return gst_element_register (qrcodedec, "qrcodedec", GST_RANK_NONE,
      GST_TYPE_QRCODEDEC);
}

/* PACKAGE: this is usually set by autotools depending on some _INIT macro
 * in configure.ac and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use autotools to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "gst-qrcode"
#endif

/* gstreamer looks for this structure to register qrcodedecs
 *
 */
GST_PLUGIN_DEFINE (
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    qrcodedec,
    "QRCode decoder",
    qrcodedec_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/"
)

