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

//// eco_conv.cpp
// class to convert from C to erlang ( encode ) and from erlang to C ( decode )

#include <stdlib.h>
#include <fstream>
#include <erl_interface.h>
#include "ec_protocol.h"
#include "eco_conv.h"
#include "eco_error.h"

#define FREE(ptr) if(ptr) { free(ptr); ptr = NULL;}

extern fstream gcout;
// IMPLEMENTATION class eco_conv
//   PUBLIC:
/////////////////////////////////////////////////////////////////////////////
// constructor
eco_conv::eco_conv(eco_err *err, int debug)
{
  _err = err;
  _str = 0;
  _cmd_name = 0;
  _result = &buff;
  _result->buff = 0;
  _result->buffsz = 0;
  if(debug){
    _debug = 1;
    _dbcout.open("dbconv.log", fstream::out);
    _dbcout << "eco_conv created " << endl;
  }
}

//destructor
eco_conv::~eco_conv()
{
  if(_debug)
    _dbcout.close();
}

///////////////////////////////////////////////////////////////////////////
// input/decode
//
void eco_conv::get_term(char *buf)
{
  int version;
  _term = buf;
  _ind = 0;

  if(_debug){
    _dbcout << "eco_conv input: " << endl;
    _dbcout << buf << endl;
  }

  if( ei_decode_version(_term, &_ind, &version))
    throw EX_TERM;

}

void eco_conv::get_type(int *type)
{
  if(ei_get_type(_term, &_ind, type, &_size))
     throw EX_TYPE;
}

int eco_conv::get_cmd( )
{
  int rc;
// first determine whether this is atom or tuple
  try{
    if(ei_get_type(_term, &_ind, &_type, &_size))
     throw EX_CMD | EX_TYPE;

    if (_type == ERL_ATOM_EXT)
       _arity = 1;
    else {
// if tuple decode the header
      get_tuple_header();
   }

// decode command itself
 //debug_print(20, _term);
  rc = get_elem(&_cmd_name);
  if(!rc)
    _cmd = 0; // _ii
  else
    _cmd = COD(_cmd_name[0], _cmd_name[1], _cmd_name[2]);
  }
  catch(int ec)
  {
    if(_debug) {
      _dbcout << "error " << hex << ec << dec << " found during decoding command" << endl;
    }
    throw ec | EX_CMD;
  }
  _arity--;

 // _cmd = COD(_cmd_name[0], _cmd_name[1], _cmd_name[2]);
  // using  macro COD defined in ec_protocol.h
  // enforces consistency
  // debug
  if(_debug) {
    _dbcout << "command = " << _cmd_name << endl;
    _dbcout << "cmd = "     << _cmd      << endl;
    _dbcout << "arity = "   << _arity    << endl;
  }
  FREE(_cmd_name);
  return _cmd;
}

int eco_conv::get_elem(char **str )
{
// string or atom
 int rc =0;

 if(ei_get_type(_term, &_ind, &_type, &_size))
   throw  EX_TYPE;

 if(_debug)
    _dbcout << "get_elem: type= " << _type << " , size= " << _size << endl;
 debug_print(_size, _term+_ind);
 switch(_type){
   case ERL_ATOM_EXT:
     get_atom(str);
     if(_debug) {
        _dbcout << "atom arg = " ;
     }
     rc = 2;
     break;

   case ERL_STRING_EXT:
      get_c_str(str);
      if(_debug) {
        _dbcout << "str arg = " ;
      }
      rc = 1;
      break;
   default:
      rc = 0;
 }
 if(rc && _debug) {
   if(str)
      _dbcout << str << endl;
   else
      _dbcout << _str << endl;
 }
 return rc;
}

void eco_conv::get_tuple_header()
{
  if(ei_get_type(_term, &_ind, &_type, &_size) ||
    !(_type == ERL_SMALL_TUPLE_EXT || _type == ERL_LARGE_TUPLE_EXT))
     throw  EX_TYPE | EX_HEADER;

  if(ei_decode_tuple_header(_term, &_ind, &_arity))
         throw  EX_HEADER;
  if(_debug)
     _dbcout << "get_tuple_header: arity= " << _arity << endl;
}

void eco_conv::get_list_header(int *num)
{
  if(ei_get_type(_term, &_ind, &_type, &_size))
    throw  EX_TYPE;

  if (_type == ERL_NIL_EXT){
   // list is empty
    *num = 0;
    if(_debug) _dbcout << "list is empty" << endl;
    return;
  }

  if( _type != ERL_LIST_EXT) {
    if(_debug) _dbcout << "type found " << _type << " , expected " << ERL_LIST_EXT << endl;
     throw  EX_TYPE | EX_LIST;
}
  if(ei_decode_list_header(_term, &_ind, num))
     throw EX_LIST;
  if(_debug) _dbcout << "list contains " << *num << " elements" << endl;
}

