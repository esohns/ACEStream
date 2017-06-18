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
#include <streams.h>
#else
#include "linux/videodev2.h"
#endif

extern "C"
{
#include "libavutil/imgutils.h"
}

#include "common_tools.h"

#include "stream_macros.h"
#include "stream_session_message_base.h"

#include "stream_dec_tools.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "stream_dev_directshow_tools.h"
#endif

#include "stream_vis_defines.h"
#include "stream_vis_tools.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                              SessionDataContainerType>::Stream_Module_Vis_GTK_Cairo_T (ISTREAM_T* stream_in)
#else
                              SessionDataContainerType>::Stream_Module_Vis_GTK_Cairo_T (typename inherited::ISTREAM_T* stream_in)
#endif
 : inherited (stream_in)
 , buffer_ (NULL)
 , codec_ (NULL)
 , codecContext_ (NULL)
 , frame_ (NULL)
// , cairoContext_ (NULL)
// , cairoSurface_ (NULL)
 , lock_ (NULL)
 , pixelBuffer_ (NULL)
 , isFirst_ (true)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::Stream_Module_Vis_GTK_Cairo_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::~Stream_Module_Vis_GTK_Cairo_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::~Stream_Module_Vis_GTK_Cairo_T"));

  if (buffer_)
    delete [] buffer_;

//  if (cairoSurface_)
//    cairo_surface_destroy (cairoSurface_);
//  if (cairoContext_)
//    cairo_destroy (cairoContext_);

  if (pixelBuffer_)
    g_object_unref (pixelBuffer_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::handleDataMessage (DataMessageType*& message_inout,
                                                                            bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);
  if (!inherited::configuration_->window)
    return; // done
//  if (inherited::configuration_->hasHeader &&
//      isFirst_)
//  {
//    isFirst_ = false;
//    return; // done
//  } // end IF
//  // *NOTE*: some formats (e.g. AVI frames) have (i.e. RIFF) 'chunk' header
//  //         prefixes that will have been prepended upstream (first
//  //         continuation) and need to be skipped over
//  if (inherited::configuration_->hasChunkHeaders)
//  {
//    ACE_ASSERT (message_block_p->cont ());
//    message_block_p = message_block_p->cont ();
//  } // end IF
  ACE_ASSERT (inherited::sessionData_);
//  ACE_ASSERT (cairoSurface_);
  ACE_ASSERT (pixelBuffer_);
  const SessionDataType& session_data_r = inherited::sessionData_->get ();

  // *NOTE*: 'crunching' the message data simplifies the data transformation
  //         algorithms, at the cost of several memory copies. This is a
  //         tradeoff that may warrant further optimization efforts
  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    return;
  }

  unsigned int width, height = 0;
  unsigned int image_size = 0;
  enum AVPixelFormat pixel_format = AV_PIX_FMT_NONE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  ACE_ASSERT (session_data_r.format);

  struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
  struct tagVIDEOINFOHEADER2* video_info_header_2 = NULL;
  if (session_data_r.format->formattype == FORMAT_VideoInfo)
    video_info_header_p =
      reinterpret_cast<struct tagVIDEOINFOHEADER*> (session_data_r.format->pbFormat);
  else if (session_data_r.format->formattype == FORMAT_VideoInfo2)
    video_info_header_2 =
      reinterpret_cast<struct tagVIDEOINFOHEADER2*> (session_data_r.format->pbFormat);
  else
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                inherited::mod_->name (),
                ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (session_data_r.format->formattype).c_str ())));
    goto error;
  } // end ELSE
  height =
    static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biHeight
                                                   : video_info_header_2->bmiHeader.biHeight);
  width =
    static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biWidth
                                                   : video_info_header_2->bmiHeader.biWidth);
  image_size =
    (video_info_header_p ? video_info_header_p->bmiHeader.biSizeImage
                         : video_info_header_2->bmiHeader.biSizeImage);
  pixel_format =
    Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (session_data_r.format->subtype);
#else
  width = session_data_r.width;
  height = session_data_r.height;
  image_size =
        av_image_get_buffer_size (session_data_r.format,
                                  width,
                                  height,
                                  1); // *TODO*: linesize alignment
  pixel_format = session_data_r.format;
#endif

  bool leave_gdk = false;
  bool release_lock = false;
  int result = -1;
  ACE_Message_Block* message_block_p = message_inout;
  unsigned int offset = 0;//, length = message_block_p->length ();
  unsigned char* data_p =
      reinterpret_cast<unsigned char*> (message_block_p->rd_ptr ());
  ACE_ASSERT (data_p);
  unsigned char* pixel_p = data_p;
  guchar* data_2 = NULL;
