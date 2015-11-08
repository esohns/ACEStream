#ifndef STREAM_MODULE_DB_COMMON_H
#define STREAM_MODULE_DB_COMMON_H

#include <string>

#include "ace/INET_Addr.h"

#include "stream_module_db_defines.h"

struct Stream_Module_DataBase_LoginOptions
{
  inline Stream_Module_DataBase_LoginOptions ()
   : database ()
    , host (static_cast<u_short> (MODULE_DB_MYSQL_DEFAULT_PORT),
            static_cast<ACE_UINT32> (INADDR_LOOPBACK))
   , password ()
   , UNIXSocket ()
   , useNamedPipe (false)
   , user (ACE_TEXT_ALWAYS_CHAR (MODULE_DB_MYSQL_DEFAULT_USER))
  {};

  std::string database;
  ACE_INET_Addr host;
  std::string password;
  std::string UNIXSocket;
  bool useNamedPipe;
  std::string user;
};

#endif
