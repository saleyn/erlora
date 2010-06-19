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

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <map>

#include <erl_interface.h>
#include "ec_protocol.h"
#include "eco_proc.h"
#include "eco_error.h"

#define OK 0
#define NOTOK 1

#define MAX_ROW_SIZE 4096
#define FREE(ptr) if(ptr) { free(ptr); ptr = NULL;}
#define DBF if(_debug) _dbcout << __FUNCTION__ << ": " << endl;
#define DBP(aaa) if(_debug) _dbcout << __FUNCTION__ << ": " << (aaa) << endl;

using namespace std;

extern fstream gcout;

/////////////////////////////////////////////////////////////////////////////
//  General section

bool ltstr::operator()(const char* s1, const char* s2) const
{
  return strcmp(s1, s2) < 0;
}

/////////////////////////////////////////////////////////////////////////////
// IMPLEMENTATION class eco_proc
//   PUBLIC:
/////////////////////////////////////////////////////////////////////////////
// constructor
eco_proc::eco_proc(int debug)
{
  _debug =  debug;
  _err = new eco_err();
  _conv = new eco_conv(_err, _debug);
  _env = Environment::createEnvironment();
  _conn = NULL;
  _stmt = NULL;
  _AutoCommitMode = false;  // default
  _max_rows = 100;          // default: not restricted
  _cache_size = 10;         // default  BB ??

  //open debug output file
  if(_debug){
     _dbcout.open("dbproc.log", fstream::out);
//     _dbcout << "eco_proc object created" << endl;
   DBP("eco_proc object created")
  }
}

//destructor
eco_proc::~eco_proc()
{
  delete _err;
  delete _conv;

  // destroy all elements first
  map<const char*, eco_row*, ltstr>::const_iterator cur;
  for(cur  = rows.begin(); cur != rows.end(); cur++)
  {
     if(cur->second)
       delete  cur->second;
  }
  rows.clear();

  if(_conn && _stmt)
    _conn->terminateStatement(_stmt);

  _env->terminateConnection(_conn);

  Environment::terminateEnvironment(_env);

  if(_debug){
     _dbcout << "eco_proc object destroyed" << endl;
     _dbcout.close();
  }
}

int eco_proc::process_term(char *term, char **result, int* size)
{
  int ret = 0;

  try{
    _conv->get_term(term);
  // Get the first element in the tuple.  It contains the function identifier
    int cmd = _conv->get_cmd( );
    switch (cmd) {
      case CONNECT:
	    init_conn();
	    break;
      case DISCONNECT:
	    close_conn();
	    break;
      case COMMIT:
	    do_commit();
	    break;
      case EXECUTE_SQL:
	    execute_sql();
	    break;
      case SELECT_SQL:
            do_select();
            break;
      case DESCRIBE:
            describe_table();
            break;
      case ROLLBACK:
            do_rollback();
	    break;
      case RUN_PLSQL:
            run_plsql();
	    break;
      case PREPARE_PLSQL:
            prepare_plsql();
	    break;
      case PROCESS_PLSQL:
            process_plsql();
	    break;
      case FREE_SQL:
	    do_free_sql();
	    break;
      case ARRAY_PLSQL:
	    process_array();
	    break;
      default:
	unknown_command( );
    }  // end switch
  }    // end try

  catch (SQLException &sqlExcp)
  {
    gcout << "ora error found" << endl;
    gcout << sqlExcp.what() << endl;

// create ORA error result
    if(!_conv->encode_error_msg("ora", sqlExcp.getErrorCode(), (char*) sqlExcp.getMessage().c_str()))
      _conv->set_result(result, size);
    return NOTOK;
  }

  catch ( int ec)
  {
// create error result
    if(_debug) {
      _dbcout << "error found: ec= " << hex << ec << dec << endl;
    }


    if(!_conv->create_error_msg(ec))
      _conv->set_result(result, size);
    return NOTOK;
  }

  catch (exception &excp)
  {
    gcout << "occi error found" << endl;
    gcout << excp.what() << endl;


    // create OCCI error result
     if(!_conv->encode_error_msg("occi", EX_UNKN, (char*) excp.what()))
        _conv->set_result(result, size);
     return NOTOK;
  }

 // ok result has been already created
  _conv->set_result(result, size);

  if(_debug) {
    _dbcout << "process has been finished, res size= " << *size << endl;
    _dbcout << "result=" << *result << endl;
  }

  return OK;
}