//  unsigned int* pixel_2 = NULL;
  guchar* data_3 = NULL;
  unsigned int* pixel_3 = NULL;
  int bytes_per_pixel = -1;
//  int bits_per_sample = 8;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  LONG row_stride = -1;
#else
  int row_stride = -1;
#endif
  int row_stride_2 = -1;

  if (lock_)
  {
    result = lock_->acquire ();
    if (result == -1)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::acquire(): \"%m\", returning\n")));
      return;
    } // end IF
    release_lock = true;
  } // end IF

  //gdk_threads_enter ();
  //leave_gdk = true;

//  unsigned char* data_2 = cairo_image_surface_get_data (cairoSurface_);
  data_2 = gdk_pixbuf_get_pixels (pixelBuffer_);
  ACE_ASSERT (data_2);
//  pixel_2 = reinterpret_cast<unsigned int*> (data_2);
  bytes_per_pixel = ((gdk_pixbuf_get_bits_per_sample (pixelBuffer_) / 8) * 4);
  ACE_ASSERT (bytes_per_pixel == 4);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  row_stride =
    ((((width * (video_info_header_p ? video_info_header_p->bmiHeader.biBitCount
                                     : video_info_header_2->bmiHeader.biBitCount)) + 31) & ~31) >> 3);
#else
  row_stride = av_image_get_linesize (session_data_r.format,
                                      width,
                                      0);
#endif

  int pixbuf_height = gdk_pixbuf_get_height (pixelBuffer_);
  int pixbuf_width = gdk_pixbuf_get_width (pixelBuffer_);
  bool scale_image =
      ((static_cast<int> (width) != pixbuf_width) ||
       (static_cast<int> (height) != pixbuf_height));
  uint8_t* in_data[4] = { NULL, NULL, NULL, NULL };
  uint8_t* out_data[4] = { NULL, NULL, NULL, NULL };

  data_3 = (scale_image ? buffer_ : data_2);
  ACE_ASSERT (data_3);
  pixel_3 = (scale_image ? reinterpret_cast<unsigned int*> (data_3)
                         : reinterpret_cast<unsigned int*> (data_2));
  ACE_ASSERT (pixel_3);
  row_stride_2 = (scale_image ? row_stride
                              : gdk_pixbuf_get_rowstride (pixelBuffer_));

  switch (pixel_format)
  {
    // RGB formats
    case AV_PIX_FMT_BGR24:
    {
      // convert BGR (24bit) to RGB (24bit)
//      row_stride =
//          cairo_format_stride_for_width (MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                                         session_data_r.format.fmt.pix.width);
      for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
          pixel_p = data_p + offset;
          pixel_3 =
              (unsigned int*)(data_3 + ((y * row_stride_2) + (x * 3)));

          *pixel_3 = ((*(pixel_p + 2) << 16) |
                      (*(pixel_p + 1) << 8)  |
                      *pixel_p);

          offset += 3;
        } // end FOR

      break;
    }
    case AV_PIX_FMT_RGB24:
    {
      // convert RGB (24bit) to RGB (24bit)
//      row_stride =
//          cairo_format_stride_for_width (MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                                         session_data_r.format.fmt.pix.width);
      for (unsigned int y = 0; y < height; ++y)
        for (unsigned int x = 0; x < width; ++x)
        {
          pixel_p = data_p + offset;
          pixel_3 =
              (unsigned int*)(data_3 + ((y * row_stride_2) + (x * 3)));

          *pixel_3 = (*pixel_p        << 16) |
                      (*(pixel_p + 1) << 8)  |
                      (*(pixel_p + 2));

          offset += 3;
        } // end FOR

      break;
    }
    // luminance / chrominance formats
    case AV_PIX_FMT_YUV420P:
    {
      // decode YVU to RGB24 (planar format)
//      cairo_surface_t* cairo_surface_p =
//          cairo_image_surface_create (MODULE_VIS_DEFAULT_CAIRO_FORMAT,       // format
//                                      session_data_r.format.fmt.pix.width,   // width
//                                      session_data_r.format.fmt.pix.height); // height
//      cairo_status_t status = cairo_surface_status (cairo_surface_p);
//      if (!cairo_surface_p ||
//          (status != CAIRO_STATUS_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to cairo_image_surface_create(%d,%d,%d): \"%s\", returning\n"),
//                    MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                    session_data_r.format.fmt.pix.width, session_data_r.format.fmt.pix.height,
//                    ACE_TEXT (cairo_status_to_string (status))));
//        return;
//      } // end IF

//      int row_stride_2 =
//          cairo_format_stride_for_width (MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                                         session_data_r.format.fmt.pix.width);
      // decode YVU to RGB24 (planar format)
      unsigned int number_of_pixels = height * width;
      unsigned char* chrominance_b_base_p = data_p           +
                                            number_of_pixels +
                                            (number_of_pixels / 4);
      unsigned char* chrominance_r_base_p = data_p + number_of_pixels;
      unsigned char* chrominance_b_p = chrominance_b_base_p; // Cb
      unsigned char* chrominance_r_p = chrominance_r_base_p; // Cr
      unsigned char* R1_p = data_3;
      unsigned char* G1_p = R1_p + 1;
      unsigned char* B1_p = R1_p + 2;
      unsigned char* R2_p = R1_p + row_stride_2;
      unsigned char* G2_p = R2_p + 1;
      unsigned char* B2_p = R2_p + 2;
      unsigned char* Y1_p = data_p;
      unsigned char* Y2_p = Y1_p + row_stride;
      unsigned char* end_p = R1_p + (row_stride_2 * height);
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

          R1_p += 3; G1_p += 3; B1_p += 3;
          R2_p += 3; G2_p += 3; B2_p += 3;

          ++Y1_p; ++Y2_p;

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

          R1_p += 3; G1_p += 3; B1_p += 3;
          R2_p += 3; G2_p += 3; B2_p += 3;

          ++Y1_p; ++Y2_p;

          ++chrominance_b_p; ++chrominance_r_p;
        } // end WHILE

        R1_p += row_stride_2; G1_p += row_stride_2; B1_p += row_stride_2;
        R2_p += row_stride_2; G2_p += row_stride_2; B2_p += row_stride_2;
        Y1_p += row_stride; Y2_p += row_stride;
      } // end WHILE

