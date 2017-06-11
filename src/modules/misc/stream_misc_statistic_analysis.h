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

#ifndef STREAM_MODULE_MISC_STATISTIC_ANALYSIS_H
#define STREAM_MODULE_MISC_STATISTIC_ANALYSIS_H

#include "ace/config-lite.h"
#if defined (ACE_WIN32) || defined (ACE_WIN64)
#include <mfapi.h>
#include <mfobjects.h>
#include <strmif.h>
#endif

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_inotify.h"

#include "common_math_sample.h"

#include "stream_task_base_synch.h"

//enum Stream_Module_StatisticAnalysis_Mode
//{
//  STREAM_MODULE_STATISTICANALYSIS_MODE_SLIDINGAVERAGE = 0,
//  ////////////////////////////////////////
//  STREAM_MODULE_STATISTICANALYSIS_MODE_MAX,
//  STREAM_MODULE_STATISTICANALYSIS_MODE_INVALID
//};

enum Stream_Module_StatisticAnalysis_Event
{
  STREAM_MODULE_STATISTICANALYSIS_EVENT_ACTIVITY = 0,
  STREAM_MODULE_STATISTICANALYSIS_EVENT_PEAK,
  ////////////////////////////////////////
  STREAM_MODULE_STATISTICANALYSIS_EVENT_MAX,
  STREAM_MODULE_STATISTICANALYSIS_EVENT_INVALID
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
          typename StatisticContainerType,
          typename SessionDataType,
          typename SessionDataContainerType,
          ////////////////////////////////
          typename ValueType,
          unsigned int Aggregation>
class Stream_Module_StatisticAnalysis_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 Stream_UserData>
 , public Common_Math_Sample_T<ValueType,
                               Aggregation>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 Stream_UserData> inherited;
  typedef Common_Math_Sample_T<ValueType,
                               Aggregation> inherited2;

 public:
  Stream_Module_StatisticAnalysis_T (typename inherited::ISTREAM_T*); // stream handle
  virtual ~Stream_Module_StatisticAnalysis_T ();

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  typedef Common_IDispatch_T<Stream_Module_StatisticAnalysis_Event> INOTIFY_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticAnalysis_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticAnalysis_T (const Stream_Module_StatisticAnalysis_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_StatisticAnalysis_T& operator= (const Stream_Module_StatisticAnalysis_T&))

  //virtual int svc (void);

#if defined (ACE_WIN32) || defined (ACE_WIN64)
  // *NOTE*: callers must free the return value !
  template <typename FormatType> AM_MEDIA_TYPE* getFormat (const FormatType format_in) { return getFormat_impl (format_in); }
  AM_MEDIA_TYPE* getFormat_impl (const struct _AMMediaType*); // return value: media type handle
  AM_MEDIA_TYPE* getFormat_impl (const IMFMediaType*); // return value: media type handle
#endif

  virtual void Process (unsigned int,  // starting slot index
                        unsigned int); // ending slot index
  inline virtual ValueType Value (unsigned int slot_in,
                                  unsigned int subSlot_in) const
  { ACE_ASSERT (false);
    ACE_NOTSUP_RETURN (0);
    ACE_NOTREACHED (return 0;)
  };

  // peak detection

  ValueType                       amplitudeSum_;
  ValueType                       amplitudeSumSqr_;
  double                          amplitudeVariance_;

  // activity detection
  unsigned int                    streak_;
  unsigned int                    streakCount_;
  double                          streakSum_;
  double                          streakSumSqr_;
  double                          streakVariance_;
  ValueType                       volume_;
  double                          volumeSum_;
  double                          volumeSumSqr_;
  double                          volumeVariance_;

  INOTIFY_T*                      eventDispatcher_;
  typename inherited2::ITERATOR_T iterator_;
  unsigned int                    sampleCount_;
};

// include template definition
#include "stream_misc_statistic_analysis.inl"

#endif
