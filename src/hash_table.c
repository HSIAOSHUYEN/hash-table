#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "hash_table.h"
#include "prime.h"

#define HT_PRIME_1 151
#define HT_PRIME_2 199

//删除标记
static ht_item HT_DELETED_ITEM = {NULL, NULL};

//新建元素
//声明为 static 因为其只在 hashtable 内部调用
static ht_item* ht_new_item(const char* k, const char* v) {
    ht_item* i = malloc(sizeof(ht_item));
    i->key = strdup(k);
    i->value = strdup(v);
    return i;
}

//新建表
static ht_hash_table* ht_new_sized(const int base_size) {
    ht_hash_table* ht = malloc(sizeof(ht_hash_table));
    ht->base_size = base_size;
    ht->size = next_prime(ht->size);
    ht->count = 0;
    ht->items = calloc((size_t)ht->size, sizeof(ht_item*));
    return ht;
}
ht_hash_table* ht_new() { return ht_new_sized(HT_INITIAL_BASE_SIZE); }

//删除表元素
static void ht_del_item(ht_item* i) {
    free(i->key);
    free(i->value);
    free(i);
}

//删除表
void ht_del_hash_table(ht_hash_table* ht) {
    //释放表中所有元素，遍历表中的所有元素逐一删除
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL) ht_del_item(item);
    }
    //释放表本身
    free(ht->items);
    free(ht);
}

// hash 函数
// s为需要hash的目标字符串
// a是一个大于字母表的素数，我们需要hash的是ASCII表，其长度为128，因此取一个大于它的素数，假设为151
// m是容量长度
static int ht_hash(const char* s, const int a, const int m) {
    long hash = 0;
    const int len_s = strlen(s);
    for (int i = 0; i < len_s; i++) {
        hash += (long)pow(a, len_s - (i + 1)) * s[i];
        hash = hash % m;
    }
    return (int)hash;
}

//双重hash以防止冲突
static int ht_get_hash(const char* s, const int num_buckets,
                       const int attempt) {
    const int hash_a = ht_hash(s, HT_PRIME_1, num_buckets);
    const int hash_b = ht_hash(s, HT_PRIME_2, num_buckets);
    return (hash_a + (attempt * (hash_b + 1))) % num_buckets;
}

//重新分配大小
static void ht_resize(ht_hash_table* ht, const int base_size) {
    if (base_size < HT_INITIAL_BASE_SIZE) {
        return;
    }
    ht_hash_table* new_ht = ht_new_sized(base_size);
    for (int i = 0; i < ht->size; i++) {
        ht_item* item = ht->items[i];
        if (item != NULL && item != &HT_DELETED_ITEM) {
            ht_insert(new_ht, item->key, item->value);
        }
    }

    ht->base_size = new_ht->base_size;
    ht->count = new_ht->count;

    // To delete new_ht, we give it ht's size and items
    const int tmp_size = ht->size;
    ht->size = new_ht->size;
    new_ht->size = tmp_size;

    ht_item** tmp_items = ht->items;
    ht->items = new_ht->items;
    new_ht->items = tmp_items;

    ht_del_hash_table(new_ht);
}

//增大容量
static void ht_resize_up(ht_hash_table* ht) {
    const int new_size = ht->base_size * 2;
    ht_resize(ht, new_size);
}

//缩小容量
static void ht_resize_down(ht_hash_table* ht) {
    const int new_size = ht->base_size / 2;
    ht_resize(ht, new_size);
}

//插入元素
void ht_insert(ht_hash_table* ht, const char* key, const char* value) {
    //插入前检查load是否需要重新分配大小
    const int load = ht->count * 100 / ht->size;
    if (load > 70) {
        ht_resize_up(ht);
    }
    //新建元素结构
    ht_item* item = ht_new_item(key, value);
    //调用double hash返回作为hash表的索引
    int index = ht_get_hash(item->key, ht->size, 0);
    ht_item* cur_item = ht->items[index];
    int i = 1;
    // iterate through indexes until we find an empty bucket
    while (cur_item != NULL) {
        if (cur_item != &HT_DELETED_ITEM) {
            //找到匹配的key则覆盖value作为更新操作
            if (strcmp(cur_item->key, key) == 0) {
                ht_del_item(cur_item);
                ht->items[index] = item;
                return;
            }
        }
        index = ht_get_hash(item->key, ht->size, i);
        cur_item = ht->items[index];
        i++;
    }
    //插入元素内容
    ht->items[index] = item;
    ht->count++;
}

//查找元素
char* ht_search(ht_hash_table* ht, const char* key) {
    int index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;
    //与插入元素类似，不过在循环中需要判断索引处的key值是否匹配
    //若索引处为NULL，则跳出循环直接返回NULL，表示未找到
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                return item->value;
            }
        }
        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }
    return NULL;
}

//删除元素，不能直接删除，因为要删除的元素可能位于哈希表冲突链中的一部分
//若删除了这个元素，则冲突链会断裂，当下次需要查找某个元素的世界位置位于这次删除的位置之后时
//因为删除了这个元素，此位置为NULL，会导致返回未找到的结果
//因此处理删除时，作软删除将其标记为删除（指向一个全局的标记），而不是释放内存
void ht_delete(ht_hash_table* ht, const char* key) {
    //删除前检查load是否需要重新分配大小
    const int load = ht->count * 100 / ht->size;
    if (load < 10) {
        ht_resize_down(ht);
    }
    //先搜索到
    int index = ht_get_hash(key, ht->size, 0);
    ht_item* item = ht->items[index];
    int i = 1;
    while (item != NULL) {
        if (item != &HT_DELETED_ITEM) {
            if (strcmp(item->key, key) == 0) {
                ht_del_item(item);
                //软删除
                ht->items[index] = &HT_DELETED_ITEM;
            }
        }
        index = ht_get_hash(key, ht->size, i);
        item = ht->items[index];
        i++;
    }
    ht->count--;
}