//      cairo_surface_destroy (cairoSurface_);
//      cairoSurface_ = cairo_surface_p;

//      data_p = cairo_image_surface_get_data (cairoSurface_);
//      row_stride =
//          cairo_format_stride_for_width (cairoFormat_,
//                                         session_data_r.format.fmt.pix.width);
      break;
    }
    case AV_PIX_FMT_YUV420P16:
    {
      // decode YUV to RGB24 (planar format)
//      cairo_surface_t* cairo_surface_p =
//          cairo_image_surface_create (MODULE_VIS_DEFAULT_CAIRO_FORMAT,       // format
//                                      session_data_r.format.fmt.pix.width,   // width
//                                      session_data_r.format.fmt.pix.height); // height
//      cairo_status_t status = cairo_surface_status (cairo_surface_p);
//      if (!cairo_surface_p ||
//          (status != CAIRO_STATUS_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to cairo_image_surface_create(%d,%d,%d): \"%s\", returning\n"),
//                    MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                    session_data_r.format.fmt.pix.width, session_data_r.format.fmt.pix.height,
//                    ACE_TEXT (cairo_status_to_string (status))));
//        return;
//      } // end IF

//      int row_stride_2 =
//          cairo_format_stride_for_width (MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                                         session_data_r.format.fmt.pix.width);
      unsigned int number_of_pixels = height * width;
      unsigned char* chrominance_b_base_p = data_p + number_of_pixels;
      unsigned char* chrominance_r_base_p = data_p +
                                            number_of_pixels +
                                            (number_of_pixels / 4);
      unsigned char* chrominance_b_p = chrominance_b_base_p; // Cb
      unsigned char* chrominance_r_p = chrominance_r_base_p; // Cr
      unsigned char* R1_p = data_3;
      unsigned char* G1_p = R1_p + 1;
      unsigned char* B1_p = R1_p + 2;
      unsigned char* R2_p = R1_p + row_stride_2;
      unsigned char* G2_p = R2_p + 1;
      unsigned char* B2_p = R2_p + 2;
      unsigned char* Y1_p = data_p;
      unsigned char* Y2_p = Y1_p + row_stride;
      unsigned char* end_p = R1_p + (row_stride_2 * height);
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

          R1_p += 3; G1_p += 3; B1_p += 3;
          R2_p += 3; G2_p += 3; B2_p += 3;

          ++Y1_p; ++Y2_p;

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

          R1_p += 3; G1_p += 3; B1_p += 3;
          R2_p += 3; G2_p += 3; B2_p += 3;

          ++Y1_p; ++Y2_p;

          ++chrominance_b_p; ++chrominance_r_p;
        } // end WHILE

        R1_p += row_stride_2; G1_p += row_stride_2; B1_p += row_stride_2;
        R2_p += row_stride_2; G2_p += row_stride_2; B2_p += row_stride_2;
        Y1_p += row_stride; Y2_p += row_stride;
      } // end WHILE

