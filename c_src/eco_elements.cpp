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

// eco_elements.cpp
#include <iostream>
#include "eco_elements.h"
#include "ec_protocol.h"

using namespace std;
using namespace oracle::occi;

#define FREE(ptr) if(ptr) { free(ptr); ptr = NULL;}

extern fstream gcout;


/////////////////////////////////////////////////////////////////////////
// class eco_element
// //////////////////////////////////////////////////////////////////////
eco_element::eco_element(eco_conv *conv, int num)
{
  _conv = conv;
  _num = num;
}

eco_element::~eco_element() {; }

/*
void eco_element::set_reg_direction(bool should_reg)
{
  _should_reg = should_reg;
}

bool eco_element::get_reg_direction()
{
  return _should_reg;
}
*/

//virtual
void eco_element::decode_element(){;}
void eco_element::encode_element(){;}
void eco_element::get_from_oracle(ResultSet *rs){;}
void eco_element::get_from_oracle(Statement *stmt){;}
void eco_element::set_to_oracle(Statement *stmt, Environment *env){;}
void eco_element::reg_to_oracle(Statement *stmt){;}

////////////////////////////////////////////////////////////////////////
//  class eco_string
// /////////////////////////////////////////////////////////////////////
eco_string::eco_string(eco_conv *conv, int num, int max_length)
: eco_element(conv, num)
{
   _value = NULL;
   _max_length = max_length;
}

eco_string::~eco_string()
{
  FREE(_value);
}

void eco_string::get_from_oracle(ResultSet *rs)
{
  const char* ptr = rs->getString(_num).c_str();
  set_value(ptr);
}

void eco_string::get_from_oracle(Statement *stmt)
{
  const char* ptr = stmt->getString(_num).c_str();
  set_value(ptr);
}

void eco_string::set_value(const char* ptr)
{
   if(!ptr){
//     cout << "empty string field number " << _num << endl;
     return;
   }

   FREE(_value);
   int size = strlen(ptr) + 1;
   _value = (char*) calloc(1, size);
   if(!_value){
      throw EX_OCCI | EX_ALLOC;
   }
   strcpy(_value, ptr);
}

void eco_string::encode_element()
{
  if(_value)
    _conv->add_c_str(_value);
  else
    throw EX_OCCI;

}

void eco_string::decode_element()
{
  _conv->get_c_str(&_value);
}

void eco_string::set_to_oracle(Statement *stmt, Environment *env)
{
  stmt->setString(_num, string(_value));
}

void eco_string::reg_to_oracle(Statement *stmt)
{
  stmt->registerOutParam(_num, OCCIINT, _max_length);
}
/////////////////////////////////////////////////////////////////////////
// class eco_integer
// //////////////////////////////////////////////////////////////////////
eco_integer::eco_integer(eco_conv *conv, int num)
: eco_element(conv, num)
{
   _value = 0;
}

void eco_integer::get_from_oracle(ResultSet *rs)
{
  _value = rs->getInt(_num);
}

void eco_integer::get_from_oracle(Statement *stmt)
{
  _value = stmt->getInt(_num);
}

void eco_integer::encode_element()
{
  _conv->add_int(_value);
}

void eco_integer::decode_element()
{
  _conv->get_int(&_value);
}

void eco_integer::set_to_oracle(Statement *stmt, Environment *env)
{
  stmt->setInt(_num, _value);
}

void eco_integer::reg_to_oracle(Statement *stmt)
{
  stmt->registerOutParam(_num, OCCIINT);
}

/////////////////////////////////////////////////////////////////////////
// class eco_date
// //////////////////////////////////////////////////////////////////////
eco_date::eco_date(eco_conv *conv, int num)
: eco_element(conv, num)
{
  _year = _month = _day = _hour = _minute = _second = 0;
}

void eco_date::get_from_oracle(ResultSet *rs)
{
  Date sd = rs->getDate(_num);
  sd.getDate(_year, _month, _day, _hour, _minute, _second);
}

void eco_date::get_from_oracle(Statement *stmt)
{
  Date sd = stmt->getDate(_num);
  sd.getDate(_year, _month, _day, _hour, _minute, _second);
}

void eco_date::encode_element()
{
  _conv->encode_tuple_header(2);
  _conv->encode_tuple_header(3);
  _conv->add_int(_year);
  _conv->add_int(_month);
  _conv->add_int(_day);
  _conv->encode_tuple_header(3);
  _conv->add_int(_hour);
  _conv->add_int(_minute);
  _conv->add_int(_second);
}

void eco_date::decode_element()
{
// BB already done  _conv->get_tuple_header(); // 2
  _conv->get_tuple_header(); // 3
  _conv->get_int(&_year);
  _conv->get_int(&_month);
  _conv->get_int(&_day);
  _conv->get_tuple_header(); // 3
  _conv->get_int(&_hour);
  _conv->get_int(&_minute);
  _conv->get_int(&_second);
}

void eco_date::set_to_oracle(Statement *stmt, Environment *env)
{
  Date sd( env, _year, _month, _day, _hour, _minute, _second);
  stmt->setDate(_num, sd);
}

void eco_date::reg_to_oracle(Statement *stmt)
{
  stmt->registerOutParam(_num, OCCIDATE);
}
///////////////////////////////////////////////////////////////////////
// class eco_float
// ////////////////////////////////////////////////////////////////////
eco_float::eco_float(eco_conv *conv, int num)
 : eco_element(conv, num)
{
   _value = 0;
}

void eco_float::get_from_oracle(ResultSet *rs)
{
  _value = rs->getDouble(_num);
}

void eco_float::get_from_oracle(Statement *stmt)
{
  _value = stmt->getDouble(_num);
}

void eco_float::encode_element()
{
  _conv->add_float(_value);
}

void eco_float::decode_element()
{
  _conv->get_float(&_value);
}

void eco_float::set_to_oracle(Statement *stmt, Environment *env)
{
  stmt->setDouble(_num, _value);
}

void eco_float::reg_to_oracle(Statement *stmt)
{
  stmt->registerOutParam(_num, OCCIFLOAT);
}

////////////////////////////////////////////////////////////////////////
// class eco_null
// /////////////////////////////////////////////////////////////////////
eco_null::eco_null(eco_conv *conv, int num)
 : eco_element(conv, num)
{
}

void eco_null::decode_element()
{
  char  *data = NULL; // to recieve "null"
  char *data_type = NULL;
  _conv->get_atom(&data); // "null"
  _conv->get_atom(&data_type);
  int type_code = COD(data_type[0], data_type[1], data_type[2]);
  FREE(data);
  FREE(data_type);
  switch(type_code)
  {
     case ECO_STR:
	 _tp = OCCISTRING;
	 break;
     case ECO_INT:
	 _tp = OCCIINT;
	 break;
     case ECO_FT:
	 _tp = OCCIFLOAT;
	 break;
     case ECO_DT:
         _tp = OCCIDATE;
	  break;
     default:
         gcout << "unknown type of null found: " << data_type  << endl;
	 throw EX_DEC | EX_UNKN | EX_TYPE;
  }

}
////	OCCINUMBER covered by  OCCIINT || OCCIFLOAT

void eco_null::set_to_oracle(Statement *stmt, Environment *env)
{
  stmt->setNull(_num, _tp);
}
