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
}
#endif /* __cplusplus */

#include "X11/Xutil.h"

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "common_image_tools.h"

#include "common_ui_defines.h"
#include "common_ui_tools.h"

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
// , buffer_ (NULL)
 , closeDisplay_ (false)
 , closeWindow_ (false)
// , context_ ()
 , display_ (NULL)
 , isFirst_ (true)
 , pixmap_ (0)
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

//  if (buffer_)
//    delete [] buffer_;
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
    if (unlikely (!result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XFreePixmap(0x%@,%u): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_, pixmap_));
  } // end IF
  if (closeWindow_)
  { ACE_ASSERT (display_ && window_);
    result = XDestroyWindow (display_, window_);
    if (unlikely (!result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XDestroyWindow(0x%@,%u): \"%m\", continuing\n"),
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
  ACE_ASSERT (display_);
  ACE_ASSERT (window_);
  ACE_ASSERT (pixmap_);

  // sanity check(s)
//  const typename SessionDataContainerType::DATA_T& session_data_r =
//      inherited::sessionData_->getR ();
//  ACE_ASSERT (!session_data_r.formats.empty ());
//  const MediaType& media_type_r = session_data_r.formats.front ();
//  struct Stream_MediaFramework_FFMPEG_MediaType media_type_2;
//  inherited2::getMediaType (media_type_r,
//                            media_type_2);
  Common_Image_Resolution_t resolution_s =
      Stream_MediaFramework_Tools::toResolution (*display_,
                                                 window_);
//  int row_size_i =
//      av_image_get_linesize (media_type_2.format,
//                             media_type_2.resolution.width,
//                             0);
//  ACE_ASSERT (media_type_2.format == AV_PIX_FMT_RGB32);
//  ACE_ASSERT ((media_type_2.resolution.width == resolution_s.width) && (media_type_2.resolution.height == resolution_s.height));

  //  bool release_lock = false;
    bool refresh_b = true;
    Status result_2 = -1;
    XImage* image_p = NULL;

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

  image_p =
      XCreateImage (display_,
                    DefaultVisual (display_, DefaultScreen (display_)),
//                    32,
                    DefaultDepth (display_, DefaultScreen (display_)),
                    ZPixmap,
                    0,
                    reinterpret_cast<char*> (message_inout->rd_ptr ()),
                    resolution_s.width, resolution_s.height,
                    32,
                    0);
//                    row_size_i);
  if (unlikely (!image_p))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XCreateImage(%@): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_));
    return;
  } // end IF

//  XLockDisplay (display_);
//  release_lock = true;

//#if defined (_DEBUG)
//  Common_UI_Tools::dump (*display_, window_);
//  Common_UI_Tools::dump (*display_, pixmap_);
//#endif // _DEBUG

  // *TODO*: currently XPutImage returns 0 in all cases...no error handling :-(
  result_2 = XPutImage (display_,
                        pixmap_,
                        DefaultGC (display_, DefaultScreen (display_)),
                        image_p,
                        0, 0, 0, 0,
                        resolution_s.width, resolution_s.height);
//  if (unlikely (result_2 != True))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to XPutImage(0x%@): \"%s\", returning\n"),
//                inherited::mod_->name (),
//                display_,
//                ACE_TEXT (Stream_MediaFramework_Tools::toString (*display_, result_2).c_str ())));
//    image_p->data = NULL; XDestroyImage (image_p); image_p = NULL;
//    refresh_b = false;
//    goto unlock;
//  } // end IF

  result_2 = XSetWindowBackgroundPixmap (display_,
                                         window_,
                                         pixmap_);
  if (unlikely (result_2 != True))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XSetWindowBackgroundPixmap(0x%@,%u,%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_, window_, pixmap_));
    image_p->data = NULL; XDestroyImage (image_p); image_p = NULL;
    refresh_b = false;
    goto unlock;
  } // end IF
  image_p->data = NULL; XDestroyImage (image_p); image_p = NULL;