//      cairo_surface_destroy (cairoSurface_);
//      cairoSurface_ = cairo_surface_p;

//      data_p = cairo_image_surface_get_data (cairoSurface_);
//      row_stride =
//          cairo_format_stride_for_width (cairoFormat_,
//                                         session_data_r.format.fmt.pix.width);
      break;
    }
    case AV_PIX_FMT_YUYV422:
    {
      // decode YUYV to RGB24 (packed format)
//      cairo_surface_t* cairo_surface_p =
//          cairo_image_surface_create (MODULE_VIS_DEFAULT_CAIRO_FORMAT,       // format
//                                      session_data_r.format.fmt.pix.width,   // width
//                                      session_data_r.format.fmt.pix.height); // height
//      cairo_status_t status = cairo_surface_status (cairo_surface_p);
//      if (!cairo_surface_p ||
//          (status != CAIRO_STATUS_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to cairo_image_surface_create(%d,%d,%d): \"%s\", returning\n"),
//                    MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                    session_data_r.format.fmt.pix.width, session_data_r.format.fmt.pix.height,
//                    ACE_TEXT (cairo_status_to_string (status))));
//        return;
//      } // end IF

      // decode YUYV to RGB24 (packed format)
      unsigned char* pointer_p = data_p;
      unsigned char* pixel_p = data_3;
      unsigned int number_of_pixels = height * width;
      int Y1, Y2, Cr, Cb = 0;
      int R, G, B = 0;
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

        *pixel_p       = static_cast<unsigned char> (R);
        *(pixel_p + 1) = static_cast<unsigned char> (G);
        *(pixel_p + 2) = static_cast<unsigned char> (B);
        pixel_p += 3;

        R = clamp (Y2 + ((454 * Cr) >> 8));
        G = clamp (Y2 - (((88 * Cr) + (183 * Cb)) >> 8));
        B = clamp (Y2 + ((359 * Cb) >> 8));

        *pixel_p       = static_cast<unsigned char> (R);
        *(pixel_p + 1) = static_cast<unsigned char> (G);
        *(pixel_p + 2) = static_cast<unsigned char> (B);
        pixel_p += 3;

        // skip to beginning of next row
        if (i && !(i % row_stride)) pixel_p += (row_stride_2 - width);

        pointer_p += 4;
      } // end FOR

//      cairo_surface_destroy (cairoSurface_);
//      cairoSurface_ = cairo_surface_p;

//      data_p = cairo_image_surface_get_data (cairoSurface_);
//      row_stride =
//          cairo_format_stride_for_width (cairoFormat_,
//                                         session_data_r.format.fmt.pix.width);
      break;
    }
    // compressed formats
    case AV_PIX_FMT_YUVJ422P:
    {
//      cairo_surface_t* cairo_surface_p =
//          cairo_image_surface_create (MODULE_VIS_DEFAULT_CAIRO_FORMAT,       // format
//                                      session_data_r.format.fmt.pix.width,   // width
//                                      session_data_r.format.fmt.pix.height); // height
//      cairo_status_t status = cairo_surface_status (cairo_surface_p);
//      if (!cairo_surface_p ||
//          (status != CAIRO_STATUS_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to cairo_image_surface_create(%d,%d,%d): \"%s\", returning\n"),
//                    MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                    session_data_r.format.fmt.pix.width, session_data_r.format.fmt.pix.height,
//                    ACE_TEXT (cairo_status_to_string (status))));
//        return;
//      } // end IF

      ACE_ASSERT (codecContext_);
      ACE_ASSERT (codec_);
      ACE_ASSERT (frame_);
//      frame_p->data[0] = cairo_image_surface_get_data (cairo_surface_p);
      frame_->data[0] = data_3;
      frame_->linesize[0] = gdk_pixbuf_get_rowstride (pixelBuffer_);
//      frame_p->linesize[0] =
//          cairo_format_stride_for_width (MODULE_VIS_DEFAULT_CAIRO_FORMAT,
//                                         session_data_r.format.fmt.pix.width);
      struct AVPacket packet_s;
      av_init_packet (&packet_s);
      packet_s.data = data_p;
      packet_s.size = image_size;
      int got_picture = -1;
      int image_size_2 =
        avpicture_get_size (codecContext_->pix_fmt,
                            codecContext_->width, codecContext_->height);
      result =
        avcodec_decode_video2 (codecContext_,
                               frame_,
                               &got_picture,
                               &packet_s);
      if (((result < 0) || !got_picture) ||
          (result != image_size_2))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_decode_video2(): \"%m\", returning\n")));
        goto clean;
      } // end IF