////////////////////////////////////////////////////////////////////////////////////
//  PRIVATE
////////////////////////////////////////////////////////////////////////////////////
// CMDs
// //////
void eco_proc::init_conn( )
{
//	connect, ConnectStr, [Options]}
//  ConnectStr = "userName/password@DBname";
  char* connStr    = NULL;
  string *userName = NULL;
  string *password = NULL;
  string *DBname   = NULL;

// fetch args and options
  try {
    _conv->get_c_str(&connStr);
    if(_debug)
        _dbcout << connStr << endl;
    parse_conn_str(connStr, userName, password, DBname);
    FREE(connStr);
    if(_debug)
      _dbcout << "init_conn( ):" << " userName=";
      if(userName) _dbcout << *userName;
      _dbcout << ", password=";
      if(password) _dbcout << *password;
      _dbcout << ", connectString=" ;
      if(DBname) _dbcout << *DBname;
      _dbcout << endl;

    get_options();

    if(_debug)
      _dbcout << "init_conn( ):" << ", AutoCommitMode=" << _AutoCommitMode
        << ", max_rows= " << _max_rows
        << endl;
  }
  catch(int ec)
  {
    if(_debug)
      _dbcout << "init_conn( ): error " << hex << ec << dec
              << " found while fetching arguments and options" << endl;
    throw ec | EX_DEC;
  }

  _conn = _env->createConnection(*userName, *password, *DBname);

  if(_cache_size)
	_conn->setStmtCacheSize(_cache_size);

  _conv->create_res_header(1, "ok");

  DBP("connection has been set up ")

  delete userName;
  delete password;
  delete DBname;
}

void eco_proc::close_conn( )
{
//disconnect
  DBP("close_conn")

  if(_conn && _stmt)
      _conn->terminateStatement(_stmt);

  _stmt = NULL;

  if(_conn )
    _env->terminateConnection(_conn);

  _conn = NULL;

  _conv->create_res_header(1, "ok");
  DBP("disconnected successfully");
}

void eco_proc::do_commit( )
{
//commit
  _conn->commit();
  _conv->create_res_header(1, "ok");
  DBF
}

void eco_proc::do_rollback()
{
//rollback
  _conn->rollback();
  _conv->create_res_header(1, "ok");
  DBF
}

void eco_proc::execute_sql()
{
//{execute_sql, QueryID, Query, BindVars}
  DBF
  prepare_stmt();
  execute_nonquery();
}

void eco_proc::do_select()
{
//{select_sql, QueryID, Query, BindVars}
  DBF
  prepare_stmt();
  execute_query();
}

void eco_proc::describe_table()
{
//{describe_table, Table}
// take tab_name as a C string
  DBF
  char *tab_name = NULL;

  _conv->get_c_str(&tab_name);
  _conv->create_res_header(2, "ok");
  get_encode_meta_data(tab_name);
  FREE(tab_name);
}

void eco_proc::prepare_plsql()
{
// decode QueryID
  DBF
  char* QueryID = NULL;
  _conv->get_elem(&QueryID);

  decode_set_stmt(QueryID);
//BB terminate => send to cache and return
  _conn->terminateStatement(_stmt, QueryID);

// decode params and  create plsql row
  eco_plsql_row* row = new eco_plsql_row(_conv, _stmt);
  row->decode_row();
//     row->set_to_ora(_env);
// create an entry in cache
  rows[QueryID] = row;
  FREE(QueryID);

  _conv->create_res_header(1, "ok");
}