unlock:
//  if (release_lock)
//    XUnlockDisplay (display_);

  if (unlikely (!refresh_b))
    return;

  // repaint window immediately
  result_2 = XClearWindow (display_,
                           window_);
  if (unlikely (result_2 != True))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XClearWindow(0x%@,%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_, window_));
    return;
  } // end IF
  result_2 = XFlush (display_);
  if (unlikely (result_2 != True))
  {
    ACE_DEBUG ((LM_ERROR,
                ACE_TEXT ("%s: failed to XFlush(0x%@,%u): \"%m\", returning\n"),
                inherited::mod_->name (),
                display_, window_));
    return;
  } // end IF
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
      ACE_ASSERT (display_);
      ACE_ASSERT (window_);
      const typename SessionDataContainerType::DATA_T& session_data_r =
          inherited::sessionData_->getR ();
//      ACE_ASSERT (!session_data_r.formats.empty ());
//      const MediaType& media_type_r = session_data_r.formats.front ();
//      struct Stream_MediaFramework_FFMPEG_MediaType media_type_2;
//      inherited2::getMediaType (media_type_r,
//                                media_type_2);
      Common_Image_Resolution_t resolution_s =
          Stream_MediaFramework_Tools::toResolution (*display_,
                                                     window_);
//      int row_size_i =
//          av_image_get_linesize (media_type_2.format,
//                                 media_type_2.resolution.width,
//                                 0);
//      int depth_i =
//          (row_size_i / media_type_2.resolution.width) * 8;
      XWindowAttributes attributes_s = Common_UI_Tools::get (*display_,
                                                             window_);
      // *NOTE*: otherwise there will be 'BadMatch' errors
//      ACE_ASSERT ((media_type_2.resolution.width == resolution_s.width) && (media_type_2.resolution.height == resolution_s.height));
//      ACE_ASSERT (depth_i == attributes_s.depth);

      // sanity check(s)
      ACE_ASSERT (!pixmap_);

      pixmap_ = XCreatePixmap (display_,
                               window_,
                               resolution_s.width, resolution_s.height,
                               attributes_s.depth);
      if (unlikely (!pixmap_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to XCreatePixmap(0x%@,%u,%ux%u,%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    display_, window_,
                    resolution_s.width, resolution_s.height,
                    attributes_s.depth));
        goto error;
      } // end IF
#if defined (_DEBUG)
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: allocated %ux%u pixmap (depth: %d bits)\n"),
                  inherited::mod_->name (),
                  resolution_s.width, resolution_s.height,
                  attributes_s.depth));
#endif // _DEBUG

      break;

error:
      this->notify (STREAM_SESSION_MESSAGE_ABORT);

      break;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      // sanity check(s)
      ACE_ASSERT (display_);

      int result = -1;
      if (pixmap_)
      {
        result = XFreePixmap (display_, pixmap_);
        if (unlikely (!result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to XFreePixmap(0x%@,%u): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      display_, pixmap_));
        pixmap_ = None;
      } // end IF
      if (closeWindow_)
      { ACE_ASSERT (display_ && window_);
        result = XDestroyWindow (display_, window_);
        if (unlikely (!result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to XDestroyWindow(0x%@,%u): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      display_, window_));
        window_ = None;
        closeWindow_ = false;
      } // end IF
      if (closeDisplay_)
      { ACE_ASSERT (display_);
        result = XCloseDisplay (display_);
        if (unlikely (result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to XCloseDisplay(0x%@): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      display_));
        display_ = NULL;
        closeDisplay_ = false;
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
void
Stream_Module_Vis_X11_Window_T<ACE_SYNCH_USE,
                               TimePolicyType,
                               ConfigurationType,
                               ControlMessageType,
                               DataMessageType,
                               SessionMessageType,
                               SessionDataContainerType,
                               MediaType>::toggle ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_Vis_X11_Window_T::toggle"));

  ACE_ASSERT (false); // *TODO*
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
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
    window_ = 0;
  } // end IF

#if defined (_DEBUG)
  XErrorHandler error_handler_p =
      XSetErrorHandler (libacestream_vis_x11_error_handler_cb);
  ACE_UNUSED_ARG (error_handler_p);
  XIOErrorHandler io_error_handler_p =
      XSetIOErrorHandler (libacestream_vis_x11_io_error_handler_cb);
  ACE_UNUSED_ARG (io_error_handler_p);
