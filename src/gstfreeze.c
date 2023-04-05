/*
 * GStreamer
 * Copyright (C) 2005 Thomas Vander Stichele <thomas@apestaart.org>
 * Copyright (C) 2005 Ronald S. Bultje <rbultje@ronald.bitfreak.net>
 * Copyright (C) 2022 Nicolas <<user@hostname.org>>
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
 * SECTION:element-freeze
 *
 * FIXME:Describe freeze here.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch -v -m fakesrc ! freeze ! fakesink silent=TRUE
 * ]|
 * </refsect2>
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gst/gst.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include "gstfreeze.h"

GST_DEBUG_CATEGORY_STATIC(gst_freeze_debug);
#define GST_CAT_DEFAULT gst_freeze_debug

/* Filter signals and args */
enum
{
    /* FILL ME */
    LAST_SIGNAL
};

enum
{
    PROP_0,
    PROP_FREEZE,
    PROP_LISTEN
};

gboolean isFrozen = FALSE;

/* the capabilities of the inputs and outputs.
 *
 * describe the real formats here.
 */
static GstStaticPadTemplate sink_factory = GST_STATIC_PAD_TEMPLATE("sink",
                                                                   GST_PAD_SINK,
                                                                   GST_PAD_ALWAYS,
                                                                   GST_STATIC_CAPS("ANY"));

static GstStaticPadTemplate src_factory = GST_STATIC_PAD_TEMPLATE("src",
                                                                  GST_PAD_SRC,
                                                                  GST_PAD_ALWAYS,
                                                                  GST_STATIC_CAPS("ANY"));

#define gst_freeze_parent_class parent_class
G_DEFINE_TYPE(Gstfreeze, gst_freeze, GST_TYPE_ELEMENT)

static void gst_freeze_set_property(GObject *object,
                                    guint prop_id, const GValue *value, GParamSpec *pspec);
static void gst_freeze_get_property(GObject *object,
                                    guint prop_id, GValue *value, GParamSpec *pspec);

static GstFlowReturn gst_freeze_chain(GstPad *pad,
                                      GstObject *parent, GstBuffer *buf);

static void gst_freeze_finalize(GObject *object);

void *freezeHandler(void *freeze)
{
    Gstfreeze *f = (Gstfreeze *)freeze;

    if (f != NULL)
    {
        sleep(1);   // Make sure the listen property has been set

        while (f->listen)
        {
            if (getc(stdin) == 'f')
            {
                f->freeze = !f->freeze;
            }
        }
    }
    else
    {
        g_print("Error: autofocus is null\n");
    }

    pthread_exit(NULL);
}

/* GObject vmethod implementations */

