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

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"
#if GTK_CHECK_VERSION(3,16,0)
#else
#if defined (GTKGL_SUPPORT)
#if defined (GTKGLAREA_SUPPORT)
#include "gtkgl/gtkglarea.h"
#endif // GTKGLAREA_SUPPORT
#endif // GTKGL_SUPPORT
#endif // GTK_CHECK_VERSION (3,16,0)

#include "common_icounter.h"
#include "common_iget.h"
#include "common_inotify.h"

#include "common_math_fft.h"

#include "common_timer_resetcounterhandler.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_stat_common.h"

#include "stream_vis_gtk_common.h"

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
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
 , public Stream_MediaFramework_MediaTypeConverter_T<MediaType>
 , public Common_Math_FFT
 , public Common_ICounter
 , public Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType>
 , public Common_ISetP_T<GdkWindow>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
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

  // override ACE_Task_Base members
  virtual int svc (void);
  // override Stream_TaskBaseSynch_T members
  virtual void stop (bool = true,   // wait for completion ?
                     bool = false); // high priority ? (i.e. do not wait for queued messages)

  bool initialize_Cairo (GdkWindow*,
                         cairo_t*&);
  virtual void dispatch (const enum Stream_Statistic_AnalysisEventType&);
  // implement Common_ICounter (triggers frame rendering)
  virtual void reset ();
  virtual void setP (GdkWindow*);

  void update ();

  cairo_t*                                           cairoContext_;
#if defined (GTKGL_SUPPORT)
#if GTK_CHECK_VERSION(3,0,0)
  GdkRGBA                                            backgroundColor_;
  GdkRGBA                                            foregroundColor_;
#else
  GdkColor                                           backgroundColor_;
  GdkColor                                           foregroundColor_;
#endif /* GTK_CHECK_VERSION (3,0,0) */
#endif /* GTKGL_SUPPORT */
  double                                             channelFactor_;
  double                                             scaleFactorX_;
  // *NOTE*: there are only (N/2)-1 meaningful values for real-valued data
  double                                             scaleFactorX_2; // for spectrum
  double                                             scaleFactorY_;
  int                                                halfHeight_;
  int                                                height_;
  int                                                width_;

  enum Stream_Visualization_SpectrumAnalyzer_2DMode* mode2D_;
//#if defined (GTKGL_SUPPORT)
//  enum Stream_Visualization_SpectrumAnalyzer_3DMode* mode3D_;
//#endif // GTKGL_SUPPORT
  typename inherited::MESSAGE_QUEUE_T                queue_;

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
