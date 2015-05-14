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

#if !defined(OPENBOOK_H)
#define OPENBOOK_H

#ifndef PERFT_MODE
void create_open_book ( const char *BOOK_TXT_FILE );
void update_open_book_eval ( const char *TXT_FILE );
int search_book ( const int i, const int j, const u64 key );

int search_openbook ( const u64 key, const int side );
void dump_hash (  );
TopenbookLeaf *search_book_tree ( TopenbookLeaf * root, const u64 key );
void make_openbook_tree (  );
void insert_openbook_leaf ( TopenbookLeaf ** root, Topenbook * l );
void serializza_book ( TopenbookLeaf * root );
int load_open_book (  );
#endif
#endif
