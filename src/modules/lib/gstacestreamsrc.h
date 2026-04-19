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
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef _GST_ACESTREAM_SRC_H_
#define _GST_ACESTREAM_SRC_H_

#include <gst/base/gstpushsrc.h>

G_BEGIN_DECLS

#define GST_TYPE_ACESTREAM_SRC   (gst_acestream_src_get_type())
#define GST_ACESTREAM_SRC(obj)   (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ACESTREAM_SRC,GstAcestreamSrc))
#define GST_ACESTREAM_SRC_CLASS(klass)   (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ACESTREAM_SRC,GstAcestreamSrcClass))
#define GST_IS_ACESTREAM_SRC(obj)   (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ACESTREAM_SRC))
#define GST_IS_ACESTREAM_SRC_CLASS(obj)   (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ACESTREAM_SRC))

typedef struct _GstAcestreamSrc GstAcestreamSrc;
typedef struct _GstAcestreamSrcClass GstAcestreamSrcClass;

struct _GstAcestreamSrc
{
  GstPushSrc push_acestreamsrc;

  gpointer   queue;
};

struct _GstAcestreamSrcClass
{
  GstPushSrcClass push_acestreamsrc_class;
};

GType gst_acestream_src_get_type (void);

G_END_DECLS

#endif