void eco_conv::get_int(long *num)
{
  long lnum;

  if(ei_get_type(_term, &_ind, &_type, &_size)
     || !( _type == ERL_INTEGER_EXT || _type == ERL_SMALL_INTEGER_EXT))
    throw  EX_INT | EX_TYPE;

  if ( ei_decode_long(_term, &_ind, &lnum))
    throw EX_INT;

   if(num)
       *num = lnum;
   else
       _ii =  lnum;
}

void eco_conv::get_int(int *num)
{
  get_int((long *) num);
}

void eco_conv::get_int(unsigned int *num)
{
   get_int((long *) num);
}

void eco_conv::get_float(double *dd)
{
  if(ei_get_type(_term, &_ind, &_type, &_size)
     || _type != ERL_FLOAT_EXT)
	  throw  EX_FLOAT || EX_TYPE;

  if( ei_decode_double(_term, &_ind, dd))
       throw  EX_FLOAT | EX_DEC;
}

void eco_conv::get_atom(char **str)
{
  char *curr;

  if( ei_get_type(_term, &_ind, &_type, &_size)
      || _type != ERL_ATOM_EXT)
    throw EX_TYPE | EX_ATOM;

  if(str)
    curr = *str;
  else
    curr = _str;

  FREE(curr);

  curr = (char *) calloc(1, _size+1);
  if (!curr)
     throw EX_ATOM | EX_ALLOC;

  if(str)
     *str = curr;
  else
     _str = curr;

  if( ei_decode_atom(_term, &_ind, curr))
      throw EX_ATOM;

  _dbcout << " get_atom :" << curr << endl;
}

void eco_conv::get_c_str(char **str)
{
  char *curr;
  //_dbcout << " get_c_str" << endl;

  if(ei_get_type(_term, &_ind, &_type, &_size)
     || _type != ERL_STRING_EXT)
    throw  EX_STR | EX_TYPE;

  //_dbcout << "type= " << _type << " , size = " << _size << endl;

  if(str)
    curr = *str;
  else
    curr = _str;

  FREE(curr);

  curr = (char *) calloc(1, _size+1);
  if ( !curr)
    throw EX_STR | EX_ALLOC;

  if(str)
    *str = curr;
  else
   _str = curr;

  if(ei_decode_string(_term, &_ind, curr))
       throw  EX_STR;

  _dbcout << " get_c_str :" << curr << endl;
}

void eco_conv::get_str(string **strg)
{
  get_c_str( );

// delete 'old' string if any
  if ( *strg)
    delete *strg;

  *strg = new string(_str);  // catch exception consistently !!
  FREE(_str);
}

void eco_conv::get_string(char **str)
{
// from any: string, atom or binary
  if(ei_get_type(_term, &_ind, &_type, &_size))
    throw  EX_STR | EX_TYPE;

  switch(_type)
  {
     case ERL_ATOM_EXT:
       get_atom(str);
       break;
     case ERL_STRING_EXT:
       get_c_str(str);
       break;
     case ERL_BINARY_EXT:
       get_binary((void**) str);
       break;
     deafult:
       throw  EX_STR | EX_TYPE;
  }

}

void eco_conv::get_binary(void **bin)
{
 long len;

 *bin = NULL;

 if(ei_get_type(_term, &_ind, &_type, &_size)
    || _type != ERL_BINARY_EXT)
   throw  EX_BIN | EX_TYPE;

  *bin = malloc(_size);
  if ( !*bin)
     throw  EX_ALLOC;

  if(ei_decode_binary(_term, &_ind, bin, &len))
     throw  EX_BIN;

  if ( len > _size)
     throw  EX_SZ | EX_BIN;
}

////////////////////////////////////////////////////////////////////////////
// output/encode
//

void eco_conv::set_result(char **result, int *size)
{
  *result = _result->buff;
  *size = _result->index;  // not buffsz;

  if(_debug){
    _dbcout << "eco_conv output: " << endl;
    _dbcout << "size=" << *size << endl;
     debug_print(*size, *result, 3);
  }
}

