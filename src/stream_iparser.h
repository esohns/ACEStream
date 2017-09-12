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

#ifndef STREAM_IPARSER_H
#define STREAM_IPARSER_H

#include "common_iscanner.h"

template <typename ConfigurationType,
          typename RecordType>
class Stream_IYaccStreamParser_T
 : public Common_IYaccParser_T<ConfigurationType>
{
 public:
  virtual RecordType& current () = 0;

  ////////////////////////////////////////
  // callbacks
  // *IMPORTANT NOTE*: fire-and-forget API
  virtual void record (RecordType*&) = 0; // data record
};

//////////////////////////////////////////

template <typename ConfigurationType,
          typename RecordType>
class Stream_IYaccRecordParser_T
 : public Stream_IYaccStreamParser_T<ConfigurationType,
                                     RecordType>
{
 public:
  // convenient types
  typedef Stream_IYaccStreamParser_T<ConfigurationType,
                                     RecordType> IPARSER_T;

  virtual bool hasFinished () const = 0;
};

#endif
