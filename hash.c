#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


// WARNING THIS IS A WORK IN PROGRESS DEMO
// I know this code is not that good

#define DEFAULT_HASH_SIZE 256
typedef struct Entry_t {
    char * key;
    int value;
    struct Entry_t * next;
} Entry;

typedef struct Hashmap_t {
    size_t size;
    Entry * items;
} Hashmap;

uint64_t hash_function(char *key) {
    assert(key != NULL);
    uint64_t hash = 5381;
    uint64_t c = 0;

    // go until string null terminating char
    while (c = *key++)
        hash = ((hash << 5) + hash) + c;
        // hash * 33 + c

    return hash;
}

void init_hash(Hashmap ** hash) {
    assert((*hash) == NULL);
    Entry * items = malloc(sizeof(Entry) * DEFAULT_HASH_SIZE);
    for (int i = 0; i < DEFAULT_HASH_SIZE; i++) {
        items[i].key = NULL;
        items[i].next = NULL;
    }
    (*hash) = malloc(sizeof(Hashmap));
    (*hash)->items = items;
    (*hash)->size = DEFAULT_HASH_SIZE;
}

void add_hash(Hashmap * hash, char *key, int value) {
    size_t array_index = hash_function(key) % hash->size;
    // either this is the first entry
    if (hash->items[array_index].key == NULL) {
        hash->items[array_index].key = key;
        hash->items[array_index].value = value;
        return;
    }
    // or this is the second and now we have to start linking
    hash->items[array_index].next = malloc(sizeof(Entry));
    add_hash(hash, key, value);
}

Entry * get_value(Hashmap * hash, char * key) {
    size_t array_index = hash_function(key) % hash->size;

    Entry * entry = &(hash->items[array_index]);
    while (true) {
        if (entry->key == NULL)
            return NULL;
        if (entry->key == key)
            return entry;
        if (entry->next == NULL)
            return NULL;
        entry = entry->next;
    }
}

void free_linked_list(Entry ** entry) {
    if ((*entry)->next != NULL) {
        free_linked_list(&((*entry)->next));
    }
    free(*entry);
    (*entry) = NULL;
}

void delete_hash(Hashmap ** hash) {
    assert((*hash) != NULL);
    for (int i = 0; i < DEFAULT_HASH_SIZE; i++) {
        if ((*hash)->items[i].next != NULL)
            free_linked_list(&((*hash)->items[i].next));
    }
    free((*hash)->items);
    (*hash) = NULL;
}

int main() {
    printf("start of the hash function\n");
    char * new_key = "something to hash";
    char * new_key_1 = "something to hash 1";

    Hashmap * hash = NULL;
    init_hash(&hash);
    add_hash(hash, new_key, 5);
    add_hash(hash, new_key_1, 10);
    Entry * entry = get_value(hash, new_key);
    printf("The key: %s has value %d\n", entry->key, entry->value);
    delete_hash(&hash);
    return 0;
}
