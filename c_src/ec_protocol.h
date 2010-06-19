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

/****  Command protocol between Erlang and C ****/
/** last update 11/07/06 **/
#ifndef EC_PRO
#define EC_PRO


/* Constats defining the command protocol between the Erlang control process
 *    and the port program.
 *    These constants based on first, second and third letters in  Erlang command,
 *    If needed must be redefined in the same time on both sides
 */
#define COD(a, b, c) ( -500 + (c) + (b)*2 + (a)*2 )
//commands
#define CONNECT                 COD('c','o','n')
#define DISCONNECT              COD('d','i','s')
#define COMMIT                  COD('c','o','m')
#define ROLLBACK                COD('r','o','l')
#define SELECT_SQL              COD('s','e','l')
#define EXECUTE_SQL             COD('e','x','e')
#define FREE_SQL                COD('f','r','e')
#define RUN_PLSQL               COD('r','u','n')
#define PREPARE_PLSQL           COD('p','r','e')
#define PROCESS_PLSQL           COD('p','r','o')
#define DESCRIBE                COD('d','e','s')
#define ARRAY_PLSQL             COD('a','r','r')
// options
#define AUTOCOMMIT              COD('a','u','t')
#define MAXROWS                 COD('m','a','x')
#define CACHESIZE               COD('q','u', 'e')
// types (for null)
#define ECO_INT                 COD('i','n','t')
#define ECO_FT                  COD('f','l','o')
#define ECO_DT                  COD('d','a','t')
#define ECO_STR                 COD('s','t','r')


// as of ora.erl 10/05/06
#endif
