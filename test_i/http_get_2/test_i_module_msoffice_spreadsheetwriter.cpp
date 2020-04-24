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

#include "test_i_module_msoffice_spreadsheetwriter.h"

#include "ace/Log_Msg.h"

#include "common_file_tools.h"

#include "common_timer_tools.h"

#include "stream_macros.h"

//#include "stream_dec_common.h"

#include "stream_document_defines.h"

#include "test_i_http_get_defines.h"

const char libacestream_default_doc_msoffice_writer_module_name_string[] =
ACE_TEXT_ALWAYS_CHAR (MODULE_DOCUMENT_MSOFFICE_WRITER_DEFAULT_NAME_STRING);

//////////////////////////////////////////

HRESULT AutoWrap(int autoType, VARIANT *pvResult, IDispatch *pDisp, LPOLESTR ptName, int cArgs...) {
    // Begin variable-argument list...
    va_list marker;
    va_start(marker, cArgs);

    if(!pDisp) {
        MessageBox(NULL, "NULL IDispatch passed to AutoWrap()", "Error", 0x10010);
        _exit(0);
    }

    // Variables used...
    DISPPARAMS dp = { NULL, NULL, 0, 0 };
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;
    char buf[200];
    char szName[200];


    // Convert down to ANSI
    WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);

    // Get DISPID for name passed...
    hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
    if(FAILED(hr)) {
        sprintf(buf, "IDispatch::GetIDsOfNames(\"%s\") failed w/err 0x%08lx", szName, hr);
        MessageBox(NULL, buf, "AutoWrap()", 0x10010);
        _exit(0);
        return hr;
    }

    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[cArgs+1];
    // Extract arguments...
    for(int i=0; i<cArgs; i++) {
        pArgs[i] = va_arg(marker, VARIANT);
    }

    // Build DISPPARAMS
    dp.cArgs = cArgs;
    dp.rgvarg = pArgs;

    // Handle special-case for property-puts!
    if(autoType & DISPATCH_PROPERTYPUT) {
        dp.cNamedArgs = 1;
        dp.rgdispidNamedArgs = &dispidNamed;
    }

    // Make the call!
    hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, autoType, &dp, pvResult, NULL, NULL);
    if(FAILED(hr)) {
        sprintf(buf, "IDispatch::Invoke(\"%s\"=%08lx) failed w/err 0x%08lx", szName, dispID, hr);
        MessageBox(NULL, buf, "AutoWrap()", 0x10010);
        _exit(0);
        return hr;
    }
    // End variable-argument section...
    va_end(marker);

    delete [] pArgs;

    return hr;
}

//////////////////////////////////////////

Test_I_MSOffice_SpreadsheetWriter::Test_I_MSOffice_SpreadsheetWriter (ISTREAM_T* stream_in)
 : inherited (stream_in)
 , application_ (NULL)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MSOffice_SpreadsheetWriter::Test_I_MSOffice_SpreadsheetWriter"));

}

Test_I_MSOffice_SpreadsheetWriter::~Test_I_MSOffice_SpreadsheetWriter ()
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MSOffice_SpreadsheetWriter::~Test_I_MSOffice_SpreadsheetWriter"));

  if (application_)
    application_->Release ();
}

