#ifndef HASHTABLE_H
#define HASHTABLE_H

#pragma once


#include <stdint.h>
#include <stddef.h>
#include <assert.h>
#include <atomic>
#include <memory>

typedef unsigned long long ull; 

// Forward declarations
class HashTable;
class HashMap;

/**
 * @struct HashNode
 * @brief Basic structure for hash table entries
 * 
 * This structure represents a node in the hash table's linked list.
 * Extend this structure to include actual key-value data for your use case.
 */
struct HashNode {
    ull hashcode;           // Hash value of the key
    HashNode* next;         // Pointer to next node in the chain
    // Add your key-value data members here
};

/**
 * @class HashTable
 * @brief Implementation of a hash table using chaining for collision resolution
 * 
 * This class provides the core functionality of a hash table, including
 * insertion, lookup, and extraction of nodes. It supports move semantics
 * for efficient table resizing operations.
 */
class HashTable {
public:
    /**
     * @brief Default constructor
     */
    HashTable() : table(nullptr), bitmask(0), table_size(0) {}

    /**
     * @brief Move constructor
     * @param other The HashTable to move from
     */
    HashTable(HashTable&& other) noexcept 
        : table(std::move(other.table)),
          bitmask(other.bitmask),
          table_size(other.table_size.load()) {
        other.bitmask = 0;
        other.table_size = 0;
    }

    /**
     * @brief Move assignment operator
     * @param other The HashTable to move from
     * @return Reference to this HashTable
     */
    HashTable& operator=(HashTable&& other) noexcept {
        if (this != &other) {
            table = std::move(other.table); // automatic free mem
            bitmask = other.bitmask;
            table_size = other.table_size.load();
            other.bitmask = 0;
            other.table_size = 0;
        }
        return *this;
    }

    /**
     * @brief Initialize the hash table
     * @param n Size of the table (must be a power of 2)
     */
    void init(ull n);

    /**
     * @brief Insert a node into the hash table
     * @param node Pointer to the node to insert
     */
    void insert(HashNode* node);

    /**
     * @brief Locate a node in the hash table
     * @param target Node to search for
     * @param compare Function to compare nodes
     * @return Pointer to pointer to found node, or nullptr if not found
     */
    HashNode** locate(HashNode* target, bool (*compare)(HashNode*, HashNode*));

    /**
     * @brief Extract a node from its location
     * @param location Pointer to pointer to node to extract
     * @return Pointer to extracted node
     */
    HashNode* extract(HashNode** location);

    // Public members for internal access
    std::unique_ptr<HashNode*[]> table;
    ull bitmask;
    std::atomic<ull> table_size;
};

/**
 * @class HashMap
 * @brief Progressive resizing hash map implementation
 * 
 * This class provides a hash map implementation that supports progressive
 * resizing to maintain consistent performance during table expansion.
 */
class HashMap {
public:
    /**
     * @brief Insert an item into the hash map
     * @param item Pointer to the node to insert
     */
    void insert(HashNode* item);

    /**
     * @brief Search for an item in the hash map
     * @param key Node containing search key
     * @param compare Function to compare nodes
     * @return Pointer to found node, or nullptr if not found
     */
    HashNode* search(HashNode* key, bool (*compare)(HashNode*, HashNode*));

    /**
     * @brief Remove an item from the hash map
     * @param target Node to remove
     * @param compare Function to compare nodes
     * @return Pointer to removed node, or nullptr if not found
     */
    HashNode* erase(HashNode* target, bool (*compare)(HashNode*, HashNode*));

    /**
     * @brief Get the total number of items in the hash map
     * @return Total number of items
     */
    ull size();

    /**
     * @brief Free all resources used by the hash map
     */
    void freeUp();

    /**
     * @brief Initiate the resizing process
     */
    void initiateResize();

    /**
     * @brief Process a batch of entries during resizing
     */
    void processResize();

    static const ull kMaxLoadFactor = 8;        // Maximum load factor before resizing
    static const ull kResizingWorkload = 128;   // Number of entries to move per resize operation
    
    std::atomic<ull> resize_pos{0};             // Current position in resize operation
    HashTable hashTable1;                       // Primary hash table
    HashTable hashTable2;                       // Secondary hash table for resizing
};


inline ull HashMap::size() {
    return hashTable1.table_size.load(std::memory_order_relaxed) + 
           hashTable2.table_size.load(std::memory_order_relaxed);
}

inline void HashMap::freeUp() {
    hashTable1 = HashTable();
    hashTable2 = HashTable();
    resize_pos = 0;
}

#endif // HASHTABLE3_H