clean:
//      cairo_surface_destroy (cairoSurface_);
//      cairoSurface_ = cairo_surface_p;

//      data_p = cairo_image_surface_get_data (cairoSurface_);
//      row_stride =
//          cairo_format_stride_for_width (cairoFormat_,
//                                         session_data_r.format.fmt.pix.width);
      break;
    }
    default:
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("invalid/unknown pixel format (was: %s), returning\n"),
                  ACE_TEXT (Stream_Module_Decoder_Tools::pixelFormatToString (pixel_format).c_str ())));

      result = -1;

      goto unlock;
    }
  } // end SWITCH

  result = 0;

  // step3: scale image to window area
  if (!scale_image)
    goto unlock; // done

  in_data[0] = buffer_;
  out_data[0] = static_cast<uint8_t*> (data_2);
  if (!Stream_Module_Decoder_Tools::scale (NULL,
                                           width, height, AV_PIX_FMT_RGB24,
                                           in_data,
                                           pixbuf_width, pixbuf_height, AV_PIX_FMT_RGB24,
                                           out_data))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to Stream_Module_Decoder_Tools::scale(), returning\n")));

    result = -1;

    goto unlock;
  } // end IF

unlock:
  if (release_lock)
  {
    ACE_ASSERT (lock_);
    result = lock_->release ();
    if (result == -1)
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("failed to ACE_SYNCH_RECURSIVE_MUTEX::release(): \"%m\", continuing\n")));
  } // end IF

  if (result == -1)
    goto error;

//  gint x, y;
//  gdk_window_get_origin (configuration_->window,
//                         &x, &y);
//  gint width, height;
//  gdk_drawable_get_size (GDK_DRAWABLE (configuration_->window),
//                         &width, &height);
//  double scale_factor, scale_factor_x, scale_factor_y;
//  scale_factor_x =
//      width / static_cast<double> (sessionData_->format.fmt.pix.width);
//  scale_factor_y =
//      height / static_cast<double> (sessionData_->format.fmt.pix.height);
//  scale_factor = MIN (scale_factor_x, scale_factor_y);
//  cairo_scale (cairoContext_,
//               scale_factor, scale_factor);

  //  cairo_surface_set_mime_data (cairoSurface_,                                               // surface
  //                               ACE_TEXT_ALWAYS_CHAR ("video/x-raw-yuv/i420"),               // MIME type
  //                               reinterpret_cast<unsigned char*> (message_inout->rd_ptr ()), // data
  //                               message_inout->length (),                                    // length
  //                               NULL,                                                        // destroy function
  //                               NULL);                                                       // destroy function act

  //  pixmap = gdk_pixmap_new(window->window, width, height, -1);
  //  gdk_draw_pixbuf((GdkDrawable *) pixmap, NULL,
  //                  pixbuf,
  //                  0, 0, 0, 0, wifth, height,
  //                  GDK_RGB_DITHER_NORMAL, 0, 0);
  //  gtk_image_set_from_pixmap((GtkImage*) image, pixmap, NULL);
//  gdk_cairo_set_source_pixbuf (cairoContext_,
//                               pixelBuffer_,
//                               0.0, 0.0);

  // step4: paste the pixel buffer into the specified window area
//  cairo_paint (cairoContext_);
//  cairo_surface_flush (cairoSurface_);

  // step5: schedule an 'expose' event
  // *NOTE*: gdk_window_invalidate_rect() is not thread-safe. It will race with
  //         the UI refresh and eventually crash (even though gdk_threads_enter/
  //         gdk_threads_leave is in effect)
  //         --> schedule a refresh with gtk_widget_queue_draw_area() instead
  // *NOTE*: this does not work either... :-(
  //         --> let the downstream event handler queue an idle request
  //gdk_window_invalidate_rect (inherited::configuration_->gdkWindow,
  //                            NULL,
  //                            false);
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

