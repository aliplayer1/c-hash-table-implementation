#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TABLE_SIZE 100

// Structure to store each entry in the hash table
typedef struct Entry{
    char *key;
    int value;
    struct Entry *next;
} Entry;

typedef struct {
    Entry **buckets;
} HashTable;

unsigned int hash(const char *key){
    unsigned long int hash = 0;
    int i = 0;
    while (key[i] != '\0'){
        hash = hash * 31 + key[i];
        i++;
    }
    return hash % TABLE_SIZE;
}

HashTable* create_table() {
    HashTable *table = malloc(sizeof(HashTable));
    table -> buckets = malloc(sizeof(Entry*) * TABLE_SIZE);
    for (int i = 0; i < TABLE_SIZE; i++) {
        table -> buckets[i] = NULL;
    }
    return table;
}

void insert(HashTable *table, const char *key, int value) {
    unsigned int index = hash(key);
    Entry *new_entry = malloc(sizeof(Entry));
    new_entry->key = strdup(key);
    new_entry->value = value;
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;
}

int search(HashTable *table, const char *key) {
    unsigned int index = hash(key);
    Entry *entry = table->buckets[index];
    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return -1; // Return -1 if key is not found
}

void delete(HashTable *table, const char *key) {
    unsigned int index = hash(key);
    Entry *entry = table->buckets[index];
    Entry *prev = NULL;

    while (entry != NULL && strcmp(entry->key, key) != 0) {
        prev = entry;
        entry = entry->next;
    }

    if (entry == NULL) {
        return; // Key not found
    }

    if (prev == NULL) {
        table->buckets[index] = entry->next;
    } else {
        prev->next = entry->next;
    }

    free(entry->key);
    free(entry);
}

void free_table(HashTable *table) {
    for (int i = 0; i < TABLE_SIZE; i++) {
        Entry *entry = table->buckets[i];
        while (entry != NULL) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp);
        }
    }
    free(table->buckets);
    free(table);
}
