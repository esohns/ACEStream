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

#ifndef STREAM_MODULE_LIBREOFFCE_DOCUMENT_WRITER_H
#define STREAM_MODULE_LIBREOFFCE_DOCUMENT_WRITER_H

#include "ace/Global_Macros.h"

#include "com/sun/star/lang/XComponent.hpp"
#include "com/sun/star/uno/Reference.h"
#include "com/sun/star/uno/XComponentContext.hpp"

#include "common_time_common.h"

#include "stream_task_base_synch.h"

using namespace ::com::sun::star;

template <typename SessionMessageType,
          typename MessageType,
          ////////////////////////////////
          typename ModuleHandlerConfigurationType,
          ////////////////////////////////
          typename SessionDataType,
          ////////////////////////////////
          typename DocumentType>
class Stream_Module_LibreOffice_Document_Writer_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ModuleHandlerConfigurationType>
{
 public:
  Stream_Module_LibreOffice_Document_Writer_T ();
  virtual ~Stream_Module_LibreOffice_Document_Writer_T ();

  // implement (part of) Stream_ITaskBase_T
  //virtual void handleDataMessage (MessageType*&, // data message handle
  //                                bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual bool initialize (const ModuleHandlerConfigurationType&);
  virtual const ModuleHandlerConfigurationType& get () const;

 protected:
  uno::Reference<lang::XComponent>       component_;
  uno::Reference<uno::XComponentContext> context_;

  ModuleHandlerConfigurationType*        configuration_;
  bool                                   isInitialized_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LibreOffice_Document_Writer_T (const Stream_Module_LibreOffice_Document_Writer_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_LibreOffice_Document_Writer_T& operator= (const Stream_Module_LibreOffice_Document_Writer_T&))
};

#include "stream_module_libreoffice_document_writer.inl"

#endif
