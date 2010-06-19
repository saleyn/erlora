/*
***** BEGIN LICENSE BLOCK *****

This file is part of the ERLORA (Erlang Oracle Interface) Library.

Copyright (c) 2006 Serge Aleynikov <saleyn@gmail.com>

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Lesser General Public
License as published by the Free Software Foundation; either
version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

***** END LICENSE BLOCK *****
*/

#include <stdio.h>
#include <string.h>

#include "eco_error.h"

// error codes & descriptions
const ECO_ERROR eco_err::_errs[] = {
{ EX_ENC, "encode"},
{ EX_DEC, "decode"},
{ EX_ORA, "ora"},
{ EX_OCCI, "occi"},
{ EX_PROC, "processing_internal"},
{ EX_TERM, "term "},
{ EX_CMD, "cmd "},
{ EX_ARG, "argument "},
{ EX_CMD_UNKN, "unknown command "},
{ EX_TUPLE, "tuple "},
{ EX_TYPE, "element type "},
{ EX_SZ, "element size "},
{ EX_OPT, "options "},
{ EX_HEADER, "header "},
{ EX_LIST, "list "},
{ EX_ATOM, "atom "},
{ EX_INT, "integer "},
{ EX_STR, "string "},
{ EX_BIN, "binary "},
{ EX_ERL_MEM, "erl memory managment "},
{ EX_ALLOC, "C  memory managment "},
{ EX_PRS, "parsing "},
{ EX_CNSTR, "connection string "},
{ EX_MK_STMT, "creating statement "},
{ EX_EX_STMT,"executing statement "},
{ EX_FETCH, "fetching data "},
{ EX_INCONS, "inconsistency "},
{ EX_UNKN, "unknown error "}
};

using namespace std;
extern int gdebug;
extern fstream gcout;
//support functions
int eco_err::err_msg_size(int ec, int cl )
{
  int num=0;
  int imin, imax;

  if(cl){
    imin = 0;
    imax = ERROR_CLASSES;
  } else {
    imin = ERROR_CLASSES;
    imax = ERROR_NUM;
  }

  for(int ii = imin; ii < imax ; ii++)
    if(ec & _errs[ii].err_code)
       num += strlen(_errs[ii].err_desc)+1;

  return num;
}

char* eco_err::get_error_msg(int ec, int cl )
{
  int size = err_msg_size(ec, cl);
  char* msg = (char*) malloc(size);
  int cp = 1;  // 1 copy, 0 cat
  int imin, imax;
  int cl_fl = 0xF;
  if(cl){
    ec &= cl_fl;
    imin = 0;
    imax = ERROR_CLASSES;
  } else {
    ec &= ~cl_fl;
    imin = ERROR_CLASSES;
    imax = ERROR_NUM;
  }

  if(!msg)
  {
     if(gdebug){
       gcout << "Could not allocate " << size << " bytes for error msg" << endl;
       gcout << "ec = " << hex << ec << dec << endl;
     }
     return NULL;
  }

  for(int ii = imin; ii < imax; ii++)
  {
    if( !(ec & _errs[ii].err_code))
      continue;

    if(cp) {
      strcpy(msg, _errs[ii].err_desc);
      cp = 0;
    }
    else
      strcat(msg, _errs[ii].err_desc);
  }

  return msg;
}

char* eco_err::get_error_class(int ec)
{
  int ecl = ec & 0xF;
  if(!ecl)
    return NULL;
  else
    return get_error_msg(ecl, 1);
}

int eco_err::get_err_code(int ii)
{
  return _errs[ii].err_code;
}

/// For debuging
int eco_err::err_count(int ec )
{
  int num=0;

  for(int ii = 0; ii< ERROR_NUM; ii++)
    if(ec & _errs[ii].err_code) num++;

	      return num;
}

int eco_err::show_err(int ec)
{
   if(!gdebug)
     return 0;

   int num = err_count(ec);
   gcout << num << " error found" << endl;

   for(int ii = 0; ii< ERROR_NUM; ii++)
    if( ec & _errs[ii].err_code )
       gcout << "msg " << ii << ": " << _errs[ii].err_desc << endl;

   return num;
}
