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

#include "ace/Global_Macros.h"
#include "ace/Singleton.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "common_icounter.h"
#include "common_iget.h"
#include "common_inotify.h"

#if defined (FFTW_SUPPORT)
#include "common_math_fftw.h"
#else
#include "common_math_fft.h"
#endif // FFTW_SUPPORT

#include "common_timer_resetcounterhandler.h"

#include "stream_vis_gtk_common.h"
#include "stream_vis_gtk_window.h"

struct Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration
{
  Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_Configuration ()
   : applyWindowFunction (false)
   , mode (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_2DMODE)
   , resolution (STREAM_VIS_SPECTRUMANALYZER_DEFAULT_RESOLUTION)
   , window (NULL)
  {}

  bool                                              applyWindowFunction; // e.g. Hamming-, Hann-, ...
  enum Stream_Visualization_SpectrumAnalyzer_2DMode mode;
  unsigned int                                      resolution;
#if GTK_CHECK_VERSION (4,0,0)
  GdkSurface*                                       window;
#else
  GdkWindow*                                        window;
#endif // GTK_CHECK_VERSION (4,0,0)
};

//////////////////////////////////////////

struct acestream_visualization_gtk_cairo_cbdata
{
  // *TODO*: in gtk4, use a GdkDrawingContext*
  cairo_t*                    context;
  Common_IDispatch*           dispatch;
#if GTK_CHECK_VERSION (4,0,0)
  Common_ISetP_T<GdkSurface>* resizeNotification;
  GdkSurface*                 window;
#else
  Common_ISetP_T<GdkWindow>*  resizeNotification;
  GdkWindow*                  window;
#endif // GTK_CHECK_VERSION (4,0,0)
};

//////////////////////////////////////////

#if GTK_CHECK_VERSION (3,0,0)
gboolean acestream_visualization_gtk_cairo_draw_cb (GtkWidget*, cairo_t*, gpointer);
#else
gboolean acestream_visualization_gtk_cairo_expose_event_cb (GtkWidget*, GdkEvent*, gpointer);
//gboolean acestream_visualization_gtk_cairo_configure_event_cb (GtkWidget*, GdkEvent*, gpointer);
#endif // GTK_CHECK_VERSION (3,0,0)
void acestream_visualization_gtk_cairo_size_allocate_cb (GtkWidget*, GdkRectangle*, gpointer);

gboolean acestream_visualization_gtk_cairo_idle_update_cb (gpointer);

extern const char libacestream_default_vis_spectrum_analyzer_module_name_string[];

//////////////////////////////////////////

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
          typename MediaType,
          typename ValueType> // buffer value-
class Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T
 : public Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType>
#if defined (FFTW_SUPPORT)
 , public Common_Math_FFTW_T<ValueType>
#else
 , public Common_Math_FFT_T<ValueType>
#endif // FFTW_SUPPORT
  //, public Common_ICounter
 , public Common_IDispatch
#if GTK_CHECK_VERSION (4,0,0)
 , public Common_ISetP_T<GdkSurface>
#else
 , public Common_ISetP_T<GdkWindow>
#endif // GTK_CHECK_VERSION (4,0,0)
{
  typedef Stream_Module_Vis_GTK_Window_T<ACE_SYNCH_USE,
                                         TimePolicyType,
                                         ConfigurationType,
                                         ControlMessageType,
                                         DataMessageType,
                                         SessionMessageType,
                                         MediaType> inherited;
#if defined (FFTW_SUPPORT)
  typedef Common_Math_FFTW_T<ValueType> inherited2;
#else
  typedef Common_Math_FFT_T<ValueType> inherited2;
#endif // FFTW_SUPPORT

 public:
  Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_IDispatch
  // *IMPORTANT NOTE*: argument is acestream_visualization_gtk_cairo_cbdata
  virtual void dispatch (void*);

  // implement Common_ISetP_T
#if GTK_CHECK_VERSION (4,0,0)
  virtual void setP (GdkSurface*); // target window
#else
  virtual void setP (GdkWindow*); // target window
#endif // GTK_CHECK_VERSION (4,0,0)

 private:
  // convenient types
  typedef ACE_Singleton<TimerManagerType,
                        ACE_SYNCH_MUTEX> TIMER_MANAGER_SINGLETON_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T (const Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T& operator= (const Stream_Visualization_GTK_Cairo_SpectrumAnalyzer_T&))

  // override ACE_Task_Base members
  virtual int svc (void);

#if GTK_CHECK_VERSION (4,0,0)
  bool initialize_Cairo (GdkSurface*, // target window
#else
  bool initialize_Cairo (GdkWindow*, // target window
#endif // GTK_CHECK_VERSION (4,0,0)
                         cairo_t*&); // return value: cairo context

  unsigned int                                       bufferedSamples_;
  struct acestream_visualization_gtk_cairo_cbdata    CBData_;
  double                                             channelFactor_;
#if GTK_CHECK_VERSION (4,0,0)
  GdkCairoContext*                                   drawingContext_;
#endif // GTK_CHECK_VERSION (4,0,0)
#if GTK_CHECK_VERSION (3,22,0)
  cairo_region_t*                                    cairoRegion_;
#endif // GTK_CHECK_VERSION (3,22,0)
  double                                             scaleFactorX_;
  // *NOTE*: there are only (N/2)-1 meaningful values for real-valued data
  double                                             scaleFactorX_2; // /2 for spectrum
  double                                             scaleFactorY_;
  double                                             scaleFactorY_2; // *2 for spectrum
  int                                                halfHeight_;
  int                                                height_;
  int                                                width_;

#if defined (FFTW_SUPPORT)
  Common_Math_FFTW_SampleIterator_T<ValueType>       sampleIterator_;
#else
  Common_Math_FFT_SampleIterator_T<ValueType>        sampleIterator_;
#endif // FFTW_SUPPORT
};

// include template definition
#include "stream_vis_gtk_cairo_spectrum_analyzer.inl"

#endif
