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

#ifndef STREAM_MODULE_PARSER_H
#define STREAM_MODULE_PARSER_H

#include <iostream>
#include <string>

#include "ace/Global_Macros.h"
#include "ace/Synch_Traits.h"

#include "common_time_common.h"

#include "stream_common.h"
#include "stream_imodule.h"
#include "stream_iparser.h"
#include "stream_task_base_synch.h"

//#include "stream_misc_exports.h"

extern const char libacestream_default_misc_parser_module_name_string[];

template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ScannerType, // (f/)lex-
          typename ScannerStateType, // implements struct Common_ScannerState
          typename ParserType, // yacc/bison-
          typename ParserConfigurationType,
          typename ParserInterfaceType, // implements Common_IParser_T
          typename ParserArgumentType, // yacc/bison-
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_CppParser_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public ParserInterfaceType
 , virtual public Common_ILexScanner_T<ScannerStateType,
                                       ParserInterfaceType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_CppParser_T (ISTREAM_T*,                     // stream handle
#else
  Stream_Module_CppParser_T (typename inherited::ISTREAM_T*, // stream handle
#endif
                             bool = false,                   // debug scanning ?
                             bool = false);                  // debug parsing ?
  virtual ~Stream_Module_CppParser_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement (part of) ParserInterfaceType
  inline virtual bool initialize (const ParserConfigurationType&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  inline virtual void dump_state () const { ACE_ASSERT (false); ACE_NOTSUP; };
  virtual bool parse (ACE_Message_Block*); // data buffer handle
//  virtual void error (const YYLTYPE&,      // location
  inline virtual void error (const yy::location&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; };

  // implement (part of) Common_ILexScanner_T
  inline virtual ACE_Message_Block* buffer () { return fragment_; };
  inline virtual bool isBlocking () const { return blockInParse_; };
  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; }; // offset (increment)
  inline virtual unsigned int offset () const { return offset_; };
  virtual bool begin (const char*,   // buffer
                      unsigned int); // size
  virtual void end ();
  virtual bool switchBuffer (bool = false); // unlink current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void waitBuffer ();
  virtual void error (const std::string&); // message
  inline virtual void debug (yyscan_t state_in, bool toggle_in) { scanner_ .debug (state_in, toggle_in); };
  inline virtual bool initialize (yyscan_t& state_in) { return scanner_.initialize (state_in); };
  virtual void finalize (yyscan_t& state_in) { scanner_.finalize (state_in); };
  virtual bool lex (yyscan_t& state_in) { return (scanner_.yylex (state_in) == 0); };
  inline virtual struct yy_buffer_state* create (yyscan_t state_in, char* buffer_in, size_t size_in) { return scanner_.create (state_in, buffer_in, size_in); };
  inline virtual void destroy (yyscan_t state_in, struct yy_buffer_state*& buffer_inout) { scanner_.destroy (state_in, buffer_inout); };
  inline virtual const ScannerStateType& getR () const { return scannerState_; };
  inline virtual const ParserInterfaceType* const getP () const { return this; };
  inline virtual void setP (ParserInterfaceType* interfaceHandle_in) { scanner_.set (interfaceHandle_in); };

 protected:
  ParserConfigurationType* configuration_;

  ACE_Message_Block*       fragment_;
  unsigned int             offset_; // parsed fragment bytes
  bool                     trace_;

  // parser
  ParserType               parser_;
//  ArgumentType            argument_;

  // scanner
  ScannerType              scanner_;
  ScannerStateType         scannerState_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_CppParser_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_CppParser_T (const Stream_Module_CppParser_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_CppParser_T& operator= (const Stream_Module_CppParser_T&))

  // override some ACE_Task_T methods
  virtual int svc (void);

  // helper types
  struct MEMORY_BUFFER_T
   : std::streambuf
  {
    void set (char* buffer_in, unsigned int size_in) {
      this->setg (buffer_in, buffer_in, buffer_in + size_in);
    }
  };

  bool                     blockInParse_;
  bool                     isFirst_;

  struct yy_buffer_state*  buffer_;
  MEMORY_BUFFER_T          streamBuffer_;
  std::istream             stream_;
};

//////////////////////////////////////////

// *NOTE*: the current implementation only supports (f/)lex-based scanners
// *TODO*: support yacc/bison-based parsing
template <ACE_SYNCH_DECL,
          typename TimePolicyType,
          ////////////////////////////////
          typename ConfigurationType,
          ////////////////////////////////
          typename ControlMessageType,
          typename DataMessageType,
          typename SessionMessageType,
          ////////////////////////////////
          typename ScannerStateType, // implements struct Common_ScannerState
          typename ParserType, // yacc/bison-
          typename ParserConfigurationType,
          typename ParserInterfaceType, // implements Common_IParser_T
          typename ParserArgumentType, // yacc/bison-
          ////////////////////////////////
          typename UserDataType>
