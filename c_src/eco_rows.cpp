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

// eco_rows.cpp
#include <iostream>
#include "eco_rows.h"
#include "ec_protocol.h"

using namespace std;
using namespace oracle::occi;


#define EL_IN     0x1
#define EL_OUT    0x2
#define EL_INOUT  EL_IN | EL_OUT
#define EL_SQL    0x8

#define FREE(ptr) if(ptr) { free(ptr); ptr = NULL;}

extern fstream gcout;

// Implementation eco_row
eco_row::eco_row(eco_conv *conv, Statement *stmt)
{
  _conv = conv;
  _conn = NULL;
  _stmt = stmt;
  _rs = NULL;
}

eco_row::~eco_row( )
{
  int size = _fields.size();

  if(!size)
    return;

  for (int ii=0; ii < _fields.size(); ii++)
    delete _fields[ii];

  _fields.clear();
}

void eco_row::set_stmt(Statement *stmt)
{
  _stmt = stmt;
}

void eco_row::set_rs(ResultSet *rs)
{
  _rs = rs;
}

void eco_row::get_encode_row_def()
{
  int size = _fields.size();

  if(!size)
    return;

// list header: 1 element:
// given row ( tuple) and all next rows ( another list)
  _conv->encode_list_header(1);

// tuple header: _size elements
   _conv->encode_tuple_header(size);

  for (int ii=0; ii < size; ii++)
  {
    _fields[ii]->get();
    _fields[ii]->encode();
  }
}

void eco_row::set_to_ora(Environment *env)
{
  int size = _fields.size();

  if(!size)
    return;

  for (int ii=0; ii < size; ii++)
    _fields[ii]->set(env);
}

int eco_row::update_field(eco_field* ef)
{
  int size = _fields.size();

  if(!size)
     return -1;

  for (int ii=0; ii < size; ii++)
  {
    if( _fields[ii]->update(ef))
      return ii;
  }

   return -1;
}

void eco_row::add_field(eco_field  *ef)
{
  int ii = update_field(ef);
  if(ii == -1)
    _fields.push_back(ef);
  else
    delete ef;
}

eco_field* eco_row::create_field(int num)
{
  int dir, type;
  eco_field* ef = 0;

  dir = get_param_prop(&num, &type);

  switch(dir)
  {
    case EL_IN:
      ef = new eco_field_in(_conv, num, type);
      break;
    case EL_OUT:
      ef = new eco_field_out(_conv, num, type);
      break;
    case EL_INOUT:
      ef = new eco_field_inout(_conv, num, type);
      break;
    case EL_SQL:
      ef =  new eco_field_sql(_conv, num, type);
      break;
  }
  ef->set_stmt(_stmt);
  ef->create_element();
  return ef;
}

void eco_row::decode_row()
{
  int size;
  _conv->get_list_header(&size);
  if(!size)
    return;

  eco_field *ef;

  for( int ii = 0; ii < size; ii++)
  {
    ef = create_field(ii);
    ef->decode();
    add_field(ef);
  }
}

//////////////////////////////////////////////////////////////////
//// class eco_sql_row
//// ///////////////////////////////////////////////////////////////
eco_sql_row::eco_sql_row(eco_conv *conv, Statement *stmt) : eco_row (conv, stmt)  { }

eco_sql_row::eco_sql_row(eco_conv *conv) : eco_row (conv, NULL)  { }

void eco_sql_row::init_row(ResultSet *rs )
{
// create row as vector of elements // types ( both ora and erl ) to take value from ora and to encode its names
   _rs = rs;
  vector<MetaData>listOfColumns;
  listOfColumns = _rs->getColumnListMetaData();
  int size = listOfColumns.size();
  if(!size)
// BB throw ??
     return;

  eco_field_sql* ef;
  _conv->encode_list_header(size);
  char* ptr = NULL;
  for (int ii=0; ii < size; ii++)
  {
// create the field of corresponding type
     ef = create_field(_conv, listOfColumns[ii], ii+1);
     ef->set_rs(_rs);
     _fields.push_back(ef);
     ptr = (char*)listOfColumns[ii].getString(MetaData::ATTR_NAME).c_str();
// add field name to title
     _conv->add_c_str(ptr);
  }
   // encode empty list to complete
  _conv->encode_list_header(0);
}

eco_field_sql* eco_sql_row::create_field(eco_conv *conv, MetaData& md, int num)
{
  eco_field_sql* efs = new eco_field_sql(_conv, md, num);
  efs->create_element_md(md);
  return efs;
}

int eco_sql_row::get_param_prop(int* num, int* type)
{
  *num++;
  _conv->get_type(type);
  return  EL_SQL;
}

void eco_sql_row::get_encode_row()
{
  get_encode_row_def();
}
//////////////////////////////////////////////////////////////////////////////
// class eco_plsql_row
// ///////////////////////////////////////////////////////////////////////////
eco_plsql_row::eco_plsql_row(eco_conv *conv, Statement *stmt) : eco_row (conv, stmt) { }

int eco_plsql_row::get_param_prop(int* num, int* type)
{
//out: num, type
  int dir = 0;
  char* direction = NULL;

  _conv->get_tuple_header();
  _conv->get_atom(&direction);
  _conv->get_int(num);
  _conv->get_type(type);

  if( direction[0] == 'i' )
    dir = EL_IN;
  else
    dir = EL_OUT;

  if( strlen(direction) > 2)
    dir |= EL_OUT;

  FREE(direction);

  return dir;
}

void eco_plsql_row::get_encode_row()
{
  get_encode_row_def();
  // encode empty list to complete
  _conv->encode_list_header(0);
}
