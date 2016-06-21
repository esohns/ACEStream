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
#include "stdafx.h"

#include "stream_module_libreoffice_document_handler.h"

#include "ace/Log_Msg.h"

#include "cppuhelper/queryinterface.hxx"
//#include "com/sun/star/beans/XPropertySet.hpp"
//#include "com/sun/star/bridge/XUnoUrlResolver.hpp"
//#include "com/sun/star/frame/Desktop.hpp"
//#include "com/sun/star/frame/XComponentLoader.hpp"
//#include "com/sun/star/lang/XMultiComponentFactory.hpp"

#include "stream_macros.h"

Stream_Module_LibreOffice_Document_Handler::Stream_Module_LibreOffice_Document_Handler ()
 : inherited ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Handler::Stream_Module_LibreOffice_Document_Handler"));

}

Stream_Module_LibreOffice_Document_Handler::~Stream_Module_LibreOffice_Document_Handler ()
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Handler::~Stream_Module_LibreOffice_Document_Handler"));

}

//uno::Any SAL_CALL
//Stream_Module_LibreOffice_Document_Handler::queryInterface (const uno::Type& type_in)
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Handler::queryInterface"));
//
//  uno::Any result =
//    ::cppu::queryInterface<inherited2> (type_in,
//                                        static_cast<inherited2*> (this));
//  if (result.hasValue ())
//    return result;
//
//  return inherited::queryInterface (type_in);
//}
//void SAL_CALL
//Stream_Module_LibreOffice_Document_Handler::acquire ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Handler::acquire"));
//
//  inherited::acquire ();
//}
//void SAL_CALL
//Stream_Module_LibreOffice_Document_Handler::release ()
//{
//  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Handler::release"));
//
//  inherited::release ();
//}

void SAL_CALL
Stream_Module_LibreOffice_Document_Handler::handle (const uno::Reference<task::XInteractionRequest>& request_in)
{
  STREAM_TRACE (ACE_TEXT ("Stream_Module_LibreOffice_Document_Handler::handle"));

  ACE_UNUSED_ARG (request_in);

  ACE_ASSERT (false);
  ACE_NOTSUP;
  ACE_NOTREACHED (return;)
}
