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

#ifdef __cplusplus
extern "C"
{
#include "libavutil/frame.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
}
#endif /* __cplusplus */

#include "X11/Xutil.h"

#include "ace/Log_Msg.h"

#include "common_ui_defines.h"

#include "stream_macros.h"

#include "stream_dec_tools.h"

#include "stream_lib_tools.h"

#include "stream_vis_defines.h"

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_X11_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::Stream_Module_Vis_X11_Window_T (typename inherited::ISTREAM_T* stream_in)
 : inherited (stream_in)
 , buffer_ (NULL)
 , closeDisplay_ (false)
 , closeWindow_ (false)
// , context_ ()
 , display_ (NULL)
 , isFirst_ (true)
 , scaleContext_ (NULL)
 , scaleContextHeight_ (0)
 , scaleContextWidth_ (0)
 , window_ (0)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_X11_Window_T::Stream_Module_Vis_X11_Window_T"));

}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
Stream_Module_Vis_X11_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::~Stream_Module_Vis_X11_Window_T ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_X11_Window_T::~Stream_Module_Vis_X11_Window_T"));

  int result = -1;

  if (buffer_)
    delete [] buffer_;
//  if (context_)
//  { ACE_ASSERT (display_);
//    result = XFreeGC (display_,
//                      context_);
//    if (unlikely (result))
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to XFreeGC(0x%@,%u): \"%m\", continuing\n"),
//                  inherited::mod_->name (),
//                  display_));
//  } // end IF
  if (pixmap_)
  { ACE_ASSERT (display_);
    result = XFreePixmap (display_,
                          pixmap_);
    if (unlikely (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XFreePixmap(0x%@,%u): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_, pixmap_));
  } // end IF
  if (closeWindow_)
  { ACE_ASSERT (display_ && window_);
    result = XUnmapWindow (display_, window_);
    if (unlikely (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XUnmapWindow(0x%@,%u): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_, window_));
  } // end IF
  if (closeDisplay_)
  { ACE_ASSERT (display_);
    result = XCloseDisplay (display_);
    if (unlikely (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XCloseDisplay(0x%@): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_));
  } // end IF

  if (scaleContext_)
    sws_freeContext (scaleContext_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_X11_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleDataMessage (DataMessageType*& message_inout,
                                                              bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_X11_Window_T::handleDataMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::sessionData_);
  ACE_ASSERT (display_ && window_);

  const typename SessionDataContainerType::DATA_T& session_data_r =
      inherited::sessionData_->getR ();
  const MediaType& media_type_r = session_data_r.formats.front ();
  enum AVPixelFormat pixel_format_e = getFormat (media_type_r);
  Common_UI_Resolution_t resolution_s = getResolution (media_type_r);
  Common_UI_Resolution_t resolution_2 =
      Stream_MediaFramework_Tools::toResolution (*display_,
                                                 window_);
  unsigned int row_stride = 0, row_stride_2 = 0;
  bool release_lock = false;
  Status result_2 = -1;
  bool transform_image = false;
  uint8_t* in_data[AV_NUM_DATA_POINTERS];
  uint8_t* out_data[AV_NUM_DATA_POINTERS];
  XImage* image_p = NULL;

  // sanity check(s)
  ACE_ASSERT (pixel_format_e != AV_PIX_FMT_NONE);
  ACE_ASSERT (resolution_s.width && resolution_s.height);

//  image_size =
//      av_image_get_buffer_size (pixel_format_e,
//                                resolution_s.width,
//                                resolution_s.height,
//                                1); // *TODO*: linesize alignment
  row_stride =
      av_image_get_linesize (pixel_format_e,
                             resolution_s.width,
                             0);
  row_stride_2 =
      av_image_get_linesize (AV_PIX_FMT_RGBA,
                             resolution_2.width,
                             0);

  // *NOTE*: 'crunching' the message data simplifies the data transformation
  //         algorithms, at the cost of (several) memory copies. This is a
  //         tradeoff that may warrant further optimization efforts
  try {
    message_inout->defragment ();
  } catch (...) {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to Stream_IDataMessage_T::defragment(), returning\n"),
                inherited::mod_->name ()));
    return;
  }

  transform_image =
      ((pixel_format_e != AV_PIX_FMT_RGBA) ||
       ((resolution_s.width != resolution_2.width) || (resolution_s.height != resolution_2.height)));
  if (unlikely (transform_image &&
                ((resolution_2.height != scaleContextHeight_) || (resolution_2.width != scaleContextWidth_))))
  {
    if (scaleContext_)
    {
      sws_freeContext (scaleContext_); scaleContext_ = NULL;
    } // end IF
    int flags = (SWS_FAST_BILINEAR | SWS_ACCURATE_RND);
//                 SWS_LANCZOS | SWS_ACCURATE_RND);
    scaleContext_ =
        sws_getCachedContext (NULL,
                              resolution_s.width, resolution_s.height, pixel_format_e,
                              resolution_2.width, resolution_2.height, AV_PIX_FMT_RGBA,
                              flags,                             // flags
                              NULL, NULL,
                              0);                                // parameters
    if (!scaleContext_)
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to sws_getCachedContext(): \"%m\", aborting\n"),
                  inherited::mod_->name ()));
      goto unlock;
    } // end IF
    scaleContextHeight_ = resolution_2.height;
    scaleContextWidth_ = resolution_2.width;
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: scaling frame(s) (resolution: %ux%u) to %ux%u\n"),
                inherited::mod_->name (),
                resolution_s.width, resolution_s.height,
                resolution_2.width, resolution_2.height));
