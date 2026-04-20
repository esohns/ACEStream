/* GStreamer
 * Copyright (C) 2026 FIXME <fixme@example.com>
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
 * Free Software Foundation, Inc., 51 Franklin Street, Suite 500,
 * Boston, MA 02110-1335, USA.
 */
/**
 * SECTION:element-gstacestreamsrc
 *
 * The acestreamsrc element does FIXME stuff.
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * gst-launch-1.0 -v fakesrc ! acestreamsrc ! FIXME ! fakesink
 * ]|
 * FIXME Describe what the pipeline does.
 * </refsect2>
 */

#include "gstacestreamsrc.h"

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

#include "ace/Message_Block.h"
#include "ace/Message_Queue.h"
#include "ace/OS.h"

#if defined (HAVE_CONFIG_H)
#include "ACEStream_config.h"
#endif // HAVE_CONFIG_H

GST_DEBUG_CATEGORY_STATIC (gst_acestream_src_debug_category);
#define GST_CAT_DEFAULT gst_acestream_src_debug_category

/* prototypes */


static void gst_acestream_src_set_property (GObject * object,
    guint property_id, const GValue * value, GParamSpec * pspec);
static void gst_acestream_src_get_property (GObject * object,
    guint property_id, GValue * value, GParamSpec * pspec);
static void gst_acestream_src_dispose (GObject * object);
//static void gst_acestream_src_finalize (GObject * object);
//
//static GstCaps *gst_acestream_src_get_caps (GstPushSrc * src, GstCaps * filter);
//static gboolean gst_acestream_src_negotiate (GstPushSrc * src);
//static GstCaps *gst_acestream_src_fixate (GstPushSrc * src, GstCaps * caps);
//static gboolean gst_acestream_src_set_caps (GstPushSrc * src, GstCaps * caps);
//static gboolean gst_acestream_src_decide_allocation (GstPushSrc * src,
//    GstQuery * query);
static gboolean gst_acestream_src_start (GstBaseSrc * src);
//static gboolean gst_acestream_src_stop (GstPushSrc * src);
//static void gst_acestream_src_get_times (GstPushSrc * src, GstBuffer * buffer,
//    GstClockTime * start, GstClockTime * end);
//static gboolean gst_acestream_src_get_size (GstPushSrc * src, guint64 * size);
//static gboolean gst_acestream_src_is_seekable (GstPushSrc * src);
//static gboolean gst_acestream_src_prepare_seek_segment (GstPushSrc * src,
//    GstEvent * seek, GstSegment * segment);
//static gboolean gst_acestream_src_do_seek (GstPushSrc * src, GstSegment * segment);
//static gboolean gst_acestream_src_unlock (GstPushSrc * src);
//static gboolean gst_acestream_src_unlock_stop (GstPushSrc * src);
//static gboolean gst_acestream_src_query (GstPushSrc * src, GstQuery * query);
//static gboolean gst_acestream_src_event (GstPushSrc * src, GstEvent * event);
//static GstFlowReturn gst_acestream_src_create (GstPushSrc * src, guint64 offset,
//    guint size, GstBuffer ** buf);
//static GstFlowReturn gst_acestream_src_alloc (GstPushSrc * src, guint64 offset,
//    guint size, GstBuffer ** buf);
static GstFlowReturn gst_acestream_src_fill (GstPushSrc * src, GstBuffer * buf);

enum
{
  PROP_0,
  PROP_QUEUE
};

/* pad templates */

static GstStaticPadTemplate gst_acestream_src_src_template =
GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS ("application/unknown")
    );


/* class initialization */

G_DEFINE_TYPE_WITH_CODE (GstAcestreamSrc, gst_acestream_src, GST_TYPE_PUSH_SRC,
  GST_DEBUG_CATEGORY_INIT (gst_acestream_src_debug_category, "acestreamsrc", 0,
  "debug category for acestreamsrc element"));

