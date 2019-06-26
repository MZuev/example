#include <cstddef>
#include <initializer_list>
#include <algorithm>

template<class ValueType>
class Set {
private:
    class Node {
    private:
        static bool _less(const ValueType* a, const ValueType* b) {
            if (a == nullptr) {
                return false;
            }
            if (b == nullptr) {
                return true;
            }
            return *a < *b;
        }

        static int _get_height(const Node* v) {
            return v == nullptr ? 0 : static_cast<int>(v->height);
        }

        Node *left;
        Node *right;
        Node *parent;
        size_t height;

        void update_height() {
            height = std::max(_get_height(left), _get_height(right)) + 1;
        }

        void clear() {
            left = right = parent = nullptr;
        }

        Node* rotate_right() {
            Node* q = left;
            left = q->right;
            if (left != nullptr) {
                left->parent = this;
            }
            q->right = this;
            q->parent = parent;
            parent = q;
            this->update_height();
            q->update_height();
            return q;
        }

        Node* rotate_left() {
            Node* q = right;
            right = q->left;
            if (right != nullptr) {
                right->parent = this;
            }
            q->left = this;
            q->parent = parent;
            parent = q;
            this->update_height();
            q->update_height();
            return q;
        }

        Node* _fix_node() {
            update_height();
            int delta_height = _get_height(left) - _get_height(right);
            if (std::abs(delta_height) <= 1) {
                return this;
            }
            if (delta_height == 2) {
                if (_get_height(left->left) >= _get_height(left->right)) {
                    return rotate_right();
                } else {
                    left = left->rotate_left();
                    return rotate_right();
                }
            } else {
                if (_get_height(right->right) >= _get_height(right->left)) {
                    return rotate_left();
                } else {
                    right = right->rotate_right();
                    return rotate_left();
                }
            }
        }

        static void _set_child(Node* new_parent, Node* old_child, Node* new_child) {
            if (new_child != nullptr) {
                new_child->parent = new_parent;
            }
            if (new_parent == nullptr) {
                return;
            }
            if (new_parent->left == old_child) {
                new_parent->left = new_child;
            } else {
                new_parent->right = new_child;
            }
        }

    public:
        ValueType* value;

        Node() : left(nullptr), right(nullptr), parent(nullptr), height(0), value(nullptr) {}

        Node(const ValueType& val, Node* _parent) : Node() {
            parent = _parent;
            height = 1;
            value = new ValueType(val);
        }

        ~Node() {
            delete value;
            delete left;
            delete right;
        }

        const Node* get_min() const {
            const Node* v = this;
            while (v->left != nullptr) {
                v = v->left;
            }
            return v;
        }

        const Node* get_max() const {
            const Node* v = this;
            while (v->right != nullptr) {
                v = v->right;
            }
            return v;
        }

        Node* insert(const ValueType& val, bool* flag_added) {
            if (_less(&val, value)) {
                if (left == nullptr) {
                    left = new Node(val, this);
                    *flag_added = true;
                } else {
                    left = left->insert(val, flag_added);
                    left->parent = this;
                }
            } else if (_less(value, &val)) {
                if (right == nullptr) {
                    right = new Node(val, this);
                    *flag_added = true;
                } else {
                    right = right->insert(val, flag_added);
                    right->parent = this;
                }
            }
            return _fix_node();
        }

        Node* erase(const ValueType& val, Node* &new_root) {
            Node* v = const_cast<Node*>(find(val));
            if (v == nullptr) {
                new_root = this;
                return nullptr;
            }
            if (v->left != nullptr && v->right != nullptr) {
                Node* nxt = const_cast<Node*>(v->next());
                std::swap(nxt->value, v->value);
                v = nxt;
            }
            Node* q = v->parent;
            if (v->left != nullptr) {
                _set_child(q, v, v->left);
                new_root = v->left;
            } else if (v->right != nullptr) {
                _set_child(q, v, v->right);
                new_root = v->right;
            } else {
                _set_child(q, v, nullptr);
            }
            while (q != nullptr) {
                Node* nq = q->parent;
                new_root = q->_fix_node();
                _set_child(nq, q, new_root);
                q = nq;
            }
            v->clear();
            return v;
        }