void
Test_I_MSOffice_SpreadsheetWriter::handleSessionMessage (Test_I_Stream_SessionMessage*& message_inout,
                                                         bool& passMessageDownstream_out)
{
  STREAM_TRACE (ACE_TEXT ("Test_I_MSOffice_SpreadsheetWriter::handleSessionMessage"));

//  int result = -1;

  // don't care (implies yes per default, if part of a stream)
  ACE_UNUSED_ARG (passMessageDownstream_out);

  // sanity check(s)
  ACE_ASSERT (inherited::configuration_);

  const Test_I_HTTPGet_SessionData_t& session_data_container_r =
    message_inout->getR ();
  struct Test_I_HTTPGet_SessionData& session_data_r =
    const_cast<struct Test_I_HTTPGet_SessionData&> (session_data_container_r.getR ());
  switch (message_inout->type ())
  {
    case STREAM_SESSION_MESSAGE_BEGIN:
    {
      // --> create new frame (see below)
      CoInitialize (NULL);

      CLSID clsid;
      HRESULT hr = CLSIDFromProgID (L"Excel.Application", &clsid);
      if (FAILED (hr))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CLSIDFromProgID(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT ("Excel.Application"),
                    ACE_TEXT (Common_Error_Tools::errorToString (hr, false, false).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (!application_);
      hr = CoCreateInstance (clsid,
                             NULL,
                             CLSCTX_LOCAL_SERVER,
                             IID_IDispatch,
                             (void**)&application_);
      if (FAILED (hr))
      {
        ACE_DEBUG ((LM_ERROR,
                    ACE_TEXT ("%s: failed to CoCreateInstance(\"%s\"): \"%s\", aborting\n"),
                    inherited::mod_->name (),
                    ACE_TEXT (Common_Tools::GUIDToString (clsid).c_str ()),
                    ACE_TEXT (Common_Error_Tools::errorToString (hr, false, false).c_str ())));
        goto error;
      } // end IF
      ACE_ASSERT (application_);
      {
        VARIANT x;
        x.vt = VT_I4;
        x.lVal = 1;
        AutoWrap (DISPATCH_PROPERTYPUT, NULL, application_, L"Visible", 1, x);
      }

      IDispatch* books_p = NULL;
      {
        VARIANT result;
        VariantInit (&result);
        AutoWrap (DISPATCH_PROPERTYGET, &result, application_, L"Workbooks", 0);
        books_p = result.pdispVal;
      }
      ACE_ASSERT (books_p);
      IDispatch* book_p = NULL;
      {
        VARIANT result;
        VariantInit (&result);
        AutoWrap (DISPATCH_PROPERTYGET, &result, books_p, L"Add", 0);
        book_p = result.pdispVal;
      }
      ACE_ASSERT (book_p);
      IDispatch* sheet_p = NULL;
      {
        VARIANT result;
        VariantInit (&result);
        AutoWrap (DISPATCH_PROPERTYGET, &result, application_, L"ActiveSheet", 0);
        sheet_p = result.pdispVal;
      }
      ACE_ASSERT (sheet_p);

      VARIANT arr;
      arr.vt = VT_ARRAY | VT_VARIANT;
      {
        SAFEARRAYBOUND sab[2];
        sab[0].lLbound = 1; sab[0].cElements = 15;
        sab[1].lLbound = 1; sab[1].cElements = 15;
        arr.parray = SafeArrayCreate (VT_VARIANT, 2, sab);
      }
      for (int i = 1; i <= 15; i++) {
        for (int j = 1; j <= 15; j++) {
          VARIANT tmp;
          tmp.vt = VT_I4;
          tmp.lVal = i * j;
          long indices[] = { i,j };
          SafeArrayPutElement (arr.parray, indices, (void*)&tmp);
        }
      }
      IDispatch* range_p = NULL;
      {
        VARIANT parm;
        parm.vt = VT_BSTR;
        parm.bstrVal = ::SysAllocString (L"A1:O15");

        VARIANT result;
        VariantInit (&result);
        AutoWrap (DISPATCH_PROPERTYGET, &result, sheet_p, L"Range", 1, parm);
        range_p = result.pdispVal;

        VariantClear (&parm);
      }
      ACE_ASSERT (range_p);
      AutoWrap (DISPATCH_PROPERTYPUT, NULL, range_p, L"Value", 1, arr);

      {
        VARIANT x;
        x.vt = VT_I4;
        x.lVal = 1;
        AutoWrap (DISPATCH_PROPERTYPUT, NULL, book_p, L"Saved", 1, x);
      }
      AutoWrap (DISPATCH_METHOD, NULL, application_, L"Quit", 0);

      break;

error:
      //notify (STREAM_SESSION_MESSAGE_ABORT);

      return;
    }
    case STREAM_SESSION_MESSAGE_END:
    {
      ACE_DEBUG ((LM_DEBUG,
                  ACE_TEXT ("%s: wrote %u record(s)...\n"),
                  inherited::mod_->name (),
                  session_data_r.data.size ()));

continue_:
error_2:
      break;
    }
    default:
      break;
  } // end SWITCH
}
