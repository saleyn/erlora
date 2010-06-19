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

// eco_fields.h
#ifndef ECO_FIELDS
#define ECO_FIELDS

#include "eco_elements.h"

//////////////////////////////////////////////////////////////////////////////
class eco_field {
  public:
    eco_field(eco_conv *conv, int num, int type);
    eco_field(eco_conv *conv, int num, eco_element *el);
    ~eco_field();
    virtual void decode();
    virtual void set(Environment *env);  // either set ot register from erl to ora
    virtual void get();  // from ora to erl
    virtual void encode();
    void set_stmt(Statement *stmt);
    void set_rs(ResultSet *rs);
    bool update(eco_field* ef);

//  protected:
//    virtual eco_element* create_element(eco_conv *conv, int type) = 0;
    virtual void create_element() = 0;
//    eco_element* create_element_def(eco_conv *conv, int type);
  protected:

    void create_element_def();
    eco_element* create_from_tuple(eco_conv *conv);
    void encode_def();
    void encode_field_num();

    eco_element *_elm;
    int _num;     // in ora row
    int _type;    // from erl to decode the element
    Statement *_stmt;
    ResultSet *_rs;
    eco_conv *_conv;
};

//////////////////////////////////////////////////////////////////////////////
class eco_field_in : public eco_field {
  public:
    eco_field_in(eco_conv *conv, int num, int type);
    eco_field_in(eco_conv *conv, int num, eco_element *el);
//    eco_element* create_element(eco_conv *conv, int type);
    void create_element();
    void decode();
    void set(Environment *env);

};

//////////////////////////////////////////////////////////////////////////////
class eco_field_out : public eco_field {
  public:
    eco_field_out(eco_conv *conv, int num, int type);
    eco_field_out(eco_conv *conv, int num, eco_element *el);
//    eco_element* create_element(eco_conv *conv, int type);
    void create_element();
    void set(Environment *env); // reg !!
    void get();
    void encode();

};

/////////////////////////////////////////////////////////////////////////////
class eco_field_inout : public eco_field {
  public:
    eco_field_inout(eco_conv *conv, int num, int type);
    eco_field_inout(eco_conv *conv, int num, eco_element *el);
  //  eco_element* create_element(eco_conv *conv, int type);
    void create_element();
    void decode();
    void set(Environment *env);
    void get();
    void encode();
};

/////////////////////////////////////////////////////////////////////////////
class eco_field_sql : public eco_field {
  public:
    eco_field_sql(eco_conv *conv, int num, int type);
    eco_field_sql(eco_conv *conv, int num, eco_element *el);
    eco_field_sql(eco_conv *conv, MetaData& md, int num);
//    eco_element* create_element(eco_conv *conv, int type);
//    eco_element* create_element(eco_conv *conv, MetaData& md, int num);
    void create_element();
    void create_element_md(MetaData& md);
    void decode();
    void set(Environment *env);
    void get();
    void encode();
};

#endif