error:
  if (leave_gdk)
    gdk_threads_leave ();
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
void
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                               bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  int result = -1;

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      if (!inherited::configuration_->window)
        break; // done
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (pixelBuffer_);

      unsigned int width, height = 0;
      enum AVCodecID codec_id = AV_CODEC_ID_NONE;
      enum AVPixelFormat pixel_format = AV_PIX_FMT_NONE;
#if defined (ACE_WIN32) || defined (ACE_WIN64)
      ACE_ASSERT (inherited::configuration_->format);
      struct tagVIDEOINFOHEADER* video_info_header_p = NULL;
      struct tagVIDEOINFOHEADER2* video_info_header_2 = NULL;
      struct _AMMediaType* media_type_p =
        &getFormat (inherited::configuration_->format);
      ACE_ASSERT (media_type_p);
      if (media_type_p->formattype == FORMAT_VideoInfo)
        video_info_header_p =
          reinterpret_cast<struct tagVIDEOINFOHEADER*> (media_type_p->pbFormat);
      else if (media_type_p->formattype == FORMAT_VideoInfo2)
        video_info_header_2 =
          reinterpret_cast<struct tagVIDEOINFOHEADER2*> (media_type_p->pbFormat);
      else
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: invalid/unknown format type (was: %s), aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Stream_Module_Decoder_Tools::GUIDToString (media_type_p->formattype).c_str ())));

        // clean up
        Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);

        goto error;
      } // end ELSE
      height =
        static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biHeight
                                                       : video_info_header_2->bmiHeader.biHeight);
      width =
        static_cast<unsigned int> (video_info_header_p ? video_info_header_p->bmiHeader.biWidth
                                                       : video_info_header_2->bmiHeader.biWidth);
      codec_id =
        Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVCodecID (media_type_p->subtype);//,
                                                                  //inherited::configuration_->useMediaFoundation);
      pixel_format =
        Stream_Module_Decoder_Tools::mediaTypeSubTypeToAVPixelFormat (media_type_p->subtype);//,
                                                                      //inherited::configuration_->useMediaFoundation);
      Stream_Module_Device_DirectShow_Tools::deleteMediaType (media_type_p);
#else
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);

      SessionDataType& session_data_r =
          const_cast<SessionDataType&> (inherited::sessionData_->get ());
      height = session_data_r.height;
      width = session_data_r.width;
      codec_id =
        Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecID (session_data_r.format);
      pixel_format = session_data_r.format;

//      // sanity check(s)
//      ACE_ASSERT (inherited::configuration_);

//      height = inherited::configuration_->height;
//      width = inherited::configuration_->width;
//      codec_id =
//        Stream_Module_Decoder_Tools::AVPixelFormatToAVCodecID (inherited::configuration_->format);
//      pixel_format = inherited::configuration_->format;
#endif
      unsigned int width_window =
          static_cast<unsigned int> (gdk_pixbuf_get_width (pixelBuffer_));
      unsigned int height_window =
          static_cast<unsigned int> (gdk_pixbuf_get_height (pixelBuffer_));

//      ACE_ASSERT (cairoContext_);
//      ACE_ASSERT (!cairoSurface_);

      // step1: allocate/retrieve a pixel buffer
