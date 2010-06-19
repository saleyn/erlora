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

// eco_fields.cpp
#include <iostream>
#include "eco_fields.h"
#include "ec_protocol.h"

using namespace std;
using namespace oracle::occi;

#define FREE(ptr) if(ptr) { free(ptr); ptr = NULL;}

extern fstream gcout;

/////////////////////////////////////////////////////////////////////////
//// class eco_field
//// //////////////////////////////////////////////////////////////////////
eco_field::eco_field(eco_conv *conv, int num, int type)
{
  _conv = conv;
  _num = num;
  _elm = 0;   //create_element(conv, type);
  _type = type;
  _stmt = 0;
  _rs = 0;
}

eco_field::eco_field(eco_conv *conv, int num, eco_element *el)
{
  _conv = conv;
  _num = num;
  _elm = el;
  _stmt = 0;
  _rs = 0;
}

eco_field::~eco_field()
{
  if(_elm)
    delete _elm;
}

void eco_field::set_stmt(Statement *stmt)
{
  _stmt = stmt;
}

void eco_field::set_rs(ResultSet *rs)
{
  _rs = rs;
}

bool eco_field::update(eco_field* ef)
{
  if( _num != ef->_num)
    return false;

  //update
  _stmt = ef->_stmt;
  _rs = ef->_rs;

  // replace elem
  delete _elm;       // delete old
  _elm = ef->_elm;   // attach new
  ef->_elm = NULL;   // unlink new from ef ( which going to be deleted!)

  return true;
}

void eco_field::encode_def()
{
  encode_field_num();
  _elm->encode_element();
}

void eco_field::encode_field_num()
{
  _conv->encode_tuple_header(2);
  _conv->add_int(_num);
}


//eco_element* eco_field::create_element_def(eco_conv *conv, int type)
void eco_field::create_element_def()
{// from Value of type, to be used in: in, inout, sql

  switch(_type)
  {
     case ERL_STRING_EXT:
     case ERL_ATOM_EXT:
     case ERL_BINARY_EXT:
       _elm = new eco_string(_conv, _num);
       break;
     case ERL_INTEGER_EXT:
     case ERL_SMALL_INTEGER_EXT:
       _elm = new eco_integer(_conv, _num);
       break;
     case ERL_SMALL_TUPLE_EXT:
     case ERL_LARGE_TUPLE_EXT:
       _elm = create_from_tuple(_conv);
       break;
     case ERL_FLOAT_EXT:
       _elm = new eco_float(_conv, _num);
       break;

     default:
       // dump error info and throw exception
       ;
  }
}

eco_element* eco_field::create_from_tuple(eco_conv *conv)
{
 int type; // inside tuple
 // tuple means: it could be either null: {null, }
 //                          or date: { { , , },  { , , } }
   _conv->get_tuple_header(); // 2
   _conv->get_type(&type);

   if( type == ERL_ATOM_EXT) //  atom => null :  {null, }
       return new eco_null(conv, _num);
   else    // == tuple=> date { , , },  { , , }
       return new eco_date(conv, _num);
}

//virtual
void eco_field::decode(){}
void eco_field::set(Environment *env){}
void eco_field::get(){}
void eco_field::encode(){}

/////////////////////////////////////////////////////////////////////////
////// class eco_field_in
////// //////////////////////////////////////////////////////////////////
eco_field_in::eco_field_in(eco_conv *conv, int num, int type) :  eco_field(conv, num, type) {}

eco_field_in::eco_field_in(eco_conv *conv, int num, eco_element *el) : eco_field(conv, num, el) {}

//eco_element* eco_field_in::create_element(eco_conv *conv, int erl_type)
void eco_field_in::create_element()
{
  return create_element_def();
}

void eco_field_in::decode()
{
  _elm->decode_element();
}

void eco_field_in::set(Environment *env)
{
  _elm->set_to_oracle(_stmt, env);
}