#endif // _DEBUG
  } // end IF

  // step3: transform image?
  if (transform_image)
  {
//    ACE_OS::memset (in_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
//    ACE_OS::memset (out_data, 0, sizeof (uint8_t*[AV_NUM_DATA_POINTERS]));
    in_data[0] = reinterpret_cast<uint8_t*> (message_inout->rd_ptr ());
    out_data[0] = buffer_;
    if (!Stream_Module_Decoder_Tools::convert (scaleContext_,
                                               resolution_s.width, resolution_s.height, pixel_format_e,
                                               in_data,
                                               resolution_2.width, resolution_2.height, AV_PIX_FMT_RGBA,
                                               out_data))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Stream_Module_Decoder_Tools::convert(), returning\n"),
                  inherited::mod_->name ()));
      goto unlock;
    } // end IF
  } // end IF

  image_p =
      XCreateImage (display_,
                    DefaultVisual (display_, DefaultScreen (display_)),
                    DefaultDepth (display_, DefaultScreen (display_)),
                    XYPixmap,
                    0,
                    (transform_image ? buffer_
                                     : reinterpret_cast<uint8_t*> (message_inout->rd_ptr ())),
                    resolution_2.width, resolution_2.height,
                    32,
                    row_stride_2);
  if (unlikely (!image_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XCreateImage(0x%@): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_));
    return;
  } // end IF

  XLockDisplay (display_);
  release_lock = true;

  result_2 = XPutImage (display_,
                        pixmap_,
//                        context_,
                        DefaultGC (display_, DefaultScreen (display_)),
                        image_p,
                        0, 0, 0, 0, resolution_2.width, resolution_2.height);
  if (unlikely (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XPutImage(0x%@): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_));
    XDestroyImage (image_p); image_p = NULL;
    goto unlock;
  } // end IF

  result_2 = XSetWindowBackgroundPixmap (display_,
                                         window_,
                                         pixmap_);
  if (unlikely (result_2))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XSetWindowBackgroundPixmap(0x%@,%u,%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_, window_, pixmap_));
    XDestroyImage (image_p); image_p = NULL;
    goto unlock;
  } // end IF
  XDestroyImage (image_p); image_p = NULL;

unlock:
  if (release_lock)
    XUnlockDisplay (display_);
}

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          typename ConfigurationType,
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          typename SessionDataContainerType,
          typename MediaType>