//      cairo_status_t result = CAIRO_STATUS_SUCCESS;
//      cairo_format_t format = CAIRO_FORMAT_INVALID;
//      int width, height;
////      cairoSurface_ =
////          cairo_image_surface_create (MODULE_VIS_DEFAULT_CAIRO_FORMAT,       // format
////                                      session_data_r.format.fmt.pix.width,   // width
////                                      session_data_r.format.fmt.pix.height); // height
////      result = cairo_surface_status (cairoSurface_);
////      if (!cairoSurface_ ||
////          (result != CAIRO_STATUS_SUCCESS))
////      {
////        ACE_DEBUG ((LM_ERROR,
////                    ACE_TEXT ("failed to cairo_image_surface_create(%d,%d,%d): \"%s\", returning\n"),
////                    cairoFormat_,
////                    session_data_r.format.fmt.pix.width, session_data_r.format.fmt.pix.height,
////                    ACE_TEXT (cairo_status_to_string (result))));
////        goto error;
////      } // end IF
////      cairo_set_source_surface (cairoContext_, // context
////                                cairoSurface_, // surface
////    //                            (width  - (scale_factor * width))  / (2.0 * scale_factor),
////    //                            (height - (scale_factor * height)) / (2.0 * scale_factor));
////                                0.0, 0.0);     // x,y offset
//      cairo_pattern_t* pattern_p = cairo_get_source (cairoContext_);
//      ACE_ASSERT (pattern_p);
////      cairo_pattern_type_t pattern_type = cairo_pattern_get_type (pattern_p);
////      ACE_ASSERT (pattern_type == CAIRO_PATTERN_TYPE_SURFACE);
//      result = cairo_pattern_get_surface (pattern_p,
//                                          &cairoSurface_);
//      if (!cairoSurface_ ||
//          (result != CAIRO_STATUS_SUCCESS))
//      {
//        ACE_DEBUG ((LM_ERROR,
//                    ACE_TEXT ("failed to cairo_pattern_get_surface(): \"%s\", aborting\n"),
//                    ACE_TEXT (cairo_status_to_string (result))));
//        goto error;
//      } // end IF
//      format = cairo_image_surface_get_format (cairoSurface_);
//      width = cairo_image_surface_get_width (cairoSurface_);
//      height = cairo_image_surface_get_height (cairoSurface_);
//      ACE_ASSERT (format == MODULE_VIS_DEFAULT_CAIRO_FORMAT);
//      ACE_ASSERT (width  >= static_cast<int> (session_data_r.format.fmt.pix.width));
//      ACE_ASSERT (height >= static_cast<int> (session_data_r.format.fmt.pix.height));

      // sanity check(s)
      ACE_ASSERT (!codec_);
      ACE_ASSERT (!codecContext_);
      ACE_ASSERT (!frame_);
      if (codec_id == AV_CODEC_ID_NONE)
        goto continue_;
      codec_ = avcodec_find_decoder (codec_id);
      if (!codec_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_find_decoder(%d): \"%m\", returning\n"),
                    codec_id));
        goto error;
      } // end IF
      codecContext_ = avcodec_alloc_context3 (codec_);
      if (!codecContext_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_alloc_context3(): \"%m\", returning\n")));
        goto error;
      } // end IF
      result = avcodec_get_context_defaults3 (codecContext_, codec_);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_get_context_defaults3(): \"%m\", returning\n")));
        goto error;
      } // end IF
//      codecContext_->flags2 |= AV_CODEC_FLAG2_FAST;
      //codecContext_->pix_fmt = AV_PIX_FMT_ARGB;
      codecContext_->width = width;
      codecContext_->height = height;
      result = avcodec_open2 (codecContext_,
                              codec_,
                              NULL);
      if (result < 0)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_open2(): \"%m\", returning\n")));
        goto error;
      } // end IF
      frame_ = av_frame_alloc ();
      if (!frame_)
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to av_frame_alloc(): \"%m\", returning\n")));
        goto error;
      } // end IF