/////////////////////////////////////////////////////////////////////////
////////  class eco_field_out
//////// ////////////////////////////////////////////////////////////////
eco_field_out::eco_field_out(eco_conv *conv, int num, int type) : eco_field(conv, num, type) {}
eco_field_out::eco_field_out(eco_conv *conv, int num, eco_element *el) : eco_field(conv, num, el) {}
//eco_element* eco_field_out::create_element(eco_conv *conv, int erl_type)
void eco_field_out::create_element()
{// from Type : out
  char *_data_type = NULL;
  if (_type == ERL_SMALL_TUPLE_EXT) // string
     _conv->get_tuple_header();

  _conv->get_atom(&_data_type);
  int type_code = COD(_data_type[0], _data_type[1], _data_type[2]);
  FREE(_data_type);

  int type;
  int max_length;

  switch(type_code)
  {
    case ECO_STR:
      _conv->get_int(&max_length);
      _elm = new eco_string(_conv, _num, max_length);
      break;

    case ECO_INT:
      _elm = new eco_integer(_conv, _num);
      break;

    case ECO_FT:
      _elm = new eco_float(_conv, _num);
      break;

    case ECO_DT:
      _elm =  new eco_date(_conv, _num);
      break;

    default:
      ;          // BB raise errror
  }
}

void eco_field_out::set(Environment *env)
{
  _elm->reg_to_oracle(_stmt);
}

void eco_field_out::get()
{
  _elm->get_from_oracle(_stmt);
}

void eco_field_out::encode()
{
  encode_def();
}

////////////////////////////////////////////////////////////////////////////
//////////  class eco_field_inout
////////// //////////////////////////////////////////////////////////////////
eco_field_inout::eco_field_inout(eco_conv *conv, int num, int type) : eco_field(conv, num, type) {}
eco_field_inout::eco_field_inout(eco_conv *conv, int num, eco_element *el) : eco_field(conv, num, el) {}

//eco_element* eco_field_inout::create_element(eco_conv *conv, int erl_type)
void eco_field_inout::create_element()
{
  create_element_def();
}

void eco_field_inout::decode()
{
  _elm->decode_element();
}

void eco_field_inout::set(Environment *env)
{
  _elm->set_to_oracle(_stmt, env);
}

void eco_field_inout::get()
{
  _elm->get_from_oracle(_stmt);
}

void eco_field_inout::encode()
{
  encode_def();
}

/////////////////////////////////////////////////////////////////////////
////////////  class eco_field_sql
/////////////////////////////////////////////////////////////////////////
eco_field_sql::eco_field_sql(eco_conv *conv, int num, int type)  : eco_field(conv, num, type) {}

eco_field_sql::eco_field_sql(eco_conv *conv, MetaData& md, int num) : eco_field(conv, num, 0) {}

eco_field_sql::eco_field_sql(eco_conv *conv, int num, eco_element *el) : eco_field(conv, num, el) {}

//eco_element* eco_field_sql::create_element(eco_conv *conv, int erl_type)
void eco_field_sql::create_element()
{
  create_element_def();
}

void eco_field_sql::create_element_md(MetaData& md)
{
  int type = md.getInt(MetaData::ATTR_DATA_TYPE);
  int scale = 0;
  int precision = 0;

  switch(type)
  {
    case OCCI_SQLT_CHR:
    case OCCI_SQLT_LNG:
    case OCCI_SQLT_VCS:
    case OCCI_SQLT_AFC:
    case OCCI_SQLT_STR:
      _elm = new eco_string(_conv, _num);
      break;

    case OCCI_SQLT_NUM:
      scale = md.getInt(MetaData::ATTR_SCALE);
      precision = md.getInt(MetaData::ATTR_PRECISION);
      if ( scale > 0 || (scale == -127 && precision == 0))
         _elm = new eco_float(_conv, _num);
      else
         _elm = new eco_integer(_conv, _num);
      break;

    case OCCIINT:
      _elm = new eco_integer(_conv, _num);
      break;

    case OCCI_SQLT_DAT:
      _elm = new eco_date(_conv, _num);
      break;

    case OCCIFLOAT:
    case OCCIDOUBLE:
      _elm =  new eco_float(_conv, _num);
      break;

    default:
 ;  // dump error info and throw exception
  //BB ??   cout << "Unknown type= " << type << endl;
 }
}

void eco_field_sql::decode()
{
  _elm->decode_element();
}

void eco_field_sql::set(Environment *env)
{
  _elm->set_to_oracle(_stmt, env);
}

void eco_field_sql::get()
{
  _elm->get_from_oracle(_rs);
}

void eco_field_sql::encode()
{
  _elm->encode_element();
}
