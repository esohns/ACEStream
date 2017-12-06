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

#ifndef STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUM_ANALYZER_H
#define STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUM_ANALYZER_H

#include <functional>
#include <random>

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfapi.h>
#include <mfobjects.h>
#endif

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <gl/GL.h>
#else
#include "GL/gl.h"
#endif
#include "gtk/gtk.h"
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
#else
#include "gtkgl/gtkglarea.h"
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGLAREA_SUPPORT)
//#include <gtkgl/gdkgl.h>
#include "gtkgl/gtkglarea.h"
#else
#include "gtk/gtkgl.h" // gtkglext
#endif /* GTKGLAREA_SUPPORT */
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif

#include "common_icounter.h"
#include "common_iget.h"
#include "common_inotify.h"
#include "common_time_common.h"

#include "common_math_fft.h"

#include "stream_resetcounterhandler.h"
#include "stream_task_base_synch.h"

#include "stream_stat_common.h"

#include "stream_vis_common.h"
#include "stream_vis_exports.h"

extern Stream_Vis_Export const char libacestream_default_vis_spectrum_analyzer_module_name_string[];

enum Stream_Module_Visualization_SpectrumAnalyzer2DMode
{ // *TODO*: implement discrete modes of operation
  STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_OSCILLOSCOPE = 0,
  STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_SPECTRUM,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_MAX,
  STREAM_MODULE_VIS_SPECTRUMANALYZER_2DMODE_INVALID
};
enum Stream_Module_Visualization_SpectrumAnalyzer3DMode
{
  STREAM_MODULE_VIS_SPECTRUMANALYZER_3DMODE_DEFAULT = 0,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_SPECTRUMANALYZER_3DMODE_MAX,
  STREAM_MODULE_VIS_SPECTRUMANALYZER_3DMODE_INVALID
};

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
          typename TimerManagerType> // implements Common_ITimer
class Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Common_Math_FFT
 , public Common_ICounter
 , public Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType>
#if GTK_CHECK_VERSION (3,10,0)
 , public Common_ISetP_T<cairo_surface_t>
#else
 , public Common_ISetP_T<GdkPixbuf>
#endif
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Common_Math_FFT inherited2;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ();

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

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T (const Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T& operator= (const Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T&))

  virtual int svc (void);

#if GTK_CHECK_VERSION (3,10,0)
  bool initialize_Cairo (GdkWindow*,
                         cairo_t*&,
                         cairo_surface_t*&);
#else
  bool initialize_Cairo (GdkWindow*,
                         cairo_t*&,
                         GdkPixbuf*&);
#endif
  virtual void dispatch (const enum Stream_Statistic_AnalysisEventType&);
  // implement Common_ICounter (triggers frame rendering)
  virtual void reset ();
#if GTK_CHECK_VERSION (3,10,0)
  virtual void setP (cairo_surface_t*);
#else
  virtual void setP (GdkPixbuf*);
#endif

  void update ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: callers must free the return value !
  template <typename FormatType> AM_MEDIA_TYPE* getFormat (const FormatType format_in) { return getFormat_impl (format_in); }
  AM_MEDIA_TYPE* getFormat_impl (const struct _AMMediaType*); // return value: media type handle
  AM_MEDIA_TYPE* getFormat_impl (const IMFMediaType*); // return value: media type handle
#endif

  cairo_t*                                                 cairoContext_;
  ACE_SYNCH_MUTEX_T*                                       surfaceLock_;
#if GTK_CHECK_VERSION (3,10,0)
  cairo_surface_t*                                         cairoSurface_;
#else
  GdkPixbuf*                                               pixelBuffer_;
#endif
#if defined (GTKGL_SUPPORT)
  Stream_Module_Visualization_OpenGLInstructions_t*        OpenGLInstructions_;
  ACE_SYNCH_MUTEX*                                         OpenGLInstructionsLock_;
  //GLuint                                                   OpenGLTextureId_;
#if GTK_CHECK_VERSION (3,0,0)
  GdkRGBA                                                  backgroundColor_;
  GdkRGBA                                                  foregroundColor_;
#else /* GTK_CHECK_VERSION (3,0,0) */
  GdkColor                                                 backgroundColor_;
  GdkColor                                                 foregroundColor_;
#endif
#if GTK_CHECK_VERSION (3,0,0)
#if GTK_CHECK_VERSION (3,16,0)
  GtkGLArea*                                               OpenGLWindow_;
#else
  GglaArea*                                                OpenGLWindow_;
#endif /* GTK_CHECK_VERSION (3,16,0) */
#else /* GTK_CHECK_VERSION (3,0,0) */
#if defined (GTKGLAREA_SUPPORT)
//  GglaArea*                                                OpenGLWindow_;
  GtkGLArea*                                               OpenGLWindow_;
#else
  GdkGLContext*                                            OpenGLContext_;
  GdkGLDrawable*                                           OpenGLWindow_;
#endif
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */
  double                                                   channelFactor_;
  double                                                   scaleFactorX_;
  double                                                   scaleFactorY_;
  int                                                      height_;
  int                                                      width_;

  enum Stream_Module_Visualization_SpectrumAnalyzer2DMode* mode2D_;
#if defined (GTKGL_SUPPORT)
  enum Stream_Module_Visualization_SpectrumAnalyzer3DMode* mode3D_;
#endif

  Stream_ResetCounterHandler                               renderHandler_;
  long                                                     renderHandlerTimerId_;

  Common_Math_FFT_SampleIterator                           sampleIterator_;

  // random number generator
  std::uniform_int_distribution<int>                       randomDistribution_;
  std::default_random_engine                               randomEngine_;
  std::function<int ()>                                    randomGenerator_;
};

// include template definition
#include "stream_vis_gtk_cairo_spectrum_analyzer.inl"

#endif
