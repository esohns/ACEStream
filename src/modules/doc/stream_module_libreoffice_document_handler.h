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

#ifndef STREAM_MODULE_LIBREOFFICE_DOCUMENT_HANDLER_H
#define STREAM_MODULE_LIBREOFFICE_DOCUMENT_HANDLER_H

#include "ace/Global_Macros.h"

#include "sal/types.h"

#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/uno/Reference.h"
//#include "com/sun/star/uno/Sequence.h"

#include "cppuhelper/implbase1.hxx"

#include "stream_document_exports.h"

using namespace ::com::sun::star;

class Stream_Document_Export Stream_Module_LibreOffice_Document_Handler
 : public ::cppu::WeakImplHelper1<task::XInteractionHandler>
{
 public:
  Stream_Module_LibreOffice_Document_Handler ();
  inline virtual ~Stream_Module_LibreOffice_Document_Handler () {}

  // implement XInteractionHandler
  //virtual uno::Any SAL_CALL queryInterface (const uno::Type&) /* throw (uno::RuntimeException, ::std::exception) */;
  //virtual void SAL_CALL acquire () /* throw () */;
  //virtual void SAL_CALL release () /* throw () */;

  virtual void SAL_CALL handle (const uno::Reference<task::XInteractionRequest>&) /* throw (uno::RuntimeException, ::std::exception) */;

 private:
  typedef ::cppu::WeakImplHelper1<task::XInteractionHandler> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LibreOffice_Document_Handler (const Stream_Module_LibreOffice_Document_Handler&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LibreOffice_Document_Handler& operator= (const Stream_Module_LibreOffice_Document_Handler&))
};

#endif
