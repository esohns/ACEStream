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

#ifndef STREAM_MODULE_HTMLPARSER_H
#define STREAM_MODULE_HTMLPARSER_H

#include "ace/Global_Macros.h"

#include "libxml/HTMLparser.h"
#include "libxml/tree.h"

#include "common_time_common.h"

#include "stream_imodule.h"
#include "stream_task_base_synch.h"

#include "stream_html_exports.h"

// SAX callbacks
//void Stream_Export
//SAXDefaultStartDocument (void*); // user data
//void Stream_Export
//SAXDefaultEndDocument (void*); // user data
//void Stream_Export
//SAXDefaultCharacters (void*,          // user data
//                      const xmlChar*, // string
//                      int);           // length
//void Stream_Export
//SAXDefaultStartElement (void*,            // user data
//                        const xmlChar*,   // name
//                        const xmlChar**); // attributes
//void Stream_Export
//SAXDefaultEndElement (void*,           // user data
//                      const xmlChar*); // name
//xmlEntityPtr Stream_Export
//SAXDefaultSGetEntity (void*,           // user data
//                      const xmlChar*); // name

void Stream_HTML_Export
SAXDefaultErrorCallback (void*,       // context
                         const char*, // message
                         ...);        // arguments
void Stream_HTML_Export
SAXDefaultStructuredErrorCallback (void*,        // user data
                                   xmlErrorPtr); // error

// definitions
#define STREAM_MODULE_HTMLPARSER_DEFAULT_MODE STREAM_MODULE_HTMLPARSER_DOM

// types
enum Stream_Module_HTMLParser_Mode
{
  STREAM_MODULE_HTMLPARSER_INVALID = -1,
  /////////////////////////////////////
  STREAM_MODULE_HTMLPARSER_DOM = 0,
  STREAM_MODULE_HTMLPARSER_SAX
};

struct Stream_Module_HTMLParser_SAXParserContextBase
{
  inline Stream_Module_HTMLParser_SAXParserContextBase ()
   : parserContext (NULL)
  {};

  htmlParserCtxtPtr parserContext;
};

template <typename SessionMessageType,
          typename MessageType,
          ///////////////////////////////
          typename ModuleHandlerConfigurationType,
          ///////////////////////////////
          typename SessionDataType,
          ///////////////////////////////
          typename ParserContextType>
class Stream_Module_HTMLParser_T
 : public Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType>
 , public Stream_IModuleHandler_T<ModuleHandlerConfigurationType>
{
 public:
  Stream_Module_HTMLParser_T ();
  virtual ~Stream_Module_HTMLParser_T ();

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (MessageType*&, // data message handle
                                  bool&);        // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement Stream_IModuleHandler_T
  virtual const ModuleHandlerConfigurationType& get () const;
  virtual bool initialize (const ModuleHandlerConfigurationType&);

 protected:
  virtual bool initializeSAXParser ();

  ModuleHandlerConfigurationType* configuration_;
  ParserContextType               parserContext_;
  htmlSAXHandler                  SAXHandler_;

 private:
  typedef Stream_TaskBaseSynch_T<Common_TimePolicy_t,
                                 SessionMessageType,
                                 MessageType> inherited;

  ACE_UNIMPLEMENTED_FUNC (Stream_Module_HTMLParser_T (const Stream_Module_HTMLParser_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_HTMLParser_T& operator= (const Stream_Module_HTMLParser_T&))

  bool                            initialized_;
  Stream_Module_HTMLParser_Mode   mode_;
};

#include "stream_module_htmlparser.inl"

#endif