static void
gst_acestream_src_class_init (GstAcestreamSrcClass * klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);
  GstBaseSrcClass* base_src_class = GST_BASE_SRC_CLASS (klass);
  GstPushSrcClass *push_src_class = GST_PUSH_SRC_CLASS (klass);

  /* Setting up pads and setting metadata should be moved to
     base_class_init if you intend to subclass this class. */
  gst_element_class_add_static_pad_template (GST_ELEMENT_CLASS(klass),
      &gst_acestream_src_src_template);

  gst_element_class_set_static_metadata (GST_ELEMENT_CLASS(klass),
      "FIXME Long name", "Generic", "FIXME Description",
      "FIXME <fixme@example.com>");

  g_object_class_install_property (gobject_class, PROP_QUEUE,
      g_param_spec_pointer ("queue", "Queue",
          "Handle of the inbound queue",
          (GParamFlags)(G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS)));

  gobject_class->set_property = gst_acestream_src_set_property;
  gobject_class->get_property = gst_acestream_src_get_property;
  gobject_class->dispose = gst_acestream_src_dispose;
  //gobject_class->finalize = gst_acestream_src_finalize;
  //push_src_class->get_caps = GST_DEBUG_FUNCPTR (gst_acestream_src_get_caps);
  //push_src_class->negotiate = GST_DEBUG_FUNCPTR (gst_acestream_src_negotiate);
  //push_src_class->fixate = GST_DEBUG_FUNCPTR (gst_acestream_src_fixate);
  //push_src_class->set_caps = GST_DEBUG_FUNCPTR (gst_acestream_src_set_caps);
  //push_src_class->decide_allocation = GST_DEBUG_FUNCPTR (gst_acestream_src_decide_allocation);
  base_src_class->start = GST_DEBUG_FUNCPTR (gst_acestream_src_start);
  //push_src_class->stop = GST_DEBUG_FUNCPTR (gst_acestream_src_stop);
  //push_src_class->get_times = GST_DEBUG_FUNCPTR (gst_acestream_src_get_times);
  //push_src_class->get_size = GST_DEBUG_FUNCPTR (gst_acestream_src_get_size);
  //push_src_class->is_seekable = GST_DEBUG_FUNCPTR (gst_acestream_src_is_seekable);
  //push_src_class->prepare_seek_segment = GST_DEBUG_FUNCPTR (gst_acestream_src_prepare_seek_segment);
  //push_src_class->do_seek = GST_DEBUG_FUNCPTR (gst_acestream_src_do_seek);
  //push_src_class->unlock = GST_DEBUG_FUNCPTR (gst_acestream_src_unlock);
  //push_src_class->unlock_stop = GST_DEBUG_FUNCPTR (gst_acestream_src_unlock_stop);
  //push_src_class->query = GST_DEBUG_FUNCPTR (gst_acestream_src_query);
  //push_src_class->event = GST_DEBUG_FUNCPTR (gst_acestream_src_event);
  //push_src_class->create = GST_DEBUG_FUNCPTR (gst_acestream_src_create);
  //push_src_class->alloc = GST_DEBUG_FUNCPTR (gst_acestream_src_alloc);
  push_src_class->fill = GST_DEBUG_FUNCPTR (gst_acestream_src_fill);
}

static void
gst_acestream_src_init (GstAcestreamSrc *acestreamsrc)
{
  acestreamsrc->buffer = NULL;
  acestreamsrc->queue = NULL;
}

void
gst_acestream_src_set_property (GObject * object, guint property_id,
    const GValue * value, GParamSpec * pspec)
{
  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (object);

  GST_DEBUG_OBJECT (acestreamsrc, "set_property");

  switch (property_id)
  {
    case PROP_QUEUE:
      acestreamsrc->queue =
        static_cast<ACE_Message_Queue_Base*> (g_value_get_pointer (value));
      g_print ("Setting Queue to %p\n", (void*)acestreamsrc->queue);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  } // end SWITCH
}

void
gst_acestream_src_get_property (GObject * object, guint property_id,
    GValue * value, GParamSpec * pspec)
{
  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (object);

  GST_DEBUG_OBJECT (acestreamsrc, "get_property");

  switch (property_id)
  {
    case PROP_QUEUE:
      g_value_set_pointer (value, acestreamsrc->queue);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
  } // end SWITCH
}

void
gst_acestream_src_dispose (GObject * object)
{
  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (object);

  GST_DEBUG_OBJECT (acestreamsrc, "dispose");

  /* clean up as possible.  may be called multiple times */

  G_OBJECT_CLASS (gst_acestream_src_parent_class)->dispose (object);
}