void eco_conv::create_res_header(int ar, char* rep)
{
  int rc = 0;

  if(_debug){
    _dbcout << "create_res_header: ar=" << ar ;
    if(rep) _dbcout << " , report: " << rep;
    _dbcout << endl;
  }

  try{
    if( _result->buffsz)
      if (ei_x_free(_result))
        throw EX_ERL_MEM;

    if ( ei_x_new_with_version(_result))
      throw EX_ERL_MEM;

    if( ar > 1)
      if ( ei_x_encode_tuple_header(_result, ar))
        throw EX_TUPLE;

    rc = ei_x_encode_atom(_result, rep);

    if(_debug){
       _dbcout << "create_res_header: rc= " << rc << endl;
       debug_print(_result->index, _result->buff, 1);
    }

   if(rc)
     throw  EX_ATOM;
   }
   catch(int ec)
   {
     throw  ec | EX_ENC;
   }
}

int eco_conv::create_error_msg(int ec)
{
  int rc =0;

  if(ec & EX_DEC)
  {
    if(ec & EX_TYPE)
      debug_print(4, _term+_ind, 100);
    else
      debug_print(_size, _term+_ind, 100);
  }

  gcout << "error found, ec= " << hex << ec << dec << endl;

  char* err_cl = _err->get_error_class( ec);
  if(!err_cl) {
    rc++;
    gcout << "could not get err_class for ec= " << hex << ec << dec << endl;
  }
  else {
    gcout << "error class " << err_cl << endl;
  }

  char* msg = _err->get_error_msg( ec);
  if(!msg){
    rc++;
    gcout << "could not get err_description for ec= " << hex << ec << dec << endl;
  }
  else {
    gcout << "error description: " << msg << endl;
  }

  rc += encode_error_msg(err_cl, ec, msg);
  FREE(err_cl);
  FREE(msg);

  return rc;

}

int eco_conv::encode_error_msg(char *err_cl, int ec, char *msg)
{
  int rc = 0;
  try {
    create_res_header(2, "error");
  }
  catch( int h_ec)
  {
    gcout << "could not create tuple header for result" << endl;
    rc++;
  }

  if(rc)
    return rc;

  if ( ei_x_encode_tuple_header(_result, 3))
  {
    gcout << "could not create tuple header for error" << endl;
    return 1;
  }

  if( err_cl && ei_x_encode_atom(_result, err_cl)){
    gcout << "could not encode error class " << err_cl << endl;
    return 1;
  }

  if( ei_x_encode_long(_result, ec)){
     gcout << "could not encode ec= " << ec << endl;
     return 1;
  }

  if( msg && ei_x_encode_string(_result, msg)){
     gcout << "could not create tuple header for err_description for ec= "
     << hex << ec << dec << endl;
     return 1;
  }

  return rc;
}

void eco_conv::encode_tuple_header(int num)
{
  if (ei_x_encode_tuple_header(_result, num))
    throw  EX_TUPLE | EX_ENC;
}

void eco_conv::encode_list_header(int num)
{
  int rc =0;

  if(num)
     rc = ei_x_encode_list_header(_result, num);
  else
    rc = ei_x_encode_empty_list(_result);

  if(rc)
     throw EX_LIST | EX_ENC;
}

void eco_conv::add_c_str(char *str)
{
  if( ei_x_encode_string(_result, str))
    throw  EX_STR | EX_ENC;
}

void eco_conv::add_string(string& str)
{
   char *cstr = (char*) str.c_str();
   add_c_str(cstr);
}

void eco_conv::add_atom(char *str)
{
   if(ei_x_encode_atom(_result, str))
     throw  EX_ATOM | EX_ENC;
}

void eco_conv::add_int(int ii)
{
  if( ei_x_encode_long(_result, ii))
   throw  EX_INT | EX_ENC;
}

void eco_conv::add_int(unsigned int ii)
{
  if( ei_x_encode_long(_result, ii))
     throw  EX_INT | EX_ENC;
}

void eco_conv::add_int(long ii)
{
  if( ei_x_encode_long(_result, ii))
     throw  EX_INT | EX_ENC;
}

void eco_conv::add_float(double dd)
{
  if( ei_x_encode_double(_result, dd))
     throw  EX_FLOAT | EX_ENC;
}

int eco_conv::debug_print(int sz, char* str, int num)
{
  if(!_debug)
    return 0;

  int ii;
  _dbcout << "buff num: " << num << ", size of " << sz << endl;
  for(ii=0; ii < sz; ii++)
  {
    _dbcout << (int)str[ii] << '"' << str[ii] << '"';
    if(ii < sz - 1)
       _dbcout << ", ";
    else
       _dbcout << endl;
  }

  _dbcout << " == end of buff: " << num << endl;
  return ii;
}
