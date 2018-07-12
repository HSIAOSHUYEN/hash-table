#include <stdio.h>
#include <stdlib.h>
#include "src/hash_table.h"

int main(int argc, char const* argv[]) {
    // new hashtable
    ht_hash_table* ht = ht_new();

    // insert into hashtable
    ht_insert(ht, "foo", "bar");
    ht_insert(ht, "name", "ruofeng");
    ht_insert(ht, "number", "41524226");

    // search from hashtable
    printf("%s\n", ht_search(ht, "foo"));
    printf("%s\n", ht_search(ht, "name"));

    // update test
    ht_insert(ht, "foo", "barr");
    printf("%s\n", ht_search(ht, "foo"));

    // key not exsit test
    if (ht_search(ht, "not") == NULL) printf("%s\n", "key(not) doesn't exist");

    // delete key test
    ht_delete(ht, "foo");
    if (ht_search(ht, "foo") == NULL) printf("%s\n", "key(foo) doesn't exist");

    // delete hash table
    ht_del_hash_table(ht);

    return 0;
}
