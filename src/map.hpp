/**
 * implement a container like std::map
 */
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
    class Key,
    class T,
    class Compare = std::less<Key>
> class map {
public:
    /**
     * the internal type of data.
     * it should have a default constructor, a copy constructor.
     * You can use sjtu::map as value_type by typedef.
     */
    typedef pair<const Key, T> value_type;

private:
    enum Color { RED, BLACK };
    
    struct Node {
        value_type *data;
        Node *left, *right, *parent;
        Color color;
        bool is_nil;
        
        Node(const value_type &val, Node *p = nullptr) 
            : data(new value_type(val)), left(nullptr), right(nullptr), 
              parent(p), color(RED), is_nil(false) {}
        
        // Constructor for nil node (no data)
        Node() : data(nullptr), left(nullptr), right(nullptr), 
                 parent(nullptr), color(BLACK), is_nil(true) {}
        
        ~Node() {
            if (!is_nil) delete data;
        }
    };
    
    Node *root;
    Node *nil;  // sentinel node
    size_t count_;
    Compare comp;
    
    void initNil() {
        nil = new Node(value_type(Key(), T()), nullptr);
        nil->color = BLACK;
        nil->left = nil->right = nil->parent = nil;
    }
    
    void destroyTree(Node *node) {
        if (node == nil) return;
        destroyTree(node->left);
        destroyTree(node->right);
        delete node;
    }
    
    Node* copyTree(Node *node, Node *other_nil, Node *p = nullptr) {
        if (node == other_nil) return nil;
        Node *newNode = new Node(*(node->data), p);
        newNode->color = node->color;
        newNode->left = copyTree(node->left, other_nil, newNode);
        newNode->right = copyTree(node->right, other_nil, newNode);
        return newNode;
    }
    
    void leftRotate(Node *x) {
        Node *y = x->right;
        x->right = y->left;
        if (y->left != nil) {
            y->left->parent = x;
        }
        y->parent = x->parent;
        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        y->left = x;
        x->parent = y;
    }
    
    void rightRotate(Node *y) {
        Node *x = y->left;
        y->left = x->right;
        if (x->right != nil) {
            x->right->parent = y;
        }
        x->parent = y->parent;
        if (y->parent == nil) {
            root = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
        x->right = y;
        y->parent = x;
    }
    
    void insertFixup(Node *z) {
        while (z->parent->color == RED) {
            if (z->parent == z->parent->parent->left) {
                Node *y = z->parent->parent->right;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                Node *y = z->parent->parent->left;
                if (y->color == RED) {
                    z->parent->color = BLACK;
                    y->color = BLACK;
                    z->parent->parent->color = RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = BLACK;
                    z->parent->parent->color = RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = BLACK;
    }
    
    void transplant(Node *u, Node *v) {
        if (u->parent == nil) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }
    
    Node* minimum(Node *node) const {
        while (node->left != nil) {
            node = node->left;
        }
        return node;
    }
    
    Node* maximum(Node *node) const {
        while (node->right != nil) {
            node = node->right;
        }
        return node;
    }
    
    void deleteFixup(Node *x) {
        while (x != root && x->color == BLACK) {
            if (x == x->parent->left) {
                Node *w = x->parent->right;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == BLACK && w->right->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->right->color == BLACK) {
                        w->left->color = BLACK;
                        w->color = RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->right->color = BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                Node *w = x->parent->left;
                if (w->color == RED) {
                    w->color = BLACK;
                    x->parent->color = RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == BLACK && w->left->color == BLACK) {
                    w->color = RED;
                    x = x->parent;
                } else {
                    if (w->left->color == BLACK) {
                        w->right->color = BLACK;
                        w->color = RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = BLACK;
                    w->left->color = BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = BLACK;
    }
    
    Node* findNode(const Key &key) const {
        Node *current = root;
        while (current != nil) {
            if (comp(key, current->data->first)) {
                current = current->left;
            } else if (comp(current->data->first, key)) {
                current = current->right;
            } else {
                return current;
            }
        }
        return nil;
    }

public:
    class const_iterator;
    class iterator {
        friend class map;
        friend class const_iterator;
    private:
        Node *node_;
        const map *map_;
        
    public:
        iterator() : node_(nullptr), map_(nullptr) {}
        
        iterator(Node *node, const map *m) : node_(node), map_(m) {}
        
        iterator(const iterator &other) : node_(other.node_), map_(other.map_) {}
        
        iterator operator++(int) {
            iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        iterator &operator++() {
            if (node_ == map_->nil) {
                throw invalid_iterator();
            }
            if (node_->right != map_->nil) {
                node_ = map_->minimum(node_->right);
            } else {
                Node *y = node_->parent;
                while (y != map_->nil && node_ == y->right) {
                    node_ = y;
                    y = y->parent;
                }
                node_ = y;
            }
            return *this;
        }
        
        iterator operator--(int) {
            iterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        iterator &operator--() {
            if (node_ == map_->nil) {
                // Move to the maximum element
                if (map_->root == map_->nil) {
                    throw invalid_iterator();
                }
                node_ = map_->maximum(map_->root);
            } else {
                if (node_->left != map_->nil) {
                    node_ = map_->maximum(node_->left);
                } else {
                    Node *y = node_->parent;
                    while (y != map_->nil && node_ == y->left) {
                        node_ = y;
                        y = y->parent;
                    }
                    if (y == map_->nil) {
                        throw invalid_iterator();
                    }
                    node_ = y;
                }
            }
            return *this;
        }
        
        value_type &operator*() const {
            return *(node_->data);
        }
        
        bool operator==(const iterator &rhs) const {
            return node_ == rhs.node_ && map_ == rhs.map_;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return node_ == rhs.node_ && map_ == rhs.map_;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
        
        value_type *operator->() const noexcept {
            return node_->data;
        }
    };
    
    class const_iterator {
        friend class map;
        friend class iterator;
    private:
        Node *node_;
        const map *map_;
        
    public:
        const_iterator() : node_(nullptr), map_(nullptr) {}
        
        const_iterator(Node *node, const map *m) : node_(node), map_(m) {}
        
        const_iterator(const const_iterator &other) : node_(other.node_), map_(other.map_) {}
        
        const_iterator(const iterator &other) : node_(other.node_), map_(other.map_) {}
        
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++(*this);
            return tmp;
        }
        
        const_iterator &operator++() {
            if (node_ == map_->nil) {
                throw invalid_iterator();
            }
            if (node_->right != map_->nil) {
                node_ = map_->minimum(node_->right);
            } else {
                Node *y = node_->parent;
                while (y != map_->nil && node_ == y->right) {
                    node_ = y;
                    y = y->parent;
                }
                node_ = y;
            }
            return *this;
        }
        
        const_iterator operator--(int) {
            const_iterator tmp = *this;
            --(*this);
            return tmp;
        }
        
        const_iterator &operator--() {
            if (node_ == map_->nil) {
                if (map_->root == map_->nil) {
                    throw invalid_iterator();
                }
                node_ = map_->maximum(map_->root);
            } else {
                if (node_->left != map_->nil) {
                    node_ = map_->maximum(node_->left);
                } else {
                    Node *y = node_->parent;
                    while (y != map_->nil && node_ == y->left) {
                        node_ = y;
                        y = y->parent;
                    }
                    if (y == map_->nil) {
                        throw invalid_iterator();
                    }
                    node_ = y;
                }
            }
            return *this;
        }
        
        const value_type &operator*() const {
            return *(node_->data);
        }
        
        bool operator==(const iterator &rhs) const {
            return node_ == rhs.node_ && map_ == rhs.map_;
        }
        
        bool operator==(const const_iterator &rhs) const {
            return node_ == rhs.node_ && map_ == rhs.map_;
        }
        
        bool operator!=(const iterator &rhs) const {
            return !(*this == rhs);
        }
        
        bool operator!=(const const_iterator &rhs) const {
            return !(*this == rhs);
        }
        
        const value_type *operator->() const noexcept {
            return node_->data;
        }
    };
    
    map() : root(nullptr), nil(nullptr), count_(0) {
        nil = new Node();  // nil node with no data
        nil->left = nil->right = nil->parent = nil;
        root = nil;
    }
    
    map(const map &other) : count_(other.count_), comp(other.comp) {
        nil = new Node();  // nil node with no data
        nil->left = nil->right = nil->parent = nil;
        root = copyTree(other.root, other.nil, nil);
    }
    
    map &operator=(const map &other) {
        if (this == &other) return *this;
        clear();
        count_ = other.count_;
        comp = other.comp;
        root = copyTree(other.root, other.nil, nil);
        return *this;
    }
    
    ~map() {
        clear();
        delete nil;
    }
    
    T &at(const Key &key) {
        Node *node = findNode(key);
        if (node == nil) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }
    
    const T &at(const Key &key) const {
        Node *node = findNode(key);
        if (node == nil) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }
    
    T &operator[](const Key &key) {
        Node *node = findNode(key);
        if (node != nil) {
            return node->data->second;
        }
        // Insert new element with default-constructed value
        auto result = insert(value_type(key, T()));
        return result.first->second;
    }
    
    const T &operator[](const Key &key) const {
        Node *node = findNode(key);
        if (node == nil) {
            throw index_out_of_bound();
        }
        return node->data->second;
    }
    
    iterator begin() {
        if (root == nil) return iterator(nil, this);
        return iterator(minimum(root), this);
    }
    
    const_iterator cbegin() const {
        if (root == nil) return const_iterator(nil, this);
        return const_iterator(minimum(root), this);
    }
    
    iterator end() {
        return iterator(nil, this);
    }
    
    const_iterator cend() const {
        return const_iterator(nil, this);
    }
    
    bool empty() const {
        return count_ == 0;
    }
    
    size_t size() const {
        return count_;
    }
    
    void clear() {
        destroyTree(root);
        root = nil;
        count_ = 0;
    }
    
    pair<iterator, bool> insert(const value_type &value) {
        Node *y = nil;
        Node *x = root;
        
        while (x != nil) {
            y = x;
            if (comp(value.first, x->data->first)) {
                x = x->left;
            } else if (comp(x->data->first, value.first)) {
                x = x->right;
            } else {
                // Key already exists
                return pair<iterator, bool>(iterator(x, this), false);
            }
        }
        
        Node *z = new Node(value, y);
        
        if (y == nil) {
            root = z;
        } else if (comp(z->data->first, y->data->first)) {
            y->left = z;
        } else {
            y->right = z;
        }
        
        z->left = nil;
        z->right = nil;
        z->color = RED;
        count_++;
        
        insertFixup(z);
        
        return pair<iterator, bool>(iterator(z, this), true);
    }
    
    void erase(iterator pos) {
        if (pos.map_ != this || pos.node_ == nil) {
            throw invalid_iterator();
        }
        
        Node *z = pos.node_;
        Node *y = z;
        Node *x;
        Color y_original_color = y->color;
        
        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            y_original_color = y->color;
            x = y->right;
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        
        delete z;
        count_--;
        
        if (y_original_color == BLACK) {
            deleteFixup(x);
        }
    }
    
    size_t count(const Key &key) const {
        return findNode(key) != nil ? 1 : 0;
    }
    
    iterator find(const Key &key) {
        Node *node = findNode(key);
        return iterator(node, this);
    }
    
    const_iterator find(const Key &key) const {
        Node *node = findNode(key);
        return const_iterator(node, this);
    }
};

}

#endif
