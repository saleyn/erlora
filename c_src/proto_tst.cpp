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

#include <iostream>
#include "ec_protocol.h"

using namespace std;

int main()
{
   cout << " Commands: " << endl;
   cout << "CONNECT " << CONNECT << endl;
   cout << "DISCONNECT " << DISCONNECT << endl;
   cout << "COMMIT " << COMMIT << endl;
   cout << "ROLLBACK " << ROLLBACK << endl;
   cout << "SELECT_SQL " << SELECT_SQL << endl;
   cout << "EXECUTE_SQL " << EXECUTE_SQL << endl;
   cout << "FREE_SQL " << FREE_SQL << endl;
   cout << "RUN_PLSQL " << RUN_PLSQL << endl;
   cout << "PREPARE_PLSQL " << PREPARE_PLSQL << endl;
   cout << "PROCESS_PLSQL " << PROCESS_PLSQL << endl;
   cout << "DESCRIBE " << DESCRIBE << endl;
   cout << "ARRAY_PLSQL " << ARRAY_PLSQL << endl;
   cout << " The End" << endl;
   return 0;
}
