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

#include "stream_task_base_synch.h"

extern const char libacestream_default_html_parser_module_name_string[];

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

void
stream_html_parser_sax_default_error_cb (void*,       // context
                                         const char*, // message
                                         ...);        // arguments
void
stream_html_parser_sax_default_structured_error_cb (void*,        // user data
                                                    xmlErrorPtr); // error

// definitions
#define STREAM_MODULE_HTMLPARSER_DEFAULT_MODE STREAM_MODULE_HTMLPARSER_MODE_DOM

// types
enum Stream_Module_HTMLParser_Mode
{
  STREAM_MODULE_HTMLPARSER_MODE_INVALID = -1,
  ////////////////////////////////////////
  STREAM_MODULE_HTMLPARSER_MODE_DOM = 0, // document (tree)
  STREAM_MODULE_HTMLPARSER_MODE_SAX      // stream (state machine)
};

struct Stream_Module_HTMLParser_SAXParserContextBase
{
  Stream_Module_HTMLParser_SAXParserContextBase ()
   : accumulate (false)
   , characters ()
   , parserContext (NULL)
   , sessionData (NULL)
  {}

  // *NOTE*: for some reason, libxml2 serves some 'characters' data in chunks
  //         --> buffer it in this member
  bool                       accumulate;
  std::string                characters;
  htmlParserCtxtPtr          parserContext;
  struct Stream_SessionData* sessionData;
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
          typename SessionDataContainerType,
          typename SessionDataType,
          ////////////////////////////////
          typename ParserContextType>
class Stream_Module_HTMLParser_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 struct Stream_UserData> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_HTMLParser_T (ISTREAM_T*);                     // stream handle
#else
  Stream_Module_HTMLParser_T (typename inherited::ISTREAM_T*); // stream handle
#endif
  virtual ~Stream_Module_HTMLParser_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

 protected:
  inline virtual bool initializeSAXParser () { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }

  DataMessageType*                   headFragment_;
  ParserContextType                  parserContext_;
  htmlSAXHandler                     SAXHandler_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_HTMLParser_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_HTMLParser_T (const Stream_Module_HTMLParser_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_HTMLParser_T& operator= (const Stream_Module_HTMLParser_T&))

  // helper methods
  bool resetParser ();

  enum Stream_Module_HTMLParser_Mode mode_;
};

// include template definition
#include "stream_module_htmlparser.inl"

#endif