#endif // _DEBUG

  ACE_ASSERT (!display_);
  // *TODO*: remove type inferences
  if (configuration_in.X11Display)
  {
    display_ = configuration_in.X11Display;
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@, default depth: %d)\n"),
                inherited::mod_->name (),
                display_, DefaultDepth (display_, DefaultScreen (display_))));
#endif // _DEBUG
  } // end IF
  else
  {
    std::string x11_display_name =
        Common_UI_Tools::getX11DisplayName (configuration_in.display.device);
    if (unlikely (x11_display_name.empty ()))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to Common_UI_Tools::getX11DisplayName(\"%s\"), aborting\n"),
                  inherited::mod_->name (),
                  ACE_TEXT (configuration_in.display.device.c_str ())));
    const char* display_name_p =
        (x11_display_name.empty () ? NULL
                                   : x11_display_name.c_str ());
    display_ = XOpenDisplay (display_name_p);
    if (unlikely (!display_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XOpenDisplay(\"%s\"): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  display_name_p));
      return false;
    } // end IF
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: opened X11 connection to \"%s\" (display: %@, default depth: %d)\n"),
                inherited::mod_->name (),
                display_name_p, display_,
                DefaultDepth (display_, DefaultScreen (display_))));
#endif // _DEBUG
    closeDisplay_ = true;
  } // end ELSE
  ACE_ASSERT (display_);
#if defined (_DEBUG)
  XSync (display_, True);
#endif // _DEBUG

  if (configuration_in.window)
  {
    window_ = configuration_in.window;
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@, window: %u)\n"),
                inherited::mod_->name (),
                display_, window_));
#endif // _DEBUG
  } // end IF
  else
  {
    int x =
        (configuration_in.fullScreen ? configuration_in.display.clippingArea.x
                                     : 0);
    int y =
        (configuration_in.fullScreen ? configuration_in.display.clippingArea.y
                                     : 0);
    unsigned int width_i =
        (configuration_in.fullScreen ? configuration_in.display.clippingArea.width
                                     : configuration_in.outputFormat.resolution.width);
    unsigned int height_i =
        (configuration_in.fullScreen ? configuration_in.display.clippingArea.height
                                     : configuration_in.outputFormat.resolution.height);
    unsigned long valuemask_i =
        (CWBackPixel       |
         CWBorderPixel     |
         CWOverrideRedirect);
    XSetWindowAttributes attributes_a;
    ACE_OS::memset (&attributes_a, 0, sizeof (XSetWindowAttributes));
    attributes_a.background_pixel  =
        XBlackPixel (display_, DefaultScreen (display_));
    attributes_a.border_pixel =
        XBlackPixel (display_, DefaultScreen (display_));
    attributes_a.override_redirect = True;
    window_ =
        XCreateWindow (display_,
                       DefaultRootWindow (display_),
                       x, y,
                       width_i, height_i,
                       0,
                       DefaultDepth (display_, DefaultScreen (display_)),
//                       32,
                       InputOutput,
                       DefaultVisual (display_, DefaultScreen (display_)),
                       valuemask_i,
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
                ACE_TEXT ("%s: created X11 window (display: %@, id: %u)\n"),
                inherited::mod_->name (),
                display_,
                window_));
#endif // _DEBUG
    closeWindow_ = true;
//    XSelectInput (display_,
//                  window_,
//                  (ExposureMask | ButtonPressMask | KeyPressMask));
    int result = XMapRaised (display_, window_);
    if (unlikely (!result))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XMapRaised(%@,%u): \"%m\", aborting\n"),
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
#if defined (_DEBUG)
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: display %@ (window: %d, depth: %d)\n"),
                inherited::mod_->name (),
                display_, window_,
//                32));
                DefaultDepth (display_, DefaultScreen (display_))));
#endif // _DEBUG

  return inherited::initialize (configuration_in,
                                allocator_in);
}
