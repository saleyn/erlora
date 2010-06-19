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

//-----------------------------------------------------------------
// C/Erlang Interface routines
//-----------------------------------------------------------------
#ifndef _MARSHAL_H_
#define _MARSHAL_H_

//-----------------------------------------------------------------
// Intialize input/output streams for communicating with Erlang
//-----------------------------------------------------------------
void  init_erl_io(char *infile, char *outfile, char *errfile);

//-----------------------------------------------------------------
// Close input/output streams for communicating with Erlang
//-----------------------------------------------------------------
void  close_erl_io(void);

//-----------------------------------------------------------------
// Read data from Erlang
// Parameters:
//   output_file - optional output file used to store the content
//                 of Erlang message for debugging
//   Note: The caller is resposible for freeing returned term.
//-----------------------------------------------------------------
char *read_erl_term();

//-----------------------------------------------------------------
// Write data to Erlang
//   Note: This function frees the term after it's been marshaled
//-----------------------------------------------------------------
int write_erl_term(char *term, int size);

#endif
