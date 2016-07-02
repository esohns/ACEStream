/***************************************************************************
 *   Copyright (C) 2009 by Erik Sohns   *
 *   erik.sohns@web.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "ace/Log_Msg.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
#include "linux/videodev2.h"
#ifdef __cplusplus
extern "C"
{
#include "libavcodec/avcodec.h"
}
#endif
#endif

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_vis_defines.h"

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::Stream_Module_Vis_GTK_Pixbuf_T ()
 : inherited ()
 , configuration_ (NULL)
 , sessionData_ (NULL)
 , pixelBuffer_ (NULL)
 , isFirst_ (true)
 , isInitialized_ (false)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::Stream_Module_Vis_GTK_Pixbuf_T"));

}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::~Stream_Module_Vis_GTK_Pixbuf_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::~Stream_Module_Vis_GTK_Pixbuf_T"));

  if (pixelBuffer_)
    g_object_unref (pixelBuffer_);

  if (sessionData_)
    sessionData_->decrease ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
int
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::clamp (int value_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::clamp"));

  return ((value_in > 255) ? 255
                           : ((value_in < 0) ? 0
                                             : value_in));
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::handleDataMessage (MessageType*& message_inout,
                                                                             bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleDataMessage"));

//  ACE_Message_Block* message_block_p = message_inout;

  // sanity check(s)
  ACE_ASSERT (configuration_);
  if (!configuration_->window)
    return; // done
//  if (configuration_->hasHeader &&
//      isFirst_)
//  {
//    isFirst_ = false;
//    return; // done
//  } // end IF
//  // *NOTE*: some formats (e.g. AVI frames) have (i.e. RIFF) 'chunk' header
//  //         prefixes that will have been prepended upstream (first
//  //         continuation) and need to be skipped over
//  if (configuration_->hasChunkHeaders)
//  {
//    ACE_ASSERT (message_block_p->cont ());
//    message_block_p = message_block_p->cont ();
//  } // end IF
  ACE_ASSERT (sessionData_);
  ACE_ASSERT (pixelBuffer_);
  const SessionDataType& session_data_r = sessionData_->get ();
  unsigned int total_length = message_inout->total_length ();
  ACE_ASSERT (total_length ==
              session_data_r.format.fmt.pix.sizeimage);

  // *NOTE*: 'crunching' the message data simplifies the data transformation
  //         algorithms, at the cost of several memory copies. This is a
  //         tradeoff that may warrant further optimization efforts
  if (!message_inout->crunch ())
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_MessageBase_T::crunch(): \"%m\", returning\n")));
    return;
  } // end IF
  ACE_ASSERT (message_inout->length () ==
              session_data_r.format.fmt.pix.sizeimage);

  ACE_Message_Block* message_block_p = message_inout;
  unsigned int offset = 0;//, length = message_block_p->length ();
  unsigned char* data_p =
      reinterpret_cast<unsigned char*> (message_block_p->rd_ptr ());
  ACE_ASSERT (data_p);
  guchar* data_2 = gdk_pixbuf_get_pixels (pixelBuffer_);
  ACE_ASSERT (data_2);

//  int bits_per_sample = 8;
  int row_stride = session_data_r.format.fmt.pix.bytesperline;
  unsigned char* pixel_p = data_p;
  unsigned int* pixel_2 = (unsigned int*)data_2;
  switch (session_data_r.format.fmt.pix.pixelformat)
  {
    // RGB formats
    case V4L2_PIX_FMT_BGR24:
    {
      // convert BGR (24bit) to RGB (32bit)
      row_stride = gdk_pixbuf_get_rowstride (pixelBuffer_);
      for (unsigned int y = 0;
           y < session_data_r.format.fmt.pix.height;
           ++y)
        for (unsigned int x = 0;
             x < session_data_r.format.fmt.pix.width;
             ++x)
        {
          pixel_p = data_p + offset;
          pixel_2 = (unsigned int*)(data_2 + ((y * row_stride) + (x * 4)));

          *pixel_2 = ((*(pixel_p + 2) << 16) |
                      (*(pixel_p + 1) << 8)  |
                      *pixel_p);

          offset += 3;
        } // end FOR

      break;
    }
    case V4L2_PIX_FMT_RGB24:
    {
      // convert RGB (24bit) to RGB (32bit)
      row_stride = gdk_pixbuf_get_rowstride (pixelBuffer_);
      for (unsigned int y = 0;
           y < session_data_r.format.fmt.pix.height;
           ++y)
        for (unsigned int x = 0;
             x < session_data_r.format.fmt.pix.width;
             ++x)
        {
          pixel_p = data_p + offset;
          pixel_2 = (unsigned int*)(data_2 + ((y * row_stride) + (x * 4)));

          *pixel_2 = (*pixel_p        << 16) |
                      (*(pixel_p + 1) << 8)  |
                      (*(pixel_p + 2));

          offset += 3;
        } // end FOR

      break;
    }
    // luminance / chrominance formats
    case V4L2_PIX_FMT_YVU420:
    {
      // decode YVU to RGB (planar format)
      int row_stride_2 = gdk_pixbuf_get_rowstride (pixelBuffer_);
      unsigned int number_of_pixels =
          (session_data_r.format.fmt.pix.height *
           session_data_r.format.fmt.pix.width);
      unsigned char* chrominance_b_base_p = data_p           +
                                            number_of_pixels +
                                            (number_of_pixels / 4);
      unsigned char* chrominance_r_base_p = data_p + number_of_pixels;
      unsigned char* chrominance_b_p = chrominance_b_base_p; // Cb
      unsigned char* chrominance_r_p = chrominance_r_base_p; // Cr
      unsigned char* R1_p = data_2;
      unsigned char* G1_p = R1_p + 1;
      unsigned char* B1_p = R1_p + 2;
      unsigned char* R2_p = R1_p + row_stride_2;
      unsigned char* G2_p = R2_p + 1;
      unsigned char* B2_p = R2_p + 2;
      unsigned char* Y1_p = data_p;
      unsigned char* Y2_p = Y1_p + row_stride;
      unsigned char* end_p =
          R1_p + (row_stride_2 * session_data_r.format.fmt.pix.height);
      unsigned char* line_end_p = R2_p;
      int y1, y2, cb, cr = 0;
      while (R1_p != end_p)
      {
        line_end_p = R2_p;
        while (R1_p != line_end_p)
        {
          // first pixel
          y1 = *Y1_p - 16;
          y2 = *Y2_p - 16;
          cb = *chrominance_b_p - 128;
          cr = *chrominance_r_p - 128;

          *R1_p = clamp (y1 + ((454 * cr) >> 8));
          *G1_p = clamp (y1 - (((88 * cr) + (183 * cb)) >> 8));
          *B1_p = clamp (y1 + ((359 * cb) >> 8));

          *R2_p = clamp (y2 + ((454 * cr) >> 8));
          *G2_p = clamp (y2 - (((88 * cr) + (183 * cb)) >> 8));
          *B2_p = clamp (y2 + ((359 * cb) >> 8));

          R1_p += 4;
          G1_p += 4;
          B1_p += 4;

          R2_p += 4;
          G2_p += 4;
          B2_p += 4;

          ++Y1_p;
          ++Y2_p;

          // second pixel
          y1 = *Y1_p - 16;
          y2 = *Y2_p - 16;
          cb = *chrominance_b_p - 128;
          cr = *chrominance_r_p - 128;

          *R1_p = clamp (y1 + ((454 * cr) >> 8));
          *G1_p = clamp (y1 - (((88 * cr) + (183 * cb)) >> 8));
          *B1_p = clamp (y1 + ((359 * cb) >> 8));

          *R2_p = clamp (y2 + ((454 * cr) >> 8));
          *G2_p = clamp (y2 - (((88 * cr) + (183 * cb)) >> 8));
          *B2_p = clamp (y2 + ((359 * cb) >> 8));

          R1_p += 4;
          G1_p += 4;
          B1_p += 4;

          R2_p += 4;
          G2_p += 4;
          B2_p += 4;

          ++Y1_p;
          ++Y2_p;

          ++chrominance_b_p;
          ++chrominance_r_p;
        } // end WHILE

        R1_p += row_stride_2;
        G1_p += row_stride_2;
        B1_p += row_stride_2;
        R2_p += row_stride_2;
        G2_p += row_stride_2;
        B2_p += row_stride_2;
        Y1_p += row_stride;
        Y2_p += row_stride;
      } // end WHILE

      break;
    }
    case V4L2_PIX_FMT_YUV420:
    {
      // decode YUV to RGB (planar format)
      int row_stride_2 = gdk_pixbuf_get_rowstride (pixelBuffer_);
      unsigned int number_of_pixels =
          (session_data_r.format.fmt.pix.height *
           session_data_r.format.fmt.pix.width);
      unsigned char* chrominance_b_base_p = data_p + number_of_pixels;
      unsigned char* chrominance_r_base_p = data_p           +
                                            number_of_pixels +
                                            (number_of_pixels / 4);
      unsigned char* chrominance_b_p = chrominance_b_base_p; // Cb
      unsigned char* chrominance_r_p = chrominance_r_base_p; // Cr
      unsigned char* R1_p = data_2;
      unsigned char* G1_p = R1_p + 1;
      unsigned char* B1_p = R1_p + 2;
      unsigned char* R2_p = R1_p + row_stride_2;
      unsigned char* G2_p = R2_p + 1;
      unsigned char* B2_p = R2_p + 2;
      unsigned char* Y1_p = data_p;
      unsigned char* Y2_p = Y1_p + row_stride;
      unsigned char* end_p =
          R1_p + (row_stride_2 * session_data_r.format.fmt.pix.height);
      unsigned char* line_end_p = R2_p;
      int y1, y2, cb, cr = 0;
      while (R1_p != end_p)
      {
        line_end_p = R2_p;
        while (R1_p != line_end_p)
        {
          // first pixel
          y1 = *Y1_p - 16;
          y2 = *Y2_p - 16;
          cb = *chrominance_b_p - 128;
          cr = *chrominance_r_p - 128;

          *R1_p = clamp (y1 + ((454 * cr) >> 8));
          *G1_p = clamp (y1 - (((88 * cr) + (183 * cb)) >> 8));
          *B1_p = clamp (y1 + ((359 * cb) >> 8));

          *R2_p = clamp (y2 + ((454 * cr) >> 8));
          *G2_p = clamp (y2 - (((88 * cr) + (183 * cb)) >> 8));
          *B2_p = clamp (y2 + ((359 * cb) >> 8));

          R1_p += 4;
          G1_p += 4;
          B1_p += 4;

          R2_p += 4;
          G2_p += 4;
          B2_p += 4;

          ++Y1_p;
          ++Y2_p;

          // second pixel
          y1 = *Y1_p - 16;
          y2 = *Y2_p - 16;
          cb = *chrominance_b_p - 128;
          cr = *chrominance_r_p - 128;

          *R1_p = clamp (y1 + ((454 * cr) >> 8));
          *G1_p = clamp (y1 - (((88 * cr) + (183 * cb)) >> 8));
          *B1_p = clamp (y1 + ((359 * cb) >> 8));

          *R2_p = clamp (y2 + ((454 * cr) >> 8));
          *G2_p = clamp (y2 - (((88 * cr) + (183 * cb)) >> 8));
          *B2_p = clamp (y2 + ((359 * cb) >> 8));

          R1_p += 4;
          G1_p += 4;
          B1_p += 4;

          R2_p += 4;
          G2_p += 4;
          B2_p += 4;

          ++Y1_p;
          ++Y2_p;

          ++chrominance_b_p;
          ++chrominance_r_p;
        } // end WHILE

        R1_p += row_stride_2;
        G1_p += row_stride_2;
        B1_p += row_stride_2;
        R2_p += row_stride_2;
        G2_p += row_stride_2;
        B2_p += row_stride_2;
        Y1_p += row_stride;
        Y2_p += row_stride;
      } // end WHILE

      break;
    }
    case V4L2_PIX_FMT_YUYV:
    {
      // decode YUYV to RGB (packed format)
      unsigned char* pointer_p = data_p;
      unsigned char* pixel_p = data_2;
      unsigned int number_of_pixels =
          (session_data_r.format.fmt.pix.height *
           session_data_r.format.fmt.pix.width);
      int Y1, Y2, Cr, Cb = 0;
      int R, G, B = 0;
      unsigned int RGB = 0;
      for (unsigned int i = 0;
           i < number_of_pixels;
           i += 2)
      {
        Y1 = *pointer_p;
        Cb = *(pointer_p + 1);
        Y2 = *(pointer_p + 2);
        Cr = *(pointer_p + 3);

        Y1 -= 16;
        Cb -= 128;
        Y2 -= 16;
        Cr -= 128;

        R = clamp (Y1 + ((454 * Cr) >> 8));
        G = clamp (Y1 - (((88 * Cr) + (183 * Cb)) >> 8));
        B = clamp (Y1 + ((359 * Cb) >> 8));
        RGB = static_cast<unsigned int> (B)         |
              (static_cast<unsigned int> (G) <<  8) |
              (static_cast<unsigned int> (R) << 16);
        *reinterpret_cast<unsigned int*> (pixel_p) = RGB;
        pixel_p += 4;

        R = clamp (Y2 + ((454 * Cr) >> 8));
        G = clamp (Y2 - (((88 * Cr) + (183 * Cb)) >> 8));
        B = clamp (Y2 + ((359 * Cb) >> 8));
        RGB = (static_cast<unsigned char> (R) << 16) |
              (static_cast<unsigned char> (G) <<  8) |
              static_cast<unsigned char> (B);
        *reinterpret_cast<unsigned int*> (pixel_p) = RGB;
        pixel_p += 4;

        pointer_p += 4;
      } // end FOR

      break;
    }
    // compressed formats
    case V4L2_PIX_FMT_MJPEG:
    {
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#else
      avcodec_register_all ();

      AVCodecContext* context_p = NULL;
      AVCodec* codec_p = avcodec_find_decoder (AV_CODEC_ID_MJPEG);
      if (!codec_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_find_decoder(AV_CODEC_ID_MJPEG): \"%m\", returning\n")));
        return;
      } // end IF
      AVFrame* frame_p = NULL;
//      frame_p = av_frame_alloc ();
//      if (!frame_p)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to av_frame_alloc(): \"%m\", returning\n")));
//        return;
//      } // end IF
//      av_frame_unref (frame_p);
      frame_p->data[0] = data_2;
      frame_p->linesize[0] = gdk_pixbuf_get_rowstride (pixelBuffer_);
      AVPacket packet;
      av_init_packet (&packet);
      packet.data = data_p;
      packet.size = session_data_r.format.fmt.pix.sizeimage;

      int result = -1;
      int got_picture = -1;
      int image_size = -1;
      context_p = avcodec_alloc_context3 (codec_p);
      if (!context_p)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_alloc_context3(): \"%m\", returning\n")));
        goto clean;
      } // end IF
      result = avcodec_get_context_defaults3 (context_p, codec_p);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_get_context_defaults3(): \"%m\", returning\n")));
        goto clean;
      } // end IF
//      context_p->flags2 |= AV_CODEC_FLAG2_FAST;
      context_p->pix_fmt = AVPixelFormat::AV_PIX_FMT_ARGB;
      context_p->width = session_data_r.format.fmt.pix.width;
      context_p->height = session_data_r.format.fmt.pix.height;
      result = avcodec_open2 (context_p, codec_p, NULL);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_open2(): \"%m\", returning\n")));
        goto clean;
      } // end IF

      result = avcodec_decode_video2 (context_p, frame_p, &got_picture, &packet);
      if ((result < 0) || !got_picture)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_decode_video2(): \"%m\", returning\n")));
        goto clean;
      } // end IF
      image_size = avpicture_get_size (context_p->pix_fmt,
                                       context_p->width, context_p->height);
      if (image_size != result)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_decode_video2(): \"%m\", returning\n")));
        goto clean;
      } // end IF

//      video_sws =
//          sws_getContext (context_p->width, context_p->height, context_p->pix_fmt,
//                          context_p->width, context_p->height,
//                          PIX_FMT_RGB24, SWS_BILINEAR, NULL, NULL, NULL);
//      sws_scale (video_sws, frame_p->data, frame_p->linesize, 0,
//                 context_p->height, frame_p->data, frame_p->linesize );
//      sws_freeContext (video_sws);

//      int size = avpicture_layout ((AVPicture*)frame_p, PIX_FMT_RGB24,
//                                   context_p->width, context_p->height,
//                                   (uint8_t*)RGB, avframe_rgb_size);
//      if (size != avframe_rgb_size)
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to avpicture_layout(): \"%m\", returning\n")));
//        goto clean;
//      } // end IF

clean:
//      if (frame_p)
//        av_frame_free (&frame_p);
      if (codec_p)
      {
        result = avcodec_close (context_p);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to avcodec_close(), continuing\n")));
      } // end IF
      if (context_p)
        avcodec_free_context (&context_p);
#endif

      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %d), returning\n"),
                  session_data_r.format.fmt.pix.pixelformat));
      return;
    }
  } // end SWITCH

  gdk_threads_enter ();

//  pixelBuffer_ =
//      gdk_pixbuf_new_from_data (data_p,                               // data
//                                GDK_COLORSPACE_RGB,                   // color space
//                                false,                                // has alpha channel ?
//                                bits_per_sample,                      // bits per sample
//                                session_data_r.format.fmt.pix.width,  // width
//                                session_data_r.format.fmt.pix.height, // height
//                                row_stride,                           // row stride
//                                NULL,                                 // destroy function
//                                NULL);                                // destroy function act
//  if (!pixelBuffer_)
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("failed to gdk_pixbuf_new_from_data(), returning\n")));

//    // clean up
//    gdk_threads_leave ();

//    return;
//  } // end IF

  // step3: draw pixbuf to image widget
  gint width, height;
  gdk_drawable_get_size (GDK_DRAWABLE (configuration_->image),
                         &width, &height);

  gdk_draw_pixbuf (GDK_DRAWABLE (configuration_->image), NULL,
                   pixelBuffer_,
                   0, 0, 0, 0, width, height,
                   GDK_RGB_DITHER_NONE, 0, 0);

  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either... :-(
  //         --> let the downstream event handler queue an idle request
  gdk_window_invalidate_rect (configuration_->window,
                              NULL,
                              false);
//  GtkWidget* widget_p = NULL;
//  gdk_window_get_user_data (configuration_->window,
//                            reinterpret_cast<gpointer*> (&widget_p));
//  ACE_ASSERT (widget_p);
//  gtk_widget_queue_draw (widget_p);
//  GtkAllocation allocation;
//  gtk_widget_get_allocation (widget_p,
//                             &allocation);
//  gtk_widget_queue_draw_area (widget_p,
//                              allocation.x, allocation.y,
//                              allocation.width, allocation.height);

  gdk_threads_leave ();
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                                bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::handleSessionMessage"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (!sessionData_);

      sessionData_ =
          &const_cast<SessionDataContainerType&> (message_inout->get ());
      sessionData_->increase ();
      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (sessionData_->get ());

      // sanity check(s)
      if (!configuration_->image)
        break; // done
      ACE_ASSERT (pixelBuffer_);

      int width = gdk_pixbuf_get_width (pixelBuffer_);
      int height = gdk_pixbuf_get_height (pixelBuffer_);
      ACE_ASSERT (width < session_data_r.format.fmt.pix.width);
      ACE_ASSERT (height < session_data_r.format.fmt.pix.height);

      break;

error:
      session_data_r.aborted = true;

      break;
    }
    case STREAM_SESSION_END:
    {
      if (pixelBuffer_)
      {
        g_object_unref (pixelBuffer_);
        pixelBuffer_ = NULL;
      } // end IF

      if (sessionData_)
      {
        sessionData_->decrease ();
        sessionData_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::initialize (const ConfigurationType& configuration_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::initialize"));

  if (isInitialized_)
  {
    configuration_ = NULL;

    if (pixelBuffer_)
    {
      g_object_unref (pixelBuffer_);
      pixelBuffer_ = NULL;
    } // end IF

    if (sessionData_)
    {
      sessionData_->decrease ();
      sessionData_ = NULL;
    } // end IF

    isFirst_ = true;

    isInitialized_ = false;
  } // end IF

  configuration_ = &configuration_in;

  // *TODO*: remove type inference
  if (configuration_->image)
  {
    gdk_threads_enter ();
    pixelBuffer_ = gtk_image_get_pixbuf (configuration_->image);
    if (!pixelBuffer_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to gtk_image_get_pixbuf(), aborting\n")));

      // clean up
      gdk_threads_leave ();

      return false;
    } // end IF
    gdk_threads_leave ();
  } // end IF

  isInitialized_ = true;

  return isInitialized_;
}
template <typename SessionMessageType,
          typename MessageType,
          typename ConfigurationType,
          typename SessionDataType,
          typename SessionDataContainerType>
const ConfigurationType&
Stream_Module_Vis_GTK_Pixbuf_T<SessionMessageType,
                               MessageType,
                               ConfigurationType,
                               SessionDataType,
                               SessionDataContainerType>::get () const
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Pixbuf_T::get"));

  // sanity check(s)
  ACE_ASSERT (configuration_);

  return *configuration_;
}
