#include "heap.h"

std::size_t Heap::parent(std::size_t i) {
    return (i + 1) / 2 - 1;
}

std::size_t Heap::left(std::size_t i) {
    return i * 2 + 1;
}

std::size_t Heap::right(std::size_t i) {
    return i * 2 + 2;
}

void Heap::heap_up(std::vector<HeapItem>& a, std::size_t pos) {
    HeapItem t = a[pos];
    while (pos > 0 && a[parent(pos)].val > t.val) {
        a[pos] = a[parent(pos)];
        if (a[pos].ref) {
            *a[pos].ref = pos;
        }
        pos = parent(pos);
    }
    a[pos] = t;
    if (a[pos].ref) {
        *a[pos].ref = pos;
    }
}

void Heap::heap_down(std::vector<HeapItem>& a, std::size_t pos, std::size_t len) {
    HeapItem t = a[pos];
    while (true) {
        std::size_t l = left(pos);
        std::size_t r = right(pos);
        std::optional<std::size_t> min_pos;
        uint64_t min_val = t.val;

        if (l < len && a[l].val < min_val) {
            min_pos = l;
            min_val = a[l].val;
        }
        if (r < len && a[r].val < min_val) {
            min_pos = r;
        }
        if (!min_pos.has_value()) {
            break;
        }

        a[pos] = a[min_pos.value()];
        if (a[pos].ref) {
            *a[pos].ref = pos;
        }
        pos = min_pos.value();
    }
    a[pos] = t;
    if (a[pos].ref) {
        *a[pos].ref = pos;
    }
}

void Heap::update(std::vector<HeapItem>& a, std::size_t pos) {
    if (pos > 0 && a[parent(pos)].val > a[pos].val) {
        heap_up(a, pos);
    } else {
        heap_down(a, pos, a.size());
    }
}
