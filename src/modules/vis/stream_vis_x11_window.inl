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

#include "X11/Xlib.h"
#include "X11/Xutil.h"

#include "ace/Log_Msg.h"

#include "common_ui_defines.h"
#include "common_ui_x11_tools.h"

#include "stream_macros.h"

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
 , inherited2 ()
 , inherited3 ()
 , closeDisplay_ (false)
 , closeWindow_ (false)
 , context_ (NULL)
 , depth_ (0)
 , display_ (NULL)
 , pixmap_ (0)
 , resolution_ ()
 , visual_ (NULL)
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

  if (context_)
  { ACE_ASSERT (display_);
    result = XFreeGC (display_,
                      context_);
    if (unlikely (!result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XFreeGC(%@,%@): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_, context_));
  } // end IF
  if (pixmap_)
  { ACE_ASSERT (display_);
    result = XFreePixmap (display_,
                          pixmap_);
    if (unlikely (!result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XFreePixmap(%@,%u): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_, pixmap_));
  } // end IF
  if (closeWindow_)
  { ACE_ASSERT (display_ && window_);
    result = XDestroyWindow (display_, window_);
    if (unlikely (!result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XDestroyWindow(%@,%u): \"%m\", continuing\n"),
                  inherited::mod_->name (),
                  display_, window_));
  } // end IF
  if (closeDisplay_)
  { ACE_ASSERT (display_);
    result = XCloseDisplay (display_);
    if (unlikely (result))
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XCloseDisplay(%@): \"%m\", continuing\n"),
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
    depth_ = 0;
    display_ = NULL;
    window_ = 0;
  } // end IF

  XErrorHandler error_handler_p =
    XSetErrorHandler (libacestream_vis_x11_error_handler_cb);
  ACE_UNUSED_ARG (error_handler_p);
  XIOErrorHandler io_error_handler_p =
    XSetIOErrorHandler (libacestream_vis_x11_io_error_handler_cb);
  ACE_UNUSED_ARG (io_error_handler_p);

  ACE_ASSERT (!display_);
  // *TODO*: remove type inferences
//  if (configuration_in.display.display)
//  {
//    display_ = configuration_in.display.display;
//    ACE_DEBUG ((LM_DEBUG,
//                ACE_TEXT ("%s: passive mode (display: %@, default depth: %d)\n"),
//                inherited::mod_->name (),
//                display_, DefaultDepth (display_, DefaultScreen (display_))));
//  } // end IF
//  else
  {
    std::string x11_display_name =
      Common_UI_X11_Tools::getX11DisplayName (configuration_in.display.device);
    if (unlikely (x11_display_name.empty ()))
      ACE_DEBUG ((LM_WARNING,
                  ACE_TEXT ("%s: failed to Common_UI_X11_Tools::getX11DisplayName(\"%s\"): using default, continuing\n"),
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
                  ACE_TEXT (display_name_p)));
      return false;
    } // end IF
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: opened X11 connection to \"%s\" (display: %@, default depth: %d)\n"),
                inherited::mod_->name (),
                ACE_TEXT (display_name_p), display_,
                DefaultDepth (display_, DefaultScreen (display_))));
    closeDisplay_ = true;
  } // end ELSE
  ACE_ASSERT (display_);
#if defined (_DEBUG)
  if (configuration_in.debug)
    XSync (display_, True);
