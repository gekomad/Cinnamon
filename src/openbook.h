#if !defined(OPENBOOK_H)
#define OPENBOOK_H

#ifndef PERFT_MODE
void create_open_book ( const char *BOOK_TXT_FILE );
int search_book ( const int i, const int j, const u64 key );
int search_openbook ( const u64 key, const int side );
void dump_hash (  );
TopenbookLeaf * search_book_tree ( TopenbookLeaf * root, const u64 key );
void make_openbook_tree (  );
void insert_openbook_leaf ( TopenbookLeaf ** root, Topenbook * l );
void serializza_book ( TopenbookLeaf * root );
int load_open_book (  );

#endif
#endif