void
Stream_Module_Vis_X11_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::handleSessionMessage (SessionMessageType*& message_inout,
                                                                 bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_X11_Window_T::handleSessionMessage"));

  ACE_UNUSED_ARG (passMessageDownstream_out);

  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // sanity check(s)
      ACE_ASSERT (inherited::sessionData_);
      ACE_ASSERT (!buffer_);
      ACE_ASSERT (display_);
      ACE_ASSERT (window_);
      ACE_ASSERT (!pixmap_);

      Common_UI_Resolution_t resolution_s =
          Stream_MediaFramework_Tools::toResolution (*display_,
                                                     window_);

      ACE_NEW_NORETURN (buffer_,
                        uint8_t [resolution_s.width * resolution_s.height * (DefaultDepth (display_, DefaultScreen (display_)) / 8)]);
      if (!buffer_)
      {
        ACE_DEBUG ((LM_CRITICAL,
                    ACE_TEXT ("%s: failed to allocate memory: \"%m\", aborting\n"),
                    inherited::mod_->name ()));
        goto error;
      } // end IF

      pixmap_ = XCreatePixmap (display_,
                               window_,
                               resolution_s.width,
                               resolution_s.height,
                               DefaultDepth (display_, DefaultScreen (display_)));
      if (unlikely (!pixmap_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to XCreatePixmap(0x%@,%u,%ux%u,%u): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    display_, window_,
                    resolution_s.width, resolution_s.height,
                    DefaultDepth (display_, DefaultScreen (display_))));
        goto error;
      } // end IF

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (display_);

      int result = XFreePixmap (display_, pixmap_);
      if (unlikely (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to XFreePixmap(0x%@,%u): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    display_, pixmap_));
      pixmap_ = 0;

      if (buffer_)
      {
        delete [] buffer_; buffer_ = NULL;
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
          typename SessionDataContainerType,
          typename MediaType>
bool
Stream_Module_Vis_X11_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::initialize (const ConfigurationType& configuration_in,
                                                       Stream_IAllocator* allocator_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_X11_Window_T::initialize"));

  if (inherited::isInitialized_)
  {
    if (closeWindow_)
    { ACE_ASSERT (display_ && window_);
      int result = XUnmapWindow (display_, window_);
      if (unlikely (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to XUnmapWindow(0x%@,%u): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    display_, window_));
    } // end IF
    if (closeDisplay_)
    { ACE_ASSERT (display_);
      int result = XCloseDisplay (display_);
      if (unlikely (result))
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to XCloseDisplay(0x%@): \"%m\", continuing\n"),
                    inherited::mod_->name (),
                    display_));
    } // end IF
    closeDisplay_ = false;
    display_ = NULL;
    isFirst_ = true;
    if (scaleContext_)
    {
      sws_freeContext (scaleContext_); scaleContext_ = NULL;
    } // end IF
    scaleContextHeight_ = 0;
    scaleContextWidth_ = 0;
    window_ = 0;
  } // end IF

  XErrorHandler error_handler_p =
      XSetErrorHandler (libacestream_vis_x11_error_handler_cb);
  ACE_UNUSED_ARG (error_handler_p);

  // *TODO*: remove type inferences
  if (configuration_in.display)
  {
    display_ = configuration_in.display;
  } // end IF
  else
  {
    ACE_TCHAR* display_p =
        ACE_OS::getenv (ACE_TEXT (STREAM_VIS_X11_DISPLAY_ENVIRONMENT_VARIABLE));
    display_ = XOpenDisplay (ACE_TEXT_ALWAYS_CHAR (display_p));
    if (unlikely (!display_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XOpenDisplay(\"%s\"): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  display_p));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: opened X11 connection (display: \"%s\")\n"),
                inherited::mod_->name (),
                display_p));
#endif // _DEBUG
    closeDisplay_ = true;
    XSync (display_, False);
  } // end ELSE
  ACE_ASSERT (display_);
  if (configuration_in.window)
    window_ = configuration_in.window;
  else
  {
    XSetWindowAttributes attributes_a;
    attributes_a.background_pixel  =
        XWhitePixel (display_, DefaultScreen (display_));
    attributes_a.border_pixel =
        XBlackPixel (display_, DefaultScreen (display_));
    attributes_a.override_redirect = 0;
    window_ =
        XCreateWindow (display_,
                       XRootWindow (display_, DefaultScreen (display_)),
                       0, 0,
                       COMMON_UI_WINDOW_DEFAULT_WIDTH, COMMON_UI_WINDOW_DEFAULT_HEIGHT,
                       0,
                       DefaultDepth (display_, DefaultScreen (display_)),
                       InputOutput,
                       DefaultVisual (display_, DefaultScreen (display_)),
                       CWBackPixel | CWBorderPixel | CWOverrideRedirect,
                       &attributes_a);
    if (unlikely (!window_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XCreateWindow(0x%@): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  display_));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: created X11 window (display: 0x%@, id: %u)\n"),
                inherited::mod_->name (),
                display_,
                window_));
#endif // _DEBUG
    closeWindow_ = true;
//    XSelectInput (display_,
//                  window_,
//                  (ExposureMask | ButtonPressMask | KeyPressMask));
    int result = XMapWindow (display_, window_);
    if (unlikely (result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XMapWindow(0x%@,%u): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  display_, window_));
      return false;
    } // end IF

//    unsigned long valuemask_i = (GCFont       |
//                                 GCFunction   |
//                                 GCPlaneMask  |
//                                 GCForeground |
//                                 GCBackground);
//    XGCValues values_s;	/* initial values for the GC.   */
//    XFontStruct* fontinfo_p = XLoadQueryFont (display_, "6x10");
//    ACE_ASSERT (fontinfo_p);
//    values_s.font =   fontinfo_p->fid;
//    values_s.function =   GXcopy;
//    values_s.plane_mask = AllPlanes;
//    values_s.foreground = BlackPixel (display_, DefaultScreen (display_));
//    values_s.background = WhitePixel (display_, DefaultScreen (display_));
//    context_ = XCreateGC (display_,
//                          window_,
//                          valuemask_i,
//                          &values_s);
//    if (unlikely (context_ < 0))
//    {
//      ACE_DEBUG ((LM_ERROR,
//                  ACE_TEXT ("%s: failed to XCreateGC(0x%@,%u,%u): \"%m\", aborting\n"),
//                  inherited::mod_->name (),
//                  display_, window_,
//                  valuemask_i));
//      return false;
//    } // end IF
  } // end ELSE

  return inherited::initialize (configuration_in,
                                allocator_in);
}
