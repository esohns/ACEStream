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

#ifndef STREAM_VISUALIZATION_GTK_CAIRO_SPECTRUM_ANALYZER_H
#define STREAM_VISUALIZATION_GTK_CAIRO_SPECTRUM_ANALYZER_H

#include <functional>
#include <random>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfapi.h>
#include <mfobjects.h>
#endif // ACE_WIN32 || ACE_WIN64

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <gl/GL.h>
#else
#include "GL/gl.h"
#endif // ACE_WIN32 || ACE_WIN64
#include "gtk/gtk.h"
//#if defined (GTKGL_SUPPORT)
//#if GTK_CHECK_VERSION(3,0,0)
//#if GTK_CHECK_VERSION(3,16,0)
//#else
//#if defined (GTKGLAREA_SUPPORT)
//#include "gtkgl/gtkglarea.h"
//#endif /* GTKGLAREA_SUPPORT */
//#endif /* GTK_CHECK_VERSION (3,16,0) */
//#elif GTK_CHECK_VERSION(2,0,0)
//#if defined (GTKGLAREA_SUPPORT)
//#include "gtkgl/gdkgl.h"
//#endif /* GTKGLAREA_SUPPORT */
//#else
//#include "gtk/gtkgl.h" // gtkglext
//#endif /* GTK_CHECK_VERSION (x,0,0) */
//#endif // GTKGL_SUPPORT

#include "common_icounter.h"
#include "common_iget.h"
#include "common_ilock.h"
#include "common_inotify.h"
#include "common_time_common.h"

#include "common_math_fft.h"

#include "common_timer_resetcounterhandler.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_stat_common.h"

#include "stream_vis_common.h"

extern const char libacestream_default_vis_spectrum_analyzer_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename TimerManagerType, // implements Common_ITimer
          ////////////////////////////////
          typename MediaType>
class Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    >
#else
                                                    ,SessionDataType>
#endif // ACE_WIN32 || ACE_WIN64
 , public Common_Math_FFT
 , public Common_ICounter
 , public Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType>
#if GTK_CHECK_VERSION(3,10,0)
 , public Common_ISetP_T<cairo_surface_t>
#else
 , public Common_ISetP_T<GdkPixbuf>
#endif // GTK_CHECK_VERSION (3,10,0)
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 Common_ILock_T<ACE_SYNCH_USE>,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType
#if defined (ACE_WIN32) || defined (ACE_WIN64)
                                                    > inherited2;
#else
                                                     ,SessionDataType> inherited2;
#endif // ACE_WIN32 || ACE_WIN64
  typedef Common_Math_FFT inherited3;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (typename inherited::ISTREAM_T*); // stream handle
#endif // ACE_WIN32 || ACE_WIN64
  virtual ~Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef ACE_Singleton<TimerManagerType,
                        ACE_SYNCH_MUTEX> TIMER_MANAGER_SINGLETON_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (const Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T& operator= (const Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T&))

  virtual int svc (void);

  bool initialize_Cairo (GdkWindow*,
                         cairo_t*&,
#if GTK_CHECK_VERSION(3,10,0)
                         cairo_surface_t*&);
#else
                         GdkPixbuf*&);
#endif // GTK_CHECK_VERSION (3,10,0)
  virtual void dispatch (const enum Stream_Statistic_AnalysisEventType&);
  // implement Common_ICounter (triggers frame rendering)
  virtual void reset ();
#if GTK_CHECK_VERSION (3,10,0)
  virtual void setP (cairo_surface_t*);
#else
  virtual void setP (GdkPixbuf*);
#endif // GTK_CHECK_VERSION (3,10,0)

  void update ();

  cairo_t*                                           cairoContext_;
  ACE_SYNCH_MUTEX_T*                                 surfaceLock_;
#if GTK_CHECK_VERSION (3,10,0)
  cairo_surface_t*                                   cairoSurface_;
#else
  GdkPixbuf*                                         pixelBuffer_;
#endif
#if defined (GTKGL_SUPPORT)
  Stream_Visualization_OpenGL_Instructions_t*        OpenGLInstructions_;
  ACE_SYNCH_MUTEX*                                   OpenGLInstructionsLock_;
  //GLuint                                             OpenGLTextureId_;
#if GTK_CHECK_VERSION(3,0,0)
  GdkRGBA                                            backgroundColor_;
  GdkRGBA                                            foregroundColor_;
#else
  GdkColor                                           backgroundColor_;
  GdkColor                                           foregroundColor_;
#endif /* GTK_CHECK_VERSION (3,0,0) */
//#if GTK_CHECK_VERSION(3,0,0)
//#if GTK_CHECK_VERSION(3,16,0)
//  GtkGLArea*                                         OpenGLWindow_;
//#else
//#if defined (GTKGLAREA_SUPPORT)
//  GglaArea*                                          OpenGLWindow_;
//#else
//  GdkWindow*                                         OpenGLWindow_;
//#endif // GTKGLAREA_SUPPORT
//#endif /* GTK_CHECK_VERSION (3,16,0) */
//#else /* GTK_CHECK_VERSION (3,0,0) */
//#if defined (GTKGLAREA_SUPPORT)
//  GtkGLArea*                                         OpenGLWindow_;
//#else
//  GdkGLContext*                                      OpenGLContext_;
//  GdkGLDrawable*                                     OpenGLWindow_;
//#endif // GTKGLAREA_SUPPORT
//#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */
  double                                             channelFactor_;
  double                                             scaleFactorX_;
  double                                             scaleFactorY_;
  int                                                height_;
  int                                                width_;

  enum Stream_Visualization_SpectrumAnalyzer_2DMode* mode2D_;
#if defined (GTKGL_SUPPORT)
  enum Stream_Visualization_SpectrumAnalyzer_3DMode* mode3D_;
#endif // GTKGL_SUPPORT

  Common_Timer_ResetCounterHandler                   renderHandler_;
  long                                               renderHandlerTimerId_;

  Common_Math_FFT_SampleIterator                     sampleIterator_;

  // random number generator
  std::uniform_int_distribution<int>                 randomDistribution_;
  std::default_random_engine                         randomEngine_;
  std::function<int ()>                              randomGenerator_;
};

// include template definition
#include "stream_vis_gtk_cairo_spectrum_analyzer.inl"

#endif
