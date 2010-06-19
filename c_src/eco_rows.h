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

// eco_rows.h
#ifndef ECO_ROWS
#define ECO_ROWS

#include  <occi.h>
#include "eco_conv.h"
#include "eco_fields.h"

using namespace std;
using namespace oracle::occi;

//////////////////////////////////////////////////////////////////////////////
class eco_row {
  public:
     eco_row(eco_conv *conv, Statement *stmt);
    ~eco_row();

// delete and create    virtual void reset_row();  // for sql clean up and create out put types
                               // init_row()
    void decode_row();    // find dir, num, type;
                          // create element and field(dir)
                          // decode element (Value | Type)(dir)
                          // add_field ( or update if found)
    void set_to_ora(Environment *env); // call set on each field
                                    // to call set_to_oracle or reg_to_oracle
    virtual void get_encode_row() = 0;  // call get_encode on each field

    void set_stmt(Statement *stmt);
    void set_rs(ResultSet *rs);

  protected:
    void get_encode_row_def();
    int  virtual get_param_prop(int* num, int* type) = 0;
    int  get_param_num();
    eco_field* create_field(int num);
    void add_field(eco_field *ef);
    int update_field(eco_field *ef);

  protected:
    Connection  *_conn;  // oracle
    Statement   *_stmt;
    ResultSet *_rs;
    vector <eco_field*> _fields;

    eco_conv *_conv;    // all decode/encode actions

    int _debug;
    fstream _dbcout;
};

class eco_sql_row : public  eco_row {
// sql param in
  public:
    eco_sql_row(eco_conv *conv, Statement *stmt); //  = NULL );  // in
    eco_sql_row(eco_conv *conv);
    void init_row(ResultSet *rs);
    void get_encode_row();

  protected:
    int get_param_prop(int* num, int* type); // get type
                                             // return EL_SQL
    eco_field_sql* create_field(eco_conv *conv, MetaData& md, int num);

  private:
};

class eco_plsql_row : public  eco_row {
// plsql param in, out, inout
  public:
   // eco_plsql_row(eco_conv *conv, Statement *stmt = NULL);
    eco_plsql_row(eco_conv *conv, Statement *stmt);
    void get_encode_row();

  protected:
    int get_param_prop(int* num, int* type); // get tuple header
                                             // get type
                                             // get dir, get num

  private:

};
#endif