class Stream_Module_Parser_T
 : public Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType>
 , public ParserInterfaceType
 , virtual public Common_ILexScanner_T<ScannerStateType,
                                       ParserInterfaceType>
{
  typedef Stream_TaskBaseSynch_T<ACE_SYNCH_USE,
                                 TimePolicyType,
                                 ConfigurationType,
                                 ControlMessageType,
                                 DataMessageType,
                                 SessionMessageType,
                                 Stream_SessionId_t,
                                 enum Stream_ControlType,
                                 enum Stream_SessionMessageType,
                                 UserDataType> inherited;

 public:
  // convenient types
  typedef Common_ILexScanner_T<ScannerStateType,
                               ParserInterfaceType> ISCANNER_T;

  // *TODO*: on MSVC 2015u3 the accurate declaration does not compile
#if defined (ACE_WIN32) || defined (ACE_WIN64)
  Stream_Module_Parser_T (ISTREAM_T*,                     // stream handle
#else
  Stream_Module_Parser_T (typename inherited::ISTREAM_T*, // stream handle
#endif
                          bool,  // debug scanning ?
                          bool); // debug parsing ?
  virtual ~Stream_Module_Parser_T ();

  virtual bool initialize (const ConfigurationType&,
                           Stream_IAllocator* = NULL);

  // implement (part of) Stream_ITaskBase_T
  virtual void handleDataMessage (DataMessageType*&, // data message handle
                                  bool&);            // return value: pass message downstream ?
  virtual void handleSessionMessage (SessionMessageType*&, // session message handle
                                     bool&);               // return value: pass message downstream ?

  // implement (part of) ParserInterfaceType
  inline virtual bool initialize (const ParserConfigurationType&) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) };
  inline virtual void dump_state () const { ACE_ASSERT (false); ACE_NOTSUP; };
  virtual bool parse (ACE_Message_Block*); // data buffer handle
//  virtual void error (const YYLTYPE&,      // location
  inline virtual void error (const yy::location&, const std::string&) { ACE_ASSERT (false); ACE_NOTSUP; };

  // implement (part of) Common_ILexScanner_T
  inline virtual ACE_Message_Block* buffer () { return fragment_; };
//  inline virtual bool debug () const { return bittorrent_get_debug (scannerState_); };
  inline virtual bool isBlocking () const { return blockInParse_; };
  inline virtual void offset (unsigned int offset_in) { offset_ += offset_in; }; // offset (increment)
  inline virtual unsigned int offset () const { return offset_; };
  virtual bool begin (const char*,   // buffer
                      unsigned int); // size
  virtual void end ();
  virtual bool switchBuffer (bool = false); // unlink current fragment ?
  // *NOTE*: (waits for and) appends the next data chunk to fragment_;
  virtual void waitBuffer ();
  virtual void error (const std::string&); // message
  inline virtual const ScannerStateType& getR_3 () const { return scannerState_; };
  inline virtual const ParserInterfaceType* const getP_2 () const { return this; };
  inline virtual void setP (ParserInterfaceType*) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) };

 protected:
  ParserConfigurationType* configuration_;

  DataMessageType*         fragment_;
  bool                     isFirst_;
  unsigned int             offset_; // parsed fragment bytes
  bool                     trace_;

  // parser
//  ParserType               parser_;
//  ArgumentType            argument_;

  // scanner
  struct yy_buffer_state*  buffer_;
  yyscan_t                 state_;
  ScannerStateType         scannerState_;
  bool                     useYYScanBuffer_;

 private:
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Parser_T ())
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Parser_T (const Stream_Module_Parser_T&))
  ACE_UNIMPLEMENTED_FUNC (Stream_Module_Parser_T& operator= (const Stream_Module_Parser_T&))

  // stub (part of) Common_ILexScanner_T
  inline virtual void debug (yyscan_t, bool) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual bool initialize (yyscan_t&, ScannerStateType*) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }
  inline virtual void finalize (yyscan_t&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual struct yy_buffer_state* create (yyscan_t, char*, size_t) { ACE_ASSERT (false); ACE_NOTSUP_RETURN (NULL); ACE_NOTREACHED (return NULL;) }
  inline virtual void destroy (yyscan_t, struct yy_buffer_state*&) { ACE_ASSERT (false); ACE_NOTSUP; ACE_NOTREACHED (return;) }
  inline virtual bool lex () { ACE_ASSERT (false); ACE_NOTSUP_RETURN (false); ACE_NOTREACHED (return false;) }

  // override some ACE_Task_T methods
  virtual int svc (void);

  // helper types
  struct MEMORY_BUFFER_T
   : std::streambuf
  {
    void set (char* buffer_in, unsigned int size_in) {
      this->setg (buffer_in, buffer_in, buffer_in + size_in);
    }
  };

  bool                     blockInParse_;

  MEMORY_BUFFER_T          streamBuffer_;
  std::istream             stream_;
};

// include template definition
#include "stream_misc_parser.inl"

#endif
