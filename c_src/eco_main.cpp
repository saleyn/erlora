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
#include <fstream>
#include <sys/unistd.h>
#include "eco_marshal.h"
#include "eco_proc.h"

using namespace std;

int gdebug;
fstream gcout;

//-----------------------------------------------------------------
// Main procedure
//-----------------------------------------------------------------

int main(int argc, char *argv[])
{
  int ret = 0;
  char *term  = NULL;     // input term
  int size = 0;           // output term size
  char *result = NULL;    // output term

  char *infile  = NULL;   // for debug
  char *outfile = NULL;
  char *errfile = NULL;
  int debug = 0;

  // check options
  int ch = 'A';
  extern char *optarg;
  extern int optind, optopt;
  gcout.open("err.log", std::fstream::out);

  while ((ch = getopt(argc, argv, ":d")) != -1) {
    gcout << "ch= " << ch << endl;
    switch(ch) {
	case 'd':
           debug = 1;
           break;
	case '?':
           fprintf(stderr, "Unrecognized option: -%c\n", optopt);
     }
  }
  gdebug = debug;
  gcout << "debug= " << debug << endl;
  debug = 1;
// re-assign streams if needed
/* for debug only
  if ( argc > 1) {
     infile  = argv[1];
     outfile = argv[2];
     errfile = argv[3];
  }
*/
  // Initialize communication with Erlang
  init_erl_io(infile, outfile, errfile);

  // Create e(rlang)c(++)o(racle)_int(erface) object
  // UserName, Password, DBName will come later from erlang OPEN_CONNECTION cmd
  eco_proc eco(debug);
//sleep(120);
  // read from erl encoded term, process it and  write to erl result
  while ((!ret && (term = read_erl_term()) != NULL )) {   // read the next term from Erlang

  // Process the tuple
    ret = eco.process_term(term, &result, &size);

  // Write Result back to Erlang
    write_erl_term(result, size);
  }

  close_erl_io();

   gcout << "process_term: exit code = " << ret << endl;
  return ret;
}
