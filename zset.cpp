#include <functional>
#include <cassert>
#include <cstring>

#include "zset.h"
#include "common.h"

ZNode::ZNode(std::string name, double score)
    : score(score), name(std::move(name)) {
    avl_init(&tree);
    hmap.next = nullptr;
    hmap.hashcode = str_hash(reinterpret_cast<const uint8_t *>(this->name.data()), this->name.size());
}

ZSet::ZSet() = default;

ZSet::~ZSet() {
    clear();
}

bool ZSet::add(const std::string &name, double score) {
    ZNode *existingNode = lookup(name);
    if (existingNode) {
        update(existingNode, score);
        return false;
    }

    auto node = std::make_unique<ZNode>(name, score);
    hmap.insert(&node->hmap);
    addToTree(std::move(node));
    return true;
}

static bool hashNodeCompare(HashNode *node, HashNode *key) {
    ZNode *znode = container_of(node, ZNode, hmap);
    HKey *hkey = container_of(key, HKey, node);
    return znode->name == std::string(hkey->name, hkey->len);
}

ZNode *ZSet::lookup(const std::string &name)  {  // removed const
    if (!tree) return nullptr;

    HKey key;
    key.node.hashcode = str_hash(reinterpret_cast<const uint8_t *>(name.data()), name.size());
    key.name = name.data();
    key.len = name.size();

    HashNode *found = hmap.search(&key.node, &hashNodeCompare);
    return found ? container_of(found, ZNode, hmap) : nullptr;
}



std::unique_ptr<ZNode> ZSet::pop(const std::string &name) {
    if (!tree) return nullptr;

    HKey key;
    key.node.hashcode = str_hash(reinterpret_cast<const uint8_t *>(name.data()), name.size());

    key.name = name.data();
    key.len = name.length();


    HashNode *found = hmap.erase(&key.node, [](HashNode *node, HashNode *key) {
        ZNode *znode = container_of(node, ZNode, hmap);
        HKey *hkey = container_of(key,HKey,node);
        return znode->name == hkey->name; 
    });

    if (!found) return nullptr;

    ZNode *node = container_of(found, ZNode, hmap);
    removeFromTree(node);

    // Return ownership of the node
    return std::unique_ptr<ZNode>(node);
}

ZNode *ZSet::query(double score, const std::string &name) const {
    AVLNode *found = nullptr;
    AVLNode *cur = tree;

    while (cur) {
        ZNode *currentNode = container_of(cur, ZNode, tree);
        if (currentNode->score < score || (currentNode->score == score && currentNode->name < name)) {
            cur = cur->right;
        } else {
            found = cur;
            cur = cur->left;
        }
    }

    return found ? container_of(found, ZNode, tree) : nullptr;
}

void ZSet::clear() {
    if (!tree) return;

    std::function<void(AVLNode *)> dispose = [&](AVLNode *node) {
        if (!node) return;
        dispose(node->left);
        dispose(node->right);
        delete container_of(node, ZNode, tree);
    };

    dispose(tree);
    tree = nullptr;
    hmap.freeUp(); // clear()
}


static bool zless(AVLNode *lhs, double score, const std::string &name) {
    ZNode *zl = container_of(lhs, ZNode, tree);
    if (zl->score != score) {
        return zl->score < score;
    }
    int rv = zl->name.compare(name);
    return rv < 0;
}

static bool zless(AVLNode *lhs, AVLNode *rhs) {
    ZNode *zr = container_of(rhs, ZNode, tree);
    return zless(lhs, zr->score, zr->name);
}

void ZSet::update(ZNode *node, double new_score) {
    if (node->score == new_score) return;

    removeFromTree(node);
    node->score = new_score;
    avl_init(&node->tree);
    addToTree(std::unique_ptr<ZNode>(node));
}

void ZSet::addToTree(std::unique_ptr<ZNode> node) {
    AVLNode *cur = nullptr;
    AVLNode **from = &tree;

    while (*from) {
        cur = *from;
        from = zless(&node->tree, cur) ? &cur->left : &cur->right;
    }

    *from = &node->tree;
    node->tree.parent = cur;
    tree = avl_fix(&node->tree);

    // Transfer ownership to the tree
    node.release();
}

void ZSet::removeFromTree(ZNode *node) {
    tree = avl_del(&node->tree);
}

ZNode* ZSet::offset(ZNode* node, int64_t offset) const {
    if (!node) return nullptr;

    AVLNode* avlNode = avl_offset(&node->tree, offset);
    return avlNode ? container_of(avlNode, ZNode, tree) : nullptr;
}