#ifndef HASH_TABLE_H
#define HASH_TABLE_H

// hashtable 元素结构
typedef struct {
    char* key;
    char* value;
} ht_item;

// hashtable 表结构
typedef struct {
    int size;
    int base_size;
    int count;
    ht_item** items;
} ht_hash_table;

//新建表
ht_hash_table* ht_new();
//删除表
void ht_del_hash_table(ht_hash_table* ht);
//插入元素（覆盖相同key内容作为更新元素操作）
void ht_insert(ht_hash_table* ht, const char* key, const char* value);
//查询元素
char* ht_search(ht_hash_table* ht, const char* key);
//删除元素
void ht_delete(ht_hash_table* h, const char* key);

#endif  // HASH_TABLE_H