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

#ifndef STREAM_VISUALIZATION_IRESIZE_H
#define STREAM_VISUALIZATION_IRESIZE_H

#include "common_ilock.h"

#include "common_image_common.h"

class Stream_Visualization_IResize
 : public Common_ILock // *TODO*: should ideally be Common_ILock_T<ACE_MT_SYNCH>
{
 public:
  virtual void resize (const Common_Image_Resolution_t&) = 0; // new resolution

  // *NOTE*: modules implementing this MUST discard all frames until the next
  // STREAM_SESSION_MESSAGE_RESIZE has been processed, upon which they need to
  // update internal context accordingly
  virtual void resizing () = 0; // new resolution
};

#endif
