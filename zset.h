#pragma once

#include <memory>
#include <string>
#include "avl.h"
#include "hashtable.h"


struct HKey {
    HashNode node;
    const char *name = nullptr;
    size_t len = 0;
};


class ZNode {
public:
    ZNode(std::string name, double score);

    ZNode(const ZNode &) = delete;
    ZNode &operator=(const ZNode &) = delete;
    ZNode(ZNode &&) = delete;
    ZNode &operator=(ZNode &&) = delete;

    double getScore() const { return score; }
    const std::string &getName() const { return name; }

    friend class ZSet;

    AVLNode tree;
    HashNode hmap;
    double score;
    std::string name;
};

class ZSet {
public:
    ZSet();
    ~ZSet();

    bool add(const std::string &name, double score);
    ZNode *lookup(const std::string &name); // removed const
    std::unique_ptr<ZNode> pop(const std::string &name);
    ZNode *query(double score, const std::string &name) const;
    ZNode* offset(ZNode* node, int64_t offset) const;
    void clear();

    ZSet(const ZSet &) = delete;
    ZSet &operator=(const ZSet &) = delete;

    void update(ZNode *node, double new_score);
    void addToTree(std::unique_ptr<ZNode> node);
    void removeFromTree(ZNode *node);

    AVLNode *tree = nullptr;
    HashMap hmap;
};