void eco_proc::run_plsql()
{
// decode QueryID
  DBF
  char* QueryID = NULL;
  _conv->get_elem(&QueryID);

  //retrive statement
  _stmt = _conn->createStatement("", string(QueryID));
  DBP("Statement has been retrived")

  eco_plsql_row* row;
  if ( rows.find(QueryID) != rows.end())
  {
  //  retrive row
  //  decode in params and bind param values if any
    row = (eco_plsql_row*) rows.find(QueryID)->second;
  // reset stmt for safety
    row->set_stmt(_stmt);
  }
  else
    row = new eco_plsql_row(_conv, _stmt);

  // decode/override in, out and inout params if any
  row->decode_row();

  // bind in, inout and register out params
  row->set_to_ora(_env);

  execute_plsql(row);
  //BB terminate => send to cache and return
  _conn->terminateStatement(_stmt, QueryID);
  FREE(QueryID);
}

void eco_proc::process_plsql()
{
  DBF
  decode_set_stmt();
  eco_plsql_row* row = new eco_plsql_row(_conv, _stmt);

  // decode/override in, out and inout params if any
  row->decode_row();

  // bind in, inout and register out params
  row->set_to_ora(_env);

  execute_plsql(row);

  delete row;
}

void eco_proc::process_array()
{
  /* to do */
	DBF
  // decode statement
  // set up total_rows to process
  //        field types
  // create arrays of correcponding data types
  //        ?? decode_set_stmt();
  // bind to ora : setDataBuffer(
  // loop ( total_rows)
  // decode row by row and fill in arrays;
 /*
  *  _stmt->executeArrayUpdate(total_rows);
  *  _conn->commit();
  *  _conn->terminateStatement(_stmt);
  */
}

void eco_proc::do_free_sql()
{
// decode QueryID
  DBF
  char* QueryID = NULL;
  _conv->get_elem(&QueryID);

// destroy the row and the entry
  map<const char*, eco_row*, ltstr>::const_iterator cur = rows.find(QueryID);
  if(cur->second)
    delete  cur->second;
  rows.erase(QueryID);

// retrive statement associated with the tag
  string tag(QueryID);
  if (_conn->isCached("", tag))
  {
    _stmt = _conn->createStatement("", tag);
    _stmt->disableCaching();
    _conn->terminateStatement(_stmt);
    _stmt = NULL;
  }

  FREE(QueryID);
}

void eco_proc::unknown_command( )
{
   DBF
   throw EX_CMD_UNKN;
}
/////////////////////////////////////////////////////////////////////////
////// support  methods
///////////////////////////////////////////////////////////////////////////
void eco_proc::parse_conn_str(char *curr, string*& userName, string*& password, string*& DBname)
{
  char *pch = NULL;
  char del;

  if ( !curr)
    throw EX_CNSTR;
  del = '/';
  pch = strchr(curr,'/');
  if ( !pch)
    throw EX_CNSTR;
  *pch = '\0';
  userName = new string(curr);
  *pch = del;

  del = '@';
  curr = pch + 1;
  if ( !curr)
    throw EX_CNSTR;

  pch = strchr(curr, del);
  if ( !pch)
     throw EX_CNSTR;
  *pch = '\0';
  password = new string(curr);
  *pch = del;

  curr = pch + 1;
  if ( !curr)
    throw EX_CNSTR;
  DBname = new string(curr);
}

void eco_proc::get_options()
{
  int num = 0;
  char *opt_name = NULL;
  char *opt_value = NULL;
  int  opt_code;

  try{
        _conv->get_list_header(&num);
        if(_debug) {
          _dbcout << "options list contains " << num << " items" << endl;
        }

	while(num--)
        {
           _conv->get_tuple_header();
           _conv->get_atom(&opt_name);
           opt_code = COD(opt_name[0], opt_name[1], opt_name[2]);
									                   switch(opt_code){
             case AUTOCOMMIT:
               _conv->get_atom(&opt_value);
               if ( !strcmp("true", opt_value))
                 _AutoCommitMode = true;
               else
                 _AutoCommitMode = false;
	       FREE(opt_value);
               break;
             case MAXROWS:
               _conv->get_int(&_max_rows);
               break;
	     case CACHESIZE:
	       _conv->get_int(&_cache_size);
	       break;
	     default:
               gcout << "unknown options found: " << opt_name  << endl;
               throw EX_OPT; 								        }
	   FREE(opt_name);
	}
  }
  catch(int ec)
  {
    throw  EX_OPT | ec;
  }
}