//void
//gst_acestream_src_finalize (GObject * object)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (object);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "finalize");
//
//  /* clean up object here */
//
//  G_OBJECT_CLASS (gst_acestream_src_parent_class)->finalize (object);
//}
//
///* get caps from subclass */
//static GstCaps *
//gst_acestream_src_get_caps (GstPushSrc * src, GstCaps * filter)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "get_caps");
//
//  return NULL;
//}
//
///* decide on caps */
//static gboolean
//gst_acestream_src_negotiate (GstPushSrc * src)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "negotiate");
//
//  return TRUE;
//}
//
///* called if, in negotiation, caps need fixating */
//static GstCaps *
//gst_acestream_src_fixate (GstPushSrc * src, GstCaps * caps)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "fixate");
//
//  return NULL;
//}
//
///* notify the subclass of new caps */
//static gboolean
//gst_acestream_src_set_caps (GstPushSrc * src, GstCaps * caps)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "set_caps");
//
//  return TRUE;
//}
//
///* setup allocation query */
//static gboolean
//gst_acestream_src_decide_allocation (GstPushSrc * src, GstQuery * query)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "decide_allocation");
//
//  return TRUE;
//}

/* start and stop processing, ideal for opening/closing the resource */
static gboolean
gst_acestream_src_start (GstBaseSrc * src)
{
  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);

  GST_DEBUG_OBJECT (acestreamsrc, "start");

  return TRUE;
}

//static gboolean
//gst_acestream_src_stop (GstPushSrc * src)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "stop");
//
//  return TRUE;
//}
//
///* given a buffer, return start and stop time when it should be pushed
// * out. The base class will sync on the clock using these times. */
//static void
//gst_acestream_src_get_times (GstPushSrc * src, GstBuffer * buffer,
//    GstClockTime * start, GstClockTime * end)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "get_times");
//
//}
//
///* get the total size of the resource in bytes */
//static gboolean
//gst_acestream_src_get_size (GstPushSrc * src, guint64 * size)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "get_size");
//
//  return TRUE;
//}
//
///* check if the resource is seekable */
//static gboolean
//gst_acestream_src_is_seekable (GstPushSrc * src)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "is_seekable");
//
//  return TRUE;
//}
//
///* Prepare the segment on which to perform do_seek(), converting to the
// * current basesrc format. */
//static gboolean
//gst_acestream_src_prepare_seek_segment (GstPushSrc * src, GstEvent * seek,
//    GstSegment * segment)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "prepare_seek_segment");
//
//  return TRUE;
//}
//
///* notify subclasses of a seek */
//static gboolean
//gst_acestream_src_do_seek (GstPushSrc * src, GstSegment * segment)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "do_seek");
//
//  return TRUE;
//}
//
///* unlock any pending access to the resource. subclasses should unlock
// * any function ASAP. */
//static gboolean
//gst_acestream_src_unlock (GstPushSrc * src)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "unlock");
//
//  return TRUE;
//}
//
///* Clear any pending unlock request, as we succeeded in unlocking */
//static gboolean
//gst_acestream_src_unlock_stop (GstPushSrc * src)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "unlock_stop");
//
//  return TRUE;
//}
//
///* notify subclasses of a query */
//static gboolean
//gst_acestream_src_query (GstPushSrc * src, GstQuery * query)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "query");
//
//  return TRUE;
//}
//
///* notify subclasses of an event */
//static gboolean
//gst_acestream_src_event (GstPushSrc * src, GstEvent * event)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "event");
//
//  return TRUE;
//}
//
///* ask the subclass to create a buffer with offset and size, the default
// * implementation will call alloc and fill. */
//static GstFlowReturn
//gst_acestream_src_create (GstPushSrc * src, guint64 offset, guint size,
//    GstBuffer ** buf)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "create");
//
//  return GST_FLOW_OK;
//}
//
///* ask the subclass to allocate an output buffer. The default implementation
// * will use the negotiated allocator. */
//static GstFlowReturn
//gst_acestream_src_alloc (GstPushSrc * src, guint64 offset, guint size,
//    GstBuffer ** buf)
//{
//  GstAcestreamSrc *acestreamsrc = GST_ACESTREAM_SRC (src);
//
//  GST_DEBUG_OBJECT (acestreamsrc, "alloc");
//
//  return GST_FLOW_OK;
//}