#endif // _DEBUG

  int count_i = 0;
  int* depths_p = XListDepths (display_, DefaultScreen (display_),
                               &count_i);
  for (unsigned int i = 0;
       i < static_cast<unsigned int> (count_i);
       ++i)
  {
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: available depth #%u: %d\n"),
                inherited::mod_->name (), i,
                depths_p[i]));
  } // end IF

  XVisualInfo visual_info_s;
  if (configuration_in.window)
  {
    inherited3::getWindowType (configuration_in.window,
                               window_);
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: passive mode (display: %@, window: %u)\n"),
                inherited::mod_->name (),
                display_, window_));
    XWindowAttributes attributes_s = Common_UI_X11_Tools::get (*display_,
                                                               window_);
    depth_ = attributes_s.depth;

    if (!XMatchVisualInfo (display_, DefaultScreen (display_),
                           depth_, TrueColor,
                           &visual_info_s))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XMatchVisualInfo(%d): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  depth_));
      return false;
    } // end IF
  } // end IF
  else
  {
    struct Stream_MediaFramework_V4L_MediaType media_type_s;
    inherited2::getMediaType (configuration_in.outputFormat,
                              STREAM_MEDIATYPE_VIDEO,
                              media_type_s);
    unsigned int width_i =
      (configuration_in.fullScreen ? WidthOfScreen (DefaultScreenOfDisplay (display_))
                                   : media_type_s.format.width);
    unsigned int height_i =
      (configuration_in.fullScreen ? HeightOfScreen (DefaultScreenOfDisplay (display_))
                                   : media_type_s.format.height);
    int x =
      (WidthOfScreen (DefaultScreenOfDisplay (display_)) - width_i) / 2;
    int y =
      (HeightOfScreen (DefaultScreenOfDisplay (display_)) - height_i) / 2;
    depth_ = 32; // *TODO*: 24 doesn't work
      // static_cast<int> (Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_s.format.pixelformat));

    if (!XMatchVisualInfo (display_, DefaultScreen (display_),
                           depth_, TrueColor,
                           &visual_info_s))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XMatchVisualInfo(%d): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  depth_));
      return false;
    } // end IF

    unsigned long valuemask_i =
      (CWBackPixel       |
       CWBorderPixel     |
       CWColormap        |
       CWOverrideRedirect);
    XSetWindowAttributes attributes_a;
    ACE_OS::memset (&attributes_a, 0, sizeof (XSetWindowAttributes));
    attributes_a.background_pixel  =
      BlackPixel (display_, DefaultScreen (display_));
    attributes_a.border_pixel =
      BlackPixel (display_, DefaultScreen (display_));
    attributes_a.colormap =
      XCreateColormap (display_, XDefaultRootWindow (display_),
                       visual_info_s.visual, AllocNone);
    attributes_a.override_redirect = True;
    window_ =
      XCreateWindow (display_,
                     DefaultRootWindow (display_),
                     x, y,
                     width_i, height_i,
                     0,
                     depth_,
                     InputOutput,
                     visual_info_s.visual,
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
    ACE_DEBUG ((LM_DEBUG,
                ACE_TEXT ("%s: created X11 window (display: %@, id: %u, width: %u, height: %u, depth: %d)\n"),
                inherited::mod_->name (),
                display_,
                window_,
                width_i, height_i, depth_));
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

    valuemask_i = (//GCFont       |
      GCFunction   |
      GCPlaneMask  |
      GCForeground |
      GCBackground);
    XGCValues values_s; /* initial values for the GC */
    //    XFontStruct* fontinfo_p = XLoadQueryFont (display_, "6x10");
    //    ACE_ASSERT (fontinfo_p);
    //    values_s.font =   fontinfo_p->fid;
    values_s.function =   GXcopy;
    values_s.plane_mask = AllPlanes;
    values_s.foreground = BlackPixel (display_, DefaultScreen (display_));
    values_s.background = WhitePixel (display_, DefaultScreen (display_));
    context_ = XCreateGC (display_,
                          window_,
                          valuemask_i,
                          &values_s);
    if (unlikely (!context_))
    {
      ACE_DEBUG ((LM_ERROR,
                  ACE_TEXT ("%s: failed to XCreateGC(0x%@,%u,%u): \"%m\", aborting\n"),
                  inherited::mod_->name (),
                  display_, window_,
                  valuemask_i));
      return false;
    } // end IF
  } // end ELSE
  visual_ = visual_info_s.visual;
  ACE_DEBUG ((LM_DEBUG,
              ACE_TEXT ("%s: display %@ (window: %d, depth: %d, visual: %@)\n"),
              inherited::mod_->name (),
              display_, window_,
              depth_, visual_));

  return inherited::initialize (configuration_in,
                                allocator_in);
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
  ACE_ASSERT (pixmap_);
  ACE_ASSERT (context_);
  ACE_ASSERT (visual_);
  ACE_ASSERT (window_);

  //  bool release_lock = false;
  bool refresh_b = true;
  Status result_2 = -1;
  XImage* image_p = NULL;

  // XWindowAttributes attributes_s = Common_UI_Tools::get (*display_,
  //                                                        window_);

  image_p =
      XCreateImage (display_,
                    visual_,
                    static_cast<unsigned int> (depth_),
                    ZPixmap,
                    0,
                    reinterpret_cast<char*> (message_inout->rd_ptr ()),
                    resolution_.width, resolution_.height,
                    32, // *TODO*: can this be attributes_s.depth ? no :-(
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

#if defined (_DEBUG)
//  Common_UI_Tools::dump (*display_, window_);
//  Common_UI_Tools::dump (*display_, pixmap_);
#endif // _DEBUG

  // *TODO*: currently XPutImage returns 0 in all cases...no error handling :-(
  result_2 = XPutImage (display_,
                        pixmap_,
                        context_,
                        image_p,
                        0, 0, 0, 0,
                        resolution_.width, resolution_.height);
//  if (unlikely (result_2 != True))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to XPutImage(%@): \"%s\", returning\n"),
//                inherited::mod_->name (),
//                display_,
//                ACE_TEXT (Common_UI_Tools::toString (*display_, result_2).c_str ())));
//    image_p->data = NULL; XDestroyImage (image_p); image_p = NULL;
//    refresh_b = false;
//    goto unlock;
//  } // end IF

  result_2 = XCopyArea (display_,
                        pixmap_, window_,
                        context_,
                        0, 0, resolution_.width, resolution_.height,
                        0, 0);
//  result_2 = XSetWindowBackgroundPixmap (display_,
//                                         window_,
//                                         pixmap_);
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
//  result_2 = XClearWindow (display_,
//                           window_);
//  if (unlikely (result_2 != True))
//  {
//    ACE_DEBUG ((LM_ERROR,
//                ACE_TEXT ("%s: failed to XClearWindow(0x%@,%u): \"%m\", returning\n"),
//                inherited::mod_->name (),
//                display_, window_));
//    return;
//  } // end IF
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
      ACE_ASSERT (!session_data_r.formats.empty ());
      const MediaType& media_type_r = session_data_r.formats.back ();
      struct Stream_MediaFramework_V4L_MediaType media_type_2;
      inherited2::getMediaType (media_type_r,
                                STREAM_MEDIATYPE_VIDEO,
                                media_type_2);
      resolution_.width = media_type_2.format.width;
      resolution_.height = media_type_2.format.height;

      // sanity check(s)
      unsigned int depth_i =
          Stream_MediaFramework_Tools::v4lFormatToBitDepth (media_type_2.format.pixelformat);
      XWindowAttributes attributes_s = Common_UI_X11_Tools::get (*display_,
                                                                  window_);
      // *NOTE*: otherwise there will be 'BadMatch' errors
      ACE_ASSERT (depth_i == static_cast<unsigned int> (attributes_s.depth));
      // *NOTE*: make sure the data fits inside the window
      Common_UI_Resolution_t resolution_s =
          Common_UI_X11_Tools::toResolution (*display_,
                                             window_);
      ACE_ASSERT (resolution_s.width >= resolution_.width && resolution_s.height >= resolution_.height);
      ACE_ASSERT (!pixmap_);

      pixmap_ = XCreatePixmap (display_,
                               window_,
                               resolution_.width, resolution_.height,
                               attributes_s.depth);
      if (unlikely (!pixmap_))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to XCreatePixmap(0x%@,%u,%ux%u,%d): \"%m\", aborting\n"),
                    inherited::mod_->name (),
                    display_, window_,
                    resolution_.width, resolution_.height,
                    attributes_s.depth));
        goto error;
      } // end IF
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: allocated %ux%u pixmap (depth: %d bits)\n"),
                  inherited::mod_->name (),
                  resolution_.width, resolution_.height,
                  attributes_s.depth));

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
      if (context_)
      {
        result = XFreeGC (display_,
                          context_);
        if (unlikely (!result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to XFreeGC(%@,%@): \"%m\", continuing\n"),
                      inherited::mod_->name (),
                      display_, context_));
        context_ = NULL;
      } // end IF
      if (pixmap_)
      {
        result = XFreePixmap (display_,
                              pixmap_);
        if (unlikely (!result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to XFreePixmap(%@,%u): \"%m\", aborting\n"),
                      inherited::mod_->name (),
                      display_, pixmap_));
        pixmap_ = None;
      } // end IF
      if (closeWindow_)
      { ACE_ASSERT (display_ && window_);
        result = XDestroyWindow (display_,
                                 window_);
        if (unlikely (!result))
          ACE_DEBUG ((LM_ERROR,
                      ACE_TEXT ("%s: failed to XDestroyWindow(%@,%u): \"%m\", continuing\n"),
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
                      ACE_TEXT ("%s: failed to XCloseDisplay(%@): \"%m\", continuing\n"),
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