void eco_proc::bind_params()
{
//BindVars = [BindVar]
//BindVar = integer() | float() | string() | date_time()
  eco_sql_row row(_conv, _stmt);
  row.decode_row();
  row.set_to_ora(_env);
}

void eco_proc::execute_query()
{
  _conv->create_res_header(4, "ok");

  ResultSet *rs;

  if(_stmt->status() == Statement::RESULT_SET_AVAILABLE)  // ==2
    rs = _stmt->getResultSet();
  else
    rs = _stmt->executeQuery();

  int processed = process_query(rs);
 // release rs and/or push it into cache

  //  encode Continue = eof | continue
  if(rs->status() == ResultSet::END_OF_FETCH){ // == 0
    _stmt->closeResultSet(rs);
    _conv->add_atom("eof");
  }
  else
  {
    _conv->add_atom("continue");
  }

  if(_debug) {
    _dbcout << "execute_sql: " << processed << " rows processed" << endl;    //
  }
}

void eco_proc::execute_nonquery()
{

  _conv->create_res_header(2, "ok");
  int num_rows = _stmt->executeUpdate();
  _conv->add_int(num_rows);
  if(_debug) {
     _dbcout << "execute_sql: " << num_rows << " updated/inserted/deleted" << endl;
  }
}

int eco_proc::process_query(ResultSet *rs)
{
// take types and create eco_row for output
// fetch data ( row by row )and encode it
  eco_sql_row row(_conv);
  row.init_row(rs);

  int row_fetched = 0;
  while (rs->next() &&  row_fetched < _max_rows)
  {
    row_fetched++;
    row.get_encode_row();
  }

  // encode empty list to complete
  _conv->encode_list_header(0);

  return  row_fetched;
}

void eco_proc::prepare_stmt()
{
// decode  QueryID
  char* QueryID = NULL;
  _conv->get_elem(&QueryID);
  decode_set_stmt(QueryID);
  FREE(QueryID);
// decode params and bind param values if any
  bind_params();
}

void eco_proc::decode_set_stmt(char* QueryID)
{
// decode  Query
  string  *SQLString = 0;
  _conv->get_str(&SQLString);

// create/retrive Statement
  if(QueryID)
    _stmt = _conn->createStatement(*SQLString, string(QueryID));
  else
    _stmt = _conn->createStatement(*SQLString);

  _stmt->setAutoCommit(_AutoCommitMode);

  delete SQLString;
}

void eco_proc::execute_plsql(eco_plsql_row* row)
{
// execute
  _stmt->execute();

  _conv->create_res_header(2, "ok");
// fetch
  row->get_encode_row();
}

void eco_proc::get_encode_meta_data(char *tab_name)
{
  // Call the getMetaData method on the Connection object obtained
  MetaData tab_metaData = _conn->getMetaData(tab_name, MetaData::PTYPE_TABLE);

  // Call getVector for attributes of list type,for example ATTR_LIST_COLUMNS
  vector<MetaData>listOfColumns;
  listOfColumns=tab_metaData.getVector(MetaData::ATTR_LIST_COLUMNS);

  int size = listOfColumns.size();
  _conv->encode_list_header(size);
  char yes[] = "true";
  char no[]  = "false";

  for (int ii=0; ii < size; ii++)
  {
    _conv->encode_tuple_header(6);

    string name(listOfColumns[ii].getString(MetaData::ATTR_NAME));
    _conv->add_string(name);

    int type = listOfColumns[ii].getInt(MetaData::ATTR_DATA_TYPE);
    _conv->add_int(type);
    int size_d = listOfColumns[ii].getInt(MetaData::ATTR_DATA_SIZE);
    _conv->add_int(size_d);
    int precision = listOfColumns[ii].getInt(MetaData::ATTR_PRECISION);
    _conv->add_int(precision);
    int scale = listOfColumns[ii].getInt(MetaData::ATTR_SCALE);
    _conv->add_int(scale);
    bool is_null = listOfColumns[ii].getBoolean(MetaData::ATTR_IS_NULL);
    if( is_null)
      _conv->add_atom(yes);
    else
      _conv->add_atom(no);
  }
  // encode empty list to complete
  _conv->encode_list_header(0);
}
