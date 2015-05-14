/*
Copyright (C) 2008-2010
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#if !defined(_butterfly_h)
#define _butterfly_h

void do_move ( int );
void dispose (  );

#ifdef PERFT_MODE
void do_perft (  );
#endif

#ifndef PERFT_MODE
#ifdef _MSC_VER
void hand_do_move (  );
#else
void *hand_do_move ( void *xxza );
#endif

#endif


#endif
