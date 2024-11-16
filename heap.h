#ifndef HEAP_H
#define HEAP_H

#include <vector>
#include <cstddef>
#include <cstdint>
#include <optional>

// Represents an item in the heap
struct HeapItem {
    uint64_t val = 0;          // Value of the heap item
    std::size_t* ref = nullptr; // Pointer to the index of the item in the heap
};

// Class to manage a min-heap
class Heap {
public:
    // Updates the heap starting from position `pos`
    static void update(std::vector<HeapItem>& a, std::size_t pos);

private:
    // Returns the index of the parent of the given index
    static std::size_t parent(std::size_t i);

    // Returns the index of the left child of the given index
    static std::size_t left(std::size_t i);

    // Returns the index of the right child of the given index
    static std::size_t right(std::size_t i);

    // Performs the heap-up operation to maintain heap properties
    static void heap_up(std::vector<HeapItem>& a, std::size_t pos);

    // Performs the heap-down operation to maintain heap properties
    static void heap_down(std::vector<HeapItem>& a, std::size_t pos, std::size_t len);
};

#endif // HEAP2_H
