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

// eco_elements.h
#ifndef ECO_ELEMENTS
#define ECO_ELEMENTS

#include  <occi.h>
#include "eco_conv.h"

using namespace std;
using namespace oracle::occi;

class eco_element {
  public:
    eco_element(eco_conv *conv, int num);
    virtual ~eco_element();
    virtual void decode_element();
    virtual void encode_element();
    virtual void get_from_oracle(ResultSet *rs);
    virtual void get_from_oracle(Statement *stmt);
    virtual void set_to_oracle(Statement *stmt, Environment *env);
    virtual void reg_to_oracle(Statement *stmt);

/*    void set_reg_direction(bool _should_reg = true);
    bool get_reg_direction();
  */

  protected:
    eco_conv *_conv;
    int _num;      // in ora row
    bool _is_null;
    int _debug;
    fstream _dbcout;
};

class eco_integer: public  eco_element {
  public:
    eco_integer(eco_conv *conv, int num);
    void decode_element();
    void encode_element();
    void get_from_oracle(ResultSet *rs);
    void get_from_oracle(Statement *stmt);
    void set_to_oracle(Statement *stmt, Environment *env);
    void reg_to_oracle(Statement *stmt);

  private:
    long _value;
};

class eco_string : public  eco_element {
  public:
    eco_string(eco_conv *conv, int num, int max_length = 0);
    ~eco_string();
    void decode_element();
    void encode_element();
    void get_from_oracle(ResultSet *rs);
    void get_from_oracle(Statement *stmt);
    void set_to_oracle(Statement *stmt, Environment *env);
    void reg_to_oracle(Statement *stmt);

  private:
    void set_value(const char* ptr);
  private:
    char *_value;
    int _max_length;
};

class eco_date: public  eco_element {
  public:
    eco_date(eco_conv *conv, int num);
    void decode_element();
    void encode_element();
    void get_from_oracle(ResultSet *rs);
    void get_from_oracle(Statement *stmt);
    void set_to_oracle(Statement *stmt, Environment *env);
    void reg_to_oracle(Statement *stmt);

  private:
    int _year;
    unsigned int _month;
    unsigned int _day;
    unsigned int _hour;
    unsigned int _minute;
    unsigned int _second;
};

class eco_float: public  eco_element {
  public:
    eco_float(eco_conv *conv, int num);
    void decode_element();
    void encode_element();
    void get_from_oracle(ResultSet *rs);
    void get_from_oracle(Statement *stmt);
    void set_to_oracle(Statement *stmt, Environment *env);
    void reg_to_oracle(Statement *stmt);

  private:
    double _value;
    int _precision;
    int _scale;
};

class eco_null: public  eco_element {
  public:
    eco_null(eco_conv *conv, int num);
    void decode_element();
    void set_to_oracle(Statement *stmt, Environment *env);

  private:
    Type _tp;
};

#endif