        const Node* next() const {
            if (right != nullptr) {
                return right->get_min();
            }
            auto v = this;
            while (v->parent != nullptr && v->parent->right == v) {
                v = v->parent;
            }
            return v->parent;
        }

        const Node* prev() const {
            if (left != nullptr) {
                return left->get_max();
            }
            auto v = this;
            while (v->parent != nullptr && v->parent->left == v) {
                v = v->parent;
            }
            return v->parent;
        }

        const Node* find(const ValueType& val) const {
            auto v = lower_bound(this, val);
            if (_less(&val, v->value)) {
                return nullptr;
            }
            return v;
        }

        static const Node* lower_bound(const Node* root, const ValueType& val) {
            const Node* ans;
            while (root != nullptr) {
                if (_less(&val, root->value) || !_less(root->value, &val)) {
                    ans = root;
                    root = root->left;
                } else {
                    root = root->right;
                }
            }
            return ans;
        }
    };

public:
    Set() : root(new Node()), begin_iterator(root), end_iterator(root), size_value(0) {}

    template<class Iter>
    Set(Iter begin, Iter end) : Set() {
        while (begin != end) {
            insert(*begin++);
        }
    }

    Set(std::initializer_list<ValueType> lst) : Set() {
        for (const auto& elem : lst) {
            insert(elem);
        }
    }

    void swap(Set& other) {
        using std::swap;
        swap(root, other.root);
        swap(begin_iterator, other.begin_iterator);
        swap(end_iterator, other.end_iterator);
        swap(size_value, other.size_value);
    }

    Set(const Set& other) : Set() {
        for (const auto& elem : other) {
            insert(elem);
        }
    }

    Set& operator = (const Set& other) {
        if (&other == this) {
            return *this;
        }
        Set new_one(other);
        this->swap(new_one);
        return *this;
    }

    ~Set() {
        delete root;
    }

    size_t size() const {
        return size_value;
    }

    bool empty() const {
        return size_value == 0;
    }

    void insert(const ValueType& val) {
        bool flag_added = false;
        root = root->insert(val, &flag_added);
        if (!flag_added) {
            return;
        }
        ++size_value;
        begin_iterator = iterator(root->get_min());
    }

    void erase(const ValueType& val) {
        Node* v = root->erase(val, root);
        if (v == nullptr) {
            return;
        }
        --size_value;
        delete v;
        begin_iterator = iterator(root->get_min());
        end_iterator = iterator(root->get_max());
    }

    class iterator {
    private:
        const Node* vertex;

    public:
        iterator() : vertex(nullptr) {}

        iterator(const Node* p) : vertex(p) {}

        const ValueType& operator * () const {
            return *vertex->value;
        }

        bool operator == (const iterator& other) const {
            return vertex == other.vertex;
        }

        bool operator != (const iterator& other) const {
            return vertex != other.vertex;
        }

        const ValueType* operator -> () const {
            return vertex->value;
        }

        iterator& operator ++ () {
            vertex = vertex->next();
            return *this;
        }

        iterator operator ++ (int) {
            auto tmp = *this;
            ++*this;
            return tmp;
        }

        iterator& operator -- () {
            vertex = vertex->prev();
            return *this;
        }

        iterator operator -- (int) {
            auto tmp = *this;
            --*this;
            return tmp;
        }
    };

    iterator begin() const {
        return begin_iterator;
    }

    iterator end() const {
        return end_iterator;
    }

    iterator find(const ValueType& val) const {
        auto v = root->find(val);
        if (v == nullptr) {
            return end_iterator;
        }
        return iterator(v);
    }

    iterator lower_bound(const ValueType& val)  const {
        return iterator(Node::lower_bound(root, val));
    }

private:
    Node* root;
    iterator begin_iterator, end_iterator;
    size_t size_value;
};
