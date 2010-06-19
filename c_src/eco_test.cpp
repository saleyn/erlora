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

/// test 08/14/06
#include "ec_protocol.h"  // ??
#include "eco_conv.h"

#include <erl_interface.h>
#include <ei.h>

#include <unistd.h>
#include <sys/io.h>
#include <stdio.h>
#include <fstream>

// #include <io.h>
/*
#define OPT_INITIALIZE 1
#define OPT_ITERATE    2
#define OPT_FINALIZE   3

#define COD(a, b, c) ( -300 + (c) + (a)*2 )
#define CONNECT                 COD('c','o','n')
#define DISCONNECT              COD('d','i','s')
#define COMMIT                  COD('c','o','m')
#define ROLLBACK                COD('r','o','l')
//#define SELECT                  COD('s','e','l')
#define PARAM_QUERY             COD('p','a','r')
#define PREPARE_SQL             COD('p','r','e')
#define EXECUTE_SQL             COD('e','x','e')
#define DESCRIBE_TABLE          COD('d','e','s')
//
{connect, ConnectionStr, [Options]}      //{connect, OraCmd, AutoCommitMode}
Option     = {autocommit, true|false}

disconnect

commit

rollback

{describe_table, Table}                           // Table = string

{execute_sql,SQLQuery, Options}
{execute_sql, QueryID, Params, Options}

{prepare_sql, SQLQuery, ParamTypes}

 {param_query, OraCmd}

{sql_query, OraCmd, Options}
OraCmd = [QueryType, SQLQuery]
QueryType =
        case SQLQuery of
        "SELECT" ++ _ -> select;
        "INSERT" ++ _ -> insert;
        "UPDATE" ++ _ -> update;
        "DELETE" ++ _ -> delete;
        "BEGIN"  ++ _ -> plsql
        end,

//{select, OraCmd}

//{describe_table, OraCmd}
//OraCmd = "SELECT * FROM " ++ Table ++ " WHERE ROWNUM < 1"


*/

//typedef unsigned char byte;
typedef char byte;

int read_cmd(byte *buf);
int write_cmd(byte *buf, int len);
int read_exact(byte *buf, int len);
int write_exact(byte *buf, int len);
int create_ok(byte *_term, int *pind, int yes );
/*
int initialize(int x) {
  return x+1;
}

int iterate(int y) {
  return y*2;
}

int final(int z) {
  return z*3;
}
*/
void debug_print(byte *buf, int len);

FILE* fptr;
using namespace std;

int gdebug;
fstream gcout;

int main() {
  int res;

  gdebug = 1;
  gcout.open("err.txt", std::fstream::out);

//input from Erlang
  //   char
     byte term[100];         // input from Erlang
     byte *result;           // output to Erlang
     int size;           // size
     int rc = 0;

  fptr = fopen("deb.txt", "w");
  //fstream dbcout("dbout.txt");  //.open("dbout.txt");

  fprintf(fptr, "Hi!\n");
//  fprintf(fptr, "tuple = %d,atom=%d\n", ERL_TUPLE, ERL_ATOM);
  erl_init(NULL, 0);

  eco_conv ecoc(1);

while (read_cmd(term) > 0) {
  fprintf(fptr,"inside while loop\n");
  try{
    fprintf(fptr, "term=\%s\n", term);

    ecoc.get_term(term);
    ecoc.get_cmd( );
    fprintf(fptr, "elem=%d\n", ecoc.get_elem( ));

//output
    ecoc.create_res_header(1);
    ecoc.add_c_str("I love Erlang");
    ecoc.set_result(&result, &size);
   }
   catch(int ec)
   {
      ecoc.create_error_msg(ec);
      fprintf(fptr, "error: ec=%d found\n", ec);
   }
    rc = write_cmd( result, size);
    fprintf(fptr, "write_cmd: rc=%d\n", rc);
    fflush(fptr);
  } // while
  return 0;
}

//-----------------------------------------------------------------
int read_cmd(byte *buf)
{
  int len;

  if (read_exact(buf, 2) != 2)
    return(-1);
  len = (buf[0] << 8) | buf[1];
  fprintf(fptr, "read_cmd: len=%d, buf=%s\n", len, (char*) buf);
  debug_print(buf, 2);
return read_exact(buf, len);
}

int write_cmd(byte *buf, int len)
{
  byte li;

  li = (len >> 8) & 0xff;
  write_exact(&li, 1);

  li = len & 0xff;
  write_exact(&li, 1);

  return write_exact(buf, len);
}

int read_exact(byte *buf, int len)
{
  int i, got=0;

  do {
    if ((i = read(0, buf+got, len-got)) <= 0)
      return(i);
    got += i;
  } while (got<len);

  fprintf(fptr, "read_exact: len=%d, buf=%s\n", len, (char*) buf);
  debug_print(buf, len);

  return(len);
}

int write_exact(byte *buf, int len)
{
  int i, wrote = 0;

  do {
    if ((i = write(1, buf+wrote, len-wrote)) <= 0)
      return (i);
    wrote += i;
  } while (wrote<len);

  return (len);
}

void debug_print(byte *buf, int len)
{
 int ch;
 int ii;

 for(ii = 0; ii < len; ii++){
   ch = buf[ii];
   fprintf(fptr, "ii=%d %d %c\n", ii, ch, ch);
 }
 fflush(fptr);
}

int create_ok(byte *_term, int *pind, int yes )
{
//output
int rc;
    *pind = 0;
    rc = ei_encode_version(_term, pind);
    rc = ei_encode_tuple_header(_term, pind, 2);
    if ( yes)
       rc = ei_encode_atom(_term, pind, "ok");
    else
       rc = ei_encode_atom(_term, pind, "error");

  return rc;
}