/* initialize the freeze's class */
static void
gst_freeze_class_init(GstfreezeClass *klass)
{
    GObjectClass *gobject_class;
    GstElementClass *gstelement_class;

    gobject_class = (GObjectClass *)klass;
    gstelement_class = (GstElementClass *)klass;

    gobject_class->set_property = gst_freeze_set_property;
    gobject_class->get_property = gst_freeze_get_property;
    gobject_class->finalize = gst_freeze_finalize;

    g_object_class_install_property(gobject_class, PROP_FREEZE,
                                    g_param_spec_boolean("freeze", "Freeze",
                                                         "Freeze the stream without blocking the pipeline",
                                                         FALSE, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, PROP_LISTEN,
                                    g_param_spec_boolean("listen", "Listen",
                                                         "Listen for user inputs in the terminal",
                                                         TRUE, G_PARAM_READWRITE));

    gst_element_class_set_details_simple(gstelement_class,
                                         "freeze",
                                         "FIXME:Generic",
                                         "Freeze and unfreeze a video stream",
                                         "Esisar-PI2022 <<user@hostname.org>>");

    gst_element_class_add_pad_template(gstelement_class,
                                       gst_static_pad_template_get(&src_factory));
    gst_element_class_add_pad_template(gstelement_class,
                                       gst_static_pad_template_get(&sink_factory));
}

/* initialize the new element
 * instantiate pads and add them to element
 * set pad calback functions
 * initialize instance structure
 */
static void
gst_freeze_init(Gstfreeze *freeze)
{
    freeze->sinkpad = gst_pad_new_from_static_template(&sink_factory, "sink");
    gst_pad_set_chain_function(freeze->sinkpad,
                               GST_DEBUG_FUNCPTR(gst_freeze_chain));
    GST_PAD_SET_PROXY_CAPS(freeze->sinkpad);
    gst_element_add_pad(GST_ELEMENT(freeze), freeze->sinkpad);

    freeze->srcpad = gst_pad_new_from_static_template(&src_factory, "src");
    GST_PAD_SET_PROXY_CAPS(freeze->srcpad);
    gst_element_add_pad(GST_ELEMENT(freeze), freeze->srcpad);

    freeze->frame = NULL;
    freeze->freeze = FALSE;
    freeze->listen = TRUE;

    pthread_t thread;

    int rc;
    if ((rc = pthread_create(&thread, NULL, freezeHandler, (void *)freeze)))
    {
        g_printerr("Error: Unable to create a thread, %d", rc);
        exit(EXIT_FAILURE);
    }
}

static void
gst_freeze_set_property(GObject *object, guint prop_id,
                        const GValue *value, GParamSpec *pspec)
{
    Gstfreeze *freeze = GST_FREEZE(object);

    switch (prop_id)
    {
    case PROP_FREEZE:
        freeze->freeze = g_value_get_boolean(value);
        break;
    case PROP_LISTEN:
        freeze->listen = g_value_get_boolean(value);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

static void
gst_freeze_get_property(GObject *object, guint prop_id,
                        GValue *value, GParamSpec *pspec)
{
    Gstfreeze *freeze = GST_FREEZE(object);

    switch (prop_id)
    {
    case PROP_FREEZE:
        g_value_set_boolean(value, freeze->freeze);
        break;
    case PROP_LISTEN:
        g_value_set_boolean(value, freeze->listen);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
}

/* chain function
 * this function does the actual processing
 */
static GstFlowReturn
gst_freeze_chain(GstPad *pad, GstObject *parent, GstBuffer *buf)
{
    Gstfreeze *freeze = GST_FREEZE(parent);

    if (freeze->freeze)
    {
        GstMapInfo map;
        gst_buffer_map(buf, &map, GST_MAP_READWRITE);

        if (freeze->freeze != isFrozen)
        {
            freeze->frame = (guint8 *)realloc(freeze->frame, map.size);

            if (freeze->frame == NULL)
            {
                g_printerr("Error: Unable to allocate memory\n");
            }
            else
            {
                freeze->frame = (guint8 *)memcpy(freeze->frame, map.data, map.size);
                freeze->frameSize = map.size;
            }

            isFrozen = freeze->freeze;
        }
        else if (freeze->frame != NULL)
        {
            map.data = memcpy(map.data, freeze->frame, freeze->frameSize);
        }

        gst_buffer_unmap(buf, &map);
    }
    else
    {
        if (freeze->freeze != isFrozen)
        {
            free(freeze->frame);
            freeze->frame = NULL;

            isFrozen = freeze->freeze;
        }
    }

    /* just push out the incoming buffer without touching it */
    return gst_pad_push(freeze->srcpad, buf);
}

/* entry point to initialize the plug-in
 * initialize the plug-in itself
 * register the element factories and other features
 */
static gboolean
freeze_init(GstPlugin *freeze)
{
    /* debug category for freezeing log messages
     *
     * exchange the string 'Template freeze' with your description
     */
    GST_DEBUG_CATEGORY_INIT(gst_freeze_debug, "freeze",
                            0, "Template freeze");

    return gst_element_register(freeze, "freeze", GST_RANK_NONE,
                                GST_TYPE_FREEZE);
}

static void gst_freeze_finalize(GObject *object)
{
    Gstfreeze *freeze = GST_FREEZE(object);

    free(freeze->frame);
}
/* PACKAGE: this is usually set by meson depending on some _INIT macro
 * in meson.build and then written into and defined in config.h, but we can
 * just set it ourselves here in case someone doesn't use meson to
 * compile this code. GST_PLUGIN_DEFINE needs PACKAGE to be defined.
 */
#ifndef PACKAGE
#define PACKAGE "myfirstfreeze"
#endif

/* gstreamer looks for this structure to register freezes
 *
 * exchange the string 'Template freeze' with your freeze description
 */

#ifdef HAVE_CONFIG_H

GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    freeze,
    "Template freeze",
    freeze_init,
    VERSION,
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/")
#else
GST_PLUGIN_DEFINE(
    GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    freeze,
    "Template freeze",
    freeze_init,
    "Unknown",
    "LGPL",
    "GStreamer",
    "http://gstreamer.net/")
#endif
