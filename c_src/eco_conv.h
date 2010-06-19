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
#ifndef _ECO_CONV
#define _ECO_CONV

//// eco_conv.h
// class to convert from C to erlang ( encode ) and from erlang to C ( decode )

#include <string>
#include <fstream>
#include <ei.h>

#include "eco_error.h"

using namespace std;

class eco_conv{
   public:
     eco_conv(eco_err *err, int debug = 0);
     ~eco_conv();

// input/decode
     void get_term(char *term);
     void get_type(int *type);
     int  get_cmd( );
     void get_tuple_header();
     void get_list_header(int *num);
     void get_int(long *num = 0);
     void get_int(int *num=0);
     void get_int(unsigned int *num=0);
     void get_float(double *dd=0);
     void get_atom(char **str=0);
     void get_c_str(char **str=0);
     void get_str(string **str);
     void get_binary(void **bin);
     void get_string(char** str=0);
     int  get_elem(char **str=0);     // debug

// output/encode
     void set_result(char **result, int *size);
     void create_res_header(int ar, char* rep);
     //void create_res_report(char* rep = 0);
     int  create_error_msg(int ec);
     int  encode_error_msg(char* err_cl, int ec, char* err_msg);
     void encode_tuple_header(int num);
     void encode_list_header(int num);
     void add_c_str(char *str);
     void add_string(string& str);
     void add_atom(char* str);
     void add_int(int ii);
     void add_int(unsigned int ii);
     void add_int(long ii);
     void add_float(double dd);

  private:
     int debug_print(int sz, char* str, int num=0);

  private:
// Variables
//input from Erlang
     char *_term;         // input from Erlang
     int _type;           // type
     int _size;           // size
     int _ind;            // index in term
     int _arity;          // number of elements in tuple
     int _ind_max;        // term_size = _ind_max +1
     int _cmd;            // command
     char *_cmd_name;     // command as string ('connect',...)
     char *_str;          // to decode a string
     int _ii;             // to decode an elem

// output to Erlang
     ei_x_buff buff;
     ei_x_buff *_result;

// error handling
     eco_err *_err;

// debug
     int _debug;
     fstream _dbcout;
};

#endif
