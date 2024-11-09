#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Define initial size and load factor threshold
#define INITIAL_SIZE 16
#define MAX_LOAD_FACTOR 0.75
#define GROWTH_FACTOR 2

// Added status codes for operations
typedef enum {
    HT_SUCCESS,
    HT_ERROR,
    HT_KEY_NOT_FOUND,
    HT_MEMORY_ERROR
} HashTableStatus;

typedef struct Entry {
    char *key;
    int value;
    struct Entry *next;
    bool is_deleted;  // For lazy deletion
} Entry;

typedef struct {
    Entry **buckets;
    size_t size;      // Current number of elements
    size_t capacity;  // Current table size
} HashTable;

// Improved hash function using FNV-1a
unsigned int hash(const char *key, size_t capacity) {
    unsigned long int hash = 14695981039346656037UL;  // FNV offset basis
    for (size_t i = 0; key[i] != '\0'; i++) {
        hash ^= (unsigned char)key[i];
        hash *= 1099511628211UL;  // FNV prime
    }
    return hash % capacity;
}

// Create table with error checking
HashTable* create_table(void) {
    HashTable *table = malloc(sizeof(HashTable));
    if (!table) return NULL;

    table->capacity = INITIAL_SIZE;
    table->size = 0;
    table->buckets = calloc(INITIAL_SIZE, sizeof(Entry*));
    
    if (!table->buckets) {
        free(table);
        return NULL;
    }
    return table;
}

// Helper function to resize the hash table
static HashTableStatus resize_table(HashTable *table) {
    size_t new_capacity = table->capacity * GROWTH_FACTOR;
    Entry **new_buckets = calloc(new_capacity, sizeof(Entry*));
    if (!new_buckets) return HT_MEMORY_ERROR;

    // Rehash all existing entries
    for (size_t i = 0; i < table->capacity; i++) {
        Entry *entry = table->buckets[i];
        while (entry) {
            Entry *next = entry->next;
            if (!entry->is_deleted) {
                unsigned int new_index = hash(entry->key, new_capacity);
                entry->next = new_buckets[new_index];
                new_buckets[new_index] = entry;
            } else {
                // Free deleted entries during resize
                free(entry->key);
                free(entry);
            }
            entry = next;
        }
    }

    free(table->buckets);
    table->buckets = new_buckets;
    table->capacity = new_capacity;
    return HT_SUCCESS;
}

HashTableStatus insert(HashTable *table, const char *key, int value) {
    if (!table || !key) return HT_ERROR;

    // Check load factor and resize if necessary
    if ((float)(table->size + 1) / table->capacity > MAX_LOAD_FACTOR) {
        HashTableStatus status = resize_table(table);
        if (status != HT_SUCCESS) return status;
    }

    unsigned int index = hash(key, table->capacity);
    
    // Check for existing key and update
    Entry *current = table->buckets[index];
    while (current) {
        if (!current->is_deleted && strcmp(current->key, key) == 0) {
            current->value = value;
            return HT_SUCCESS;
        }
        current = current->next;
    }

    // Create new entry
    Entry *new_entry = malloc(sizeof(Entry));
    if (!new_entry) return HT_MEMORY_ERROR;

    new_entry->key = strdup(key);
    if (!new_entry->key) {
        free(new_entry);
        return HT_MEMORY_ERROR;
    }

    new_entry->value = value;
    new_entry->is_deleted = false;
    new_entry->next = table->buckets[index];
    table->buckets[index] = new_entry;
    table->size++;

    return HT_SUCCESS;
}

HashTableStatus get(HashTable *table, const char *key, int *value) {
    if (!table || !key || !value) return HT_ERROR;

    unsigned int index = hash(key, table->capacity);
    Entry *entry = table->buckets[index];

    while (entry) {
        if (!entry->is_deleted && strcmp(entry->key, key) == 0) {
            *value = entry->value;
            return HT_SUCCESS;
        }
        entry = entry->next;
    }
    return HT_KEY_NOT_FOUND;
}

HashTableStatus delete(HashTable *table, const char *key) {
    if (!table || !key) return HT_ERROR;

    unsigned int index = hash(key, table->capacity);
    Entry *entry = table->buckets[index];
    Entry *prev = NULL;

    while (entry) {
        if (!entry->is_deleted && strcmp(entry->key, key) == 0) {
            // Use lazy deletion
            entry->is_deleted = true;
            table->size--;
            return HT_SUCCESS;
        }
        prev = entry;
        entry = entry->next;
    }
    return HT_KEY_NOT_FOUND;
}

void free_table(HashTable *table) {
    if (!table) return;

    for (size_t i = 0; i < table->capacity; i++) {
        Entry *entry = table->buckets[i];
        while (entry) {
            Entry *temp = entry;
            entry = entry->next;
            free(temp->key);
            free(temp);
        }
    }
    free(table->buckets);
    free(table);
}

// New function to get current load factor
float get_load_factor(HashTable *table) {
    return table ? (float)table->size / table->capacity : 0.0f;
}

// New function to get statistics about the hash table
void get_stats(HashTable *table, size_t *size, size_t *capacity, float *load_factor) {
    if (!table) return;
    if (size) *size = table->size;
    if (capacity) *capacity = table->capacity;
    if (load_factor) *load_factor = get_load_factor(table);
}