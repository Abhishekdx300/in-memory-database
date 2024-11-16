#include "hashtable.h"

void HashTable::init(ull n) {
    assert(n > 0 && ((n - 1) & n) == 0);
    table = std::make_unique<HashNode*[]>(n);
    for (ull i = 0; i < n; i++) {
        table[i] = nullptr;
    }
    bitmask = n - 1;
    table_size = 0;
}

void HashTable::insert(HashNode* node) {
    ull position = node->hashcode & bitmask;
    HashNode* next = table[position];
    node->next = next;
    table[position] = node;
    table_size.fetch_add(1, std::memory_order_relaxed);
}

HashNode** HashTable::locate(HashNode* target, bool (*compare)(HashNode*, HashNode*)) {
    if (!table) return nullptr;
    
    ull index = target->hashcode & bitmask;
    HashNode** pointer = table.get() + index;
    while (*pointer != nullptr) {
        if ((*pointer)->hashcode == target->hashcode && compare(*pointer, target)) {
            return pointer;
        }
        pointer = &(*pointer)->next;
    }
    return nullptr;
}

HashNode* HashTable::extract(HashNode** location) {
    HashNode* extracted = *location;
    *location = extracted->next;
    table_size.fetch_sub(1, std::memory_order_relaxed);
    return extracted;
}

void HashMap::insert(HashNode* item) {
    if (!hashTable1.table) {
        hashTable1.init(4);
    }
    hashTable1.insert(item);

    if (!hashTable2.table) {
        ull load = hashTable1.table_size.load(std::memory_order_relaxed) / 
                  (hashTable1.bitmask + 1);
        if (load >= kMaxLoadFactor) {
            initiateResize();
        }
    }
    processResize();
}

void HashMap::initiateResize() {
    assert(!hashTable2.table);
    hashTable2 = std::move(hashTable1);
    hashTable1.init((hashTable2.bitmask + 1) * 2);
    resize_pos = 0;
}

void HashMap::processResize() {
    if (!hashTable2.table) return;

    ull work_left = kResizingWorkload;
    while (work_left > 0 && hashTable2.table_size.load(std::memory_order_relaxed) > 0) {
        // ull index = resize_pos.fetch_add(1, std::memory_order_relaxed) & hashTable2.bitmask;

        HashNode** source = hashTable2.table.get() + resize_pos.load(std::memory_order_relaxed);
        if (!*source) {
            resize_pos.fetch_add(1, std::memory_order_relaxed);
            continue;
        }
        hashTable1.insert(hashTable2.extract(source));
        work_left--;
    }

    if (hashTable2.table_size.load(std::memory_order_relaxed) == 0) {
        hashTable2 = HashTable();
    }
}

HashNode* HashMap::search(HashNode* key, bool (*compare)(HashNode*, HashNode*)) {
    processResize();
    HashNode** location = hashTable1.locate(key, compare);
    if (!location) {
        location = hashTable2.locate(key, compare);
    }
    return location ? *location : nullptr;
}

HashNode* HashMap::erase(HashNode* target, bool (*compare)(HashNode*, HashNode*)) {
    processResize();
    HashNode** location = hashTable1.locate(target, compare);
    if (location) {
        return hashTable1.extract(location);
    }
    location = hashTable2.locate(target, compare);
    if (location) {
        return hashTable2.extract(location);
    }
    return nullptr;
}