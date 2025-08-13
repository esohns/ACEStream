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

#ifndef STREAM_STAT_STATISTIC_ANALYSIS_H
#define STREAM_STAT_STATISTIC_ANALYSIS_H

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_inotify.h"

#include "common_math_sample.h"

#include "stream_task_base_synch.h"

#include "stream_lib_mediatype_converter.h"

#include "stream_stat_common.h"

extern const char libacestream_default_stat_analysis_module_name_string[];

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
          typename MediaType,
          typename ValueType> // buffer value-
class Stream_Statistic_StatisticAnalysis_T
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
 , public Common_Math_Sample_T<ValueType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_ControlType,
                                 Stream_SessionMessageType,
                                 Stream_UserData> inherited;
  typedef Stream_MediaFramework_MediaTypeConverter_T<MediaType> inherited2;
  typedef Common_Math_Sample_T<ValueType> inherited3;

 public:
  Stream_Statistic_StatisticAnalysis_T (typename inherited::ISTREAM_T*); // stream handle
  inline virtual ~Stream_Statistic_StatisticAnalysis_T () {}

  // override (part of) Stream_IModuleHandler_T
  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 private:
  // convenient types
  typedef Common_IDispatch_T<enum Stream_Statistic_AnalysisEventType> INOTIFY_T;

  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticAnalysis_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticAnalysis_T (const Stream_Statistic_StatisticAnalysis_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Statistic_StatisticAnalysis_T& operator= (const Stream_Statistic_StatisticAnalysis_T&))

  virtual void Process (unsigned int,  // channel#
                        unsigned int,  // starting slot index
                        unsigned int); // ending slot index
  inline virtual ValueType Value (unsigned int slot_in, unsigned int subSlot_in) const { ACE_UNUSED_ARG (slot_in); ACE_UNUSED_ARG (subSlot_in); ACE_ASSERT (false); ACE_NOTSUP_RETURN (0); ACE_NOTREACHED (return 0;) }

  // peak detection
  double                          amplitudeM_; // mean
  double                          amplitudeS_;

  // activity detection
  unsigned int                    streak_;
  unsigned int                    streakCount_; // total-
  double                          streakM_;
  double                          streakS_;
  double                          volumeM_;
  double                          volumeS_;

  INOTIFY_T*                      eventDispatcher_;
  typename inherited3::ITERATOR_T iterator_;
  unsigned int                    frameCount_; // processed-
  bool                            sampleIsSigned_;
};

// include template definition
#include "stream_stat_statistic_analysis.inl"

#endif