/* ask the subclass to fill the buffer with data from offset and size */
static GstFlowReturn
gst_acestream_src_fill (GstPushSrc * src, GstBuffer * buf)
{
  GstAcestreamSrc* acestreamsrc = GST_ACESTREAM_SRC (src);

  GST_DEBUG_OBJECT (acestreamsrc, "fill");

  // sanity check(s)
  ACE_ASSERT (acestreamsrc->queue);

  gsize available_buffer_size_i;
  GstMapInfo map_info_s;
  size_t bytes_to_write_i;
  size_t offset_i = 0;
  ACE_Message_Block* message_block_p;
  int result;

  gst_buffer_map (buf, &map_info_s, GST_MAP_WRITE);
  available_buffer_size_i = gst_buffer_get_size (buf);

  if (acestreamsrc->buffer)
    goto continue_;

deqeue_next_buffer:
  ACE_ASSERT (!acestreamsrc->buffer);
  result = acestreamsrc->queue->dequeue_head (acestreamsrc->buffer, NULL);
  if (result == -1)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to ACE_Message_Queue_Base::dequeue_head(): \"%m\", aborting\n")));
    gst_buffer_unmap (buf, &map_info_s);
    return GST_FLOW_EOS; // --> stop
  } // end IF
continue_:
  ACE_ASSERT (acestreamsrc->buffer);
  if (acestreamsrc->buffer->msg_type () == ACE_Message_Block::MB_STOP)
  {
    acestreamsrc->buffer->release (); acestreamsrc->buffer = NULL;
    gst_buffer_unmap (buf, &map_info_s);
    return GST_FLOW_EOS; // --> stop
  } // end IF

  bytes_to_write_i =
    std::min (acestreamsrc->buffer->length (), static_cast<size_t> (available_buffer_size_i));
  ACE_OS::memcpy (map_info_s.data + offset_i, acestreamsrc->buffer->rd_ptr (), bytes_to_write_i);
  offset_i += bytes_to_write_i;
  available_buffer_size_i -= static_cast<gsize> (bytes_to_write_i);
  acestreamsrc->buffer->rd_ptr (bytes_to_write_i);
  if (!acestreamsrc->buffer->length ())
  {
    if (acestreamsrc->buffer->cont ())
    {
      message_block_p = acestreamsrc->buffer->cont ();
      acestreamsrc->buffer->cont (NULL);
      acestreamsrc->buffer->release (); acestreamsrc->buffer = NULL;
      acestreamsrc->buffer = message_block_p;
      if (available_buffer_size_i)
        goto continue_;
    } // end IF
    else
    {
      acestreamsrc->buffer->release (); acestreamsrc->buffer = NULL;
    } // end ELSE
  } // end IF
  if (available_buffer_size_i)
    goto deqeue_next_buffer;

  gst_buffer_unmap (buf, &map_info_s);

  return GST_FLOW_OK;
}

static gboolean
plugin_init (GstPlugin * plugin)
{

  /* FIXME Remember to set the rank if it's an element that is meant
     to be autoplugged by decodebin. */
  return gst_element_register (plugin, "acestreamsrc", GST_RANK_NONE,
      GST_TYPE_ACESTREAM_SRC);
}

/* FIXME: these are normally defined by the GStreamer build system.
   If you are creating an element to be included in gst-plugins-*,
   remove these, as they're always defined.  Otherwise, edit as
   appropriate for your external plugin package. */
#ifndef VERSION
#define VERSION "0.0.1"
#endif
#ifndef PACKAGE
#define PACKAGE "acestreamsrc"
#endif
#ifndef PACKAGE_NAME
#define PACKAGE_NAME "acestreamsrc"
#endif
#ifndef GST_PACKAGE_ORIGIN
#define GST_PACKAGE_ORIGIN "https://github.com/esohns/ACEStream"
#endif

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    acestreamsrc,
    "ACEStream to GStreamer adapter",
    plugin_init, VERSION, "LGPL", PACKAGE_NAME, GST_PACKAGE_ORIGIN)

