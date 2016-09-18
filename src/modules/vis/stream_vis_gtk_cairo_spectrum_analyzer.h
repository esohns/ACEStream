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

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include "mfapi.h"
#include "mfobjects.h"
#endif

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "gtk/gtk.h"

#include "common_icounter.h"
#include "common_time_common.h"

#include "common_math_fft.h"

#include "stream_resetcounterhandler.h"
#include "stream_task_base_synch.h"

enum Stream_Module_Visualization_GTKCairoSpectrumAnalyzerMode
{
  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_OSCILLOSCOPE = 0,
  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_SPECTRUMANALYZER,
  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_FREQUENCYANALYZER,
  ////////////////////////////////////////
  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_MAX,
  STREAM_MODULE_VIS_GTK_CAIRO_SPECTRUMANALYZER_MODE_INVALID
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
          typename SessionDataContainerType>
class Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType>
 , public Common_Math_FFT
 , public Common_ICounter
{
 public:
  Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ();
  virtual ~Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T ();

  virtual bool initialize (const ConfigurationType&);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Common_ICounter (triggers frame rendering)
  virtual void reset ();

 private:
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_SessionMessageType> inherited;
  typedef Common_Math_FFT inherited2;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T (const Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T& operator= (const Stream_Module_Vis_GTK_Cairo_SpectrumAnalyzer_T&))

  virtual int svc (void);

#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
  bool initialize_Cairo (GdkWindow*,
                         cairo_t*&,
                         cairo_surface_t*&);
#else
  bool initialize_Cairo (GdkWindow*,
                         cairo_t*&,
                         GdkPixbuf*&);
#endif
  void update ();

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: callers must free the return value !
  template <typename FormatType> AM_MEDIA_TYPE* getFormat (const FormatType format_in) { return getFormat_impl (format_in); }
  AM_MEDIA_TYPE* getFormat_impl (const struct _AMMediaType*); // return value: media type handle
  AM_MEDIA_TYPE* getFormat_impl (const IMFMediaType*); // return value: media type handle
#endif

  cairo_t*                                                 cairoContext_;
#if defined (GTK_MAJOR_VERSION) && (GTK_MAJOR_VERSION >= 3)
  cairo_surface_t*                                         cairoSurface_;
#else
  GdkPixbuf*                                               pixelBuffer_;
#endif
  double                                                   channelFactor_;
  double                                                   scaleFactorX_;
  double                                                   scaleFactorY_;
  int                                                      height_;
  int                                                      width_;
  ACE_SYNCH_MUTEX_T*                                       lock_;
  Stream_Module_Visualization_GTKCairoSpectrumAnalyzerMode mode_;
  Stream_ResetCounterHandler                               renderHandler_;
  long                                                     renderHandlerTimerID_;
  SampleIterator                                           sampleIterator_;
};

// include template definition
#include "stream_vis_gtk_cairo_spectrum_analyzer.inl"

#endif
