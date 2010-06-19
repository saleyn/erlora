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

#include <unistd.h>
#include <sys/io.h>
#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <erl_interface.h>
#include "eco_marshal.h"


typedef unsigned char byte;
#define MIN_SIZE 512

char defbuf[MIN_SIZE];

FILE *save_infile  = NULL;
//FILE *save_outfile = NULL;
//FILE *save_errfile = NULL;

int read_exact(char*, int);
int write_exact(char*, int);

using namespace std;

//=================================================================
//====================== PUBLIC ROUTINES ==========================
//=================================================================

//-----------------------------------------------------------------
// Initializing stdin / stdout
//-----------------------------------------------------------------
void init_erl_io(char *infile, char *outfile, char *errfile)
{
  if (infile) {
    assert((save_infile = freopen(infile, "rb", stdin)) != NULL);
  } else {
    assert((save_infile = fopen("input.log", "wb")) != NULL);
  }
  // Initialize Erlang interface library
  erl_init(NULL, 0);
}

void close_erl_io(void)
{
  /* UNDO: now undo the effects of redirecting stdin/stdout/stderr ... */
  if (save_infile) {
    fclose(save_infile);
    save_infile = NULL;
  }
}

//-----------------------------------------------------------------
// Read term from Erlang
//   Note: The caller is resposible for freeing returned term.
//-----------------------------------------------------------------
char *read_erl_term()
{
  int      size, len, allocated;
  char     *buf;
//  ofstream file;

  if (read_exact(&defbuf[0], 4) != 4)
    return NULL;

  if (save_infile) {
    fwrite(defbuf, 1, 4, save_infile);
  }

//  size = (byte) ( (defbuf[0] << 24) | (defbuf[1] << 16) | (defbuf[2] << 8) | defbuf[3]);
  size = (byte) defbuf[3];
  int ii, sft;
  for(ii = 2, sft = 8; ii >= 0;  ii --, sft += 8)
	  size |= ((byte) defbuf[ii]) << sft;

  if(!size)
    return NULL;

  allocated = (size > MIN_SIZE);

  if (!allocated)
    buf = &defbuf[0];
  else if ((buf = (char *) malloc(size)) == NULL) {
//    erl_err_sys("Error in read(): erl_malloc(%d)!", size);
    return NULL;
  }

   len = read_exact(buf, size);

   fprintf(save_infile, "Term size=%d, len=%d, (min_size=%d)\n", size, len, MIN_SIZE);
   fprintf(save_infile, "  buf=%d,%d,%d,%d)\n", defbuf[0],defbuf[1],defbuf[2],defbuf[3]);

   if (len != size)
  //   buf = NULL;
     return NULL;

   if (save_infile) {
     fwrite(buf, 1, size, save_infile);
    fprintf(save_infile, "\nbuffer of size %d read\n", size);
    for(int ii=0; ii < size; ii++)
    {
      fprintf(save_infile, " %d,%c", buf[ii], buf[ii]);
      if(ii < size - 1)
        fprintf(save_infile, ",");
      else
        fprintf(save_infile, "\n");
    }
    fflush(save_infile);
  }
  return (char*)buf;
}

//-----------------------------------------------------------------
// Write term to Erlang
//   Note: The caller is resposible for freeing returned term.
//-----------------------------------------------------------------
int write_erl_term(char *term, int  size)
{
  int result;

  if (term == NULL)
    return 0;

  // Write size in big_endian format

  defbuf[0] = (size >> 24) & 0xff; write_exact(&defbuf[0], 1);
  defbuf[0] = (size >> 16) & 0xff; write_exact(&defbuf[0], 1);
  defbuf[0] = (size >> 8)  & 0xff; write_exact(&defbuf[0], 1);
  defbuf[0] = size         & 0xff; write_exact(&defbuf[0], 1);

  result = write_exact(term, size);
  if (result != size)
      erl_err_sys("Error in write_exact()! Bytes written: %d of %d!", result, size);

  return result;
}

//=================================================================
//==================== INTERNAL ROUTINES ==========================
//=================================================================


//-----------------------------------------------------------------
// Data marshaling routines
//-----------------------------------------------------------------

int read_exact(char *buf, int len)
{
  // Read size in big_endian format
  cin.read(buf, len);
  len = cin.gcount();

  return len;
}

//-----------------------------------------------------------------
int write_exact(char *buf, int len)
{
  cout.write(buf, len);
  if (cout.fail())
    return 0;

  return len;
}
