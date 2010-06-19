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

#ifndef _ECO_INTERN
#define _ECO_INTERN

#include <string>
#include <fstream>
#include <map>

using namespace std;

#ifndef OCCI_ORACLE
#include <occi.h>
#endif

#include "eco_conv.h"
#include "eco_rows.h"
#include "eco_error.h"

using namespace oracle::occi;

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const;
};

class eco_proc{
   public:
     eco_proc(int debug = 0);
     ~eco_proc();
     int process_term(char *term, char **result, int *size);

   private:
// cmd from erl
     void init_conn( );
     void close_conn( );
     void do_commit( );
     void do_rollback( );
     void execute_sql( );
     void describe_table( );
     void do_select();
     void prepare_plsql();
     void run_plsql();
     void do_free_sql();
     void process_plsql();
     void unknown_command( );
     void process_array();  // to do
// support
     void parse_conn_str(char *curr,
             string*& userName, string*& password, string*& DBname);
     void get_options();
     void bind_params();
     void execute_query();
     void execute_nonquery();
     int  process_query(ResultSet *rs);
     void prepare_stmt();
     void execute_plsql(eco_plsql_row* row);
     void decode_set_stmt(char* QueryID = 0);
     void get_encode_meta_data(char *tab_name);

// Variables
// co - occi
     Environment *_env;
     Connection  *_conn;
     Statement   *_stmt;

// cache ( plsql)
     map<const char*, eco_row*, ltstr> rows;

// options
     bool        _AutoCommitMode;
     int         _max_rows;
     int         _cache_size;

//input from Erlang
     eco_conv *_conv;    // all decode/encode actions

// error handling
     eco_err *_err;

// debug
     int _debug;
     fstream _dbcout;
};

#endif
