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

#ifndef _ECO_ERRORS
#define _ECO_ERRORS

#include <iostream>
#include <fstream>

#define ERROR_NUM 28    // classes + errors, so far used, could be up to 31
#define ERROR_CLASSES 5 //
// error classes
#define EX_ENC       0x1
#define EX_DEC       0x2
#define EX_ORA       0x4
#define EX_OCCI      0x8
#define EX_PROC      0x10
// errors
#define EX_TERM      0x20 | EX_DEC
#define EX_CMD       0x40 | EX_DEC
#define EX_ARG       0x80 | EX_DEC
#define EX_CMD_UNKN  0x100 | EX_DEC
#define EX_TUPLE     0x200
#define EX_TYPE      0x400 | EX_DEC
#define EX_SZ        0x800
#define EX_OPT       0x1000 | EX_DEC
#define EX_HEADER    0x2000
#define EX_LIST      0x4000
#define EX_ATOM      0x8000
#define EX_INT       0x10000
#define EX_STR       0x20000
#define EX_BIN       0x40000
#define EX_FLOAT     0x80000
#define EX_ERL_MEM   0x100000
#define EX_ALLOC     0x200000
#define EX_PRS       0x400000
#define EX_CNSTR     0x800000
#define EX_MK_STMT   0x1000000
#define EX_EX_STMT   0x2000000
#define EX_FETCH     0x4000000
#define EX_INCONS    0x8000000 | EX_PROC
#define EX_UNKN      0x10000000
// 0x20000000
// 0x40000000

typedef struct eco_error{
  int err_code;
  char* err_desc;
} ECO_ERROR;

class eco_err{
  public:
    eco_err(){};
    ~eco_err(){};

    char* get_error_msg(int ec, int cl=0);
    int err_msg_size( int ec, int cl=0 );
    int get_err_code(int ii);
    char* get_error_class(int ec);
// debug
    int err_count(  int ec );
    int show_err( int ec);
 private:
  const static ECO_ERROR _errs[];

};

#endif