continue_:
      if ((height != height_window) || (width != width_window))
      {
        unsigned int image_size =
          av_image_get_buffer_size ((pixel_format != AV_PIX_FMT_NONE) ? pixel_format : AV_PIX_FMT_RGBA,
                                    width, height,
                                    1); // *TODO*: linesize alignment
        ACE_NEW_NORETURN (buffer_,
                          uint8_t[image_size]);
        if (!buffer_)
        {
          ACE_DEBUG ((LM_CRITICAL,
                      ACE_TEXT ("%s: failed to allocate memory(%u): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      image_size));
          goto error;
        } // end IF

        ACE_DEBUG ((LM_DEBUG,
                    ACE_TEXT ("%s: scaling frame(s) (size: %ux%u) to %ux%u...\n"),
                    inherited::mod_->name (),
                    width, height,
                    width_window, height_window));
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      if (buffer_)
      {
        delete [] buffer_;
        buffer_ = NULL;
      } // end IF

      codec_ = NULL;
      if (codecContext_)
      {
        result = avcodec_close (codecContext_);
        if (result < 0)
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("failed to avcodec_close(), continuing\n")));
        avcodec_free_context (&codecContext_);
      } // end IF
      if (frame_)
      {
        av_frame_unref (frame_);
        frame_ = NULL;
      } // end IF

//      if (cairoSurface_)
//      {
//        cairo_surface_destroy (cairoSurface_);
//        cairoSurface_ = NULL;
//      } // end IF

      if (pixelBuffer_)
      {
        g_object_unref (pixelBuffer_);
        pixelBuffer_ = NULL;
      } // end IF

      break;
    }
    default:
      break;
  } // end SWITCH
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
bool
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::initialize (const ConfigurationType& configuration_in,
                                                                     Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::initialize"));

  int result = -1;

  if (inherited::isInitialized_)
  {
    if (buffer_)
    {
      delete[] buffer_;
      buffer_ = NULL;
    } // end IF

    codec_ = NULL;
    if (codecContext_)
    {
      result = avcodec_close (codecContext_);
      if (result < 0)
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to avcodec_close(), continuing\n")));
      avcodec_free_context (&codecContext_);
    } // end IF
    if (frame_)
    {
      av_frame_unref (frame_);
      frame_ = NULL;
    } // end IF

//    if (cairoSurface_)
//    {
//      cairo_surface_destroy (cairoSurface_);
//      cairoSurface_ = NULL;
//    } // end IF

//    if (cairoContext_)
//    {
//      cairo_destroy (cairoContext_);
//      cairoContext_ = NULL;
//    } // end IF

    if (pixelBuffer_)
    {
      g_object_unref (pixelBuffer_);
      pixelBuffer_ = NULL;
    } // end IF

    isFirst_ = true;
  } // end IF

  lock_ = configuration_in.lock;
  if (!configuration_in.pixelBuffer)
  {
    // *TODO*: remove type inference
    if (configuration_in.window)
    {
      gdk_threads_enter ();

      // sanity check(s)
      // *TODO*: remove type inference
      ACE_ASSERT (!configuration_in.pixelBuffer);

      // *TODO*: remove type inference
      pixelBuffer_ =
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
          gdk_pixbuf_get_from_window (configuration_in.window,
                                      0, 0,
                                      configuration_in.area.width, configuration_in.area.height);
#else
        gdk_pixbuf_get_from_drawable (NULL,
                                      GDK_DRAWABLE (configuration_in.window),
                                      NULL,
                                      0, 0,
                                      0, 0, configuration_in.area.width, configuration_in.area.height);
#endif
      if (!pixelBuffer_)
      { // *NOTE*: most probable reason: window is not mapped
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("failed to gdk_pixbuf_get_from_window(), aborting\n")));

        // clean up
        gdk_threads_leave ();

        return false;
      } // end IF
      gdk_threads_leave ();
    } // end IF
  } // end IF
  else
  {
    g_object_ref (configuration_in.pixelBuffer);
    pixelBuffer_ = configuration_in.pixelBuffer;
  } // end ELSE
  if (!pixelBuffer_)
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to obtain pixel buffer, aborting\n")));
    return false;
  } // end IF

//  // *TODO*: remove type inference
//  if (configuration_->window)
//  {
//    ACE_ASSERT (!cairoContext_);

//    cairoContext_ = gdk_cairo_create (GDK_DRAWABLE (configuration_->window));
//    if (!cairoContext_)
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("failed to gdk_cairo_create(), aborting\n")));
//      return false;
//    } // end IF
//    gdk_cairo_set_source_pixbuf (cairoContext_,
//                                 pixelBuffer_,
//                                 0.0, 0.0);
//  } // end IF

  if (isFirst_)
    avcodec_register_all ();

  return inherited::initialize (configuration_in,
                                allocator_in);
}

#if defined (ACE_WIN32) || defined (ACE_WIN64)
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
AM_MEDIA_TYPE&
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::getFormat_impl (const struct _AMMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;
  if (!Stream_Module_Device_DirectShow_Tools::copyMediaType (*format_in,
                                                             result_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_Module_Device_DirectShow_Tools::copyMediaType(), aborting\n"),
                inherited::mod_->name ()));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataType,
          typename SessionDataContainerType>
AM_MEDIA_TYPE&
Stream_Module_Vis_GTK_Cairo_T<ACE_SYNCH_USE,
                              TimePolicyType,
                              ConfigurationType,
                              ControlMessageType,
                              DataMessageType,
                              SessionMessageType,
                              SessionDataType,
                              SessionDataContainerType>::getFormat_impl (const IMFMediaType* format_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_GTK_Cairo_T::getFormat_impl"));

  // sanity check(s)
  ACE_ASSERT (format_in);

  struct _AMMediaType* result_p = NULL;

  HRESULT result =
    MFCreateAMMediaTypeFromMFMediaType (const_cast<IMFMediaType*> (format_in),
                                        GUID_NULL,
                                        &result_p);
  if (FAILED (result))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("failed to MFCreateAMMediaTypeFromMFMediaType(): \"%s\", aborting\n"),
                ACE_TEXT (Common_Tools::error2String (result).c_str ())));
    return struct _AMMediaType (); // *TODO*: will crash
  } // end IF
  ACE_ASSERT (result_p);

  return *result_p;
}
#endif
