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

        static int _get_h(const Node* v) {
            return v == nullptr ? 0 : static_cast<int>(v->h);
        }   

        Node *l, *r, *p;
        size_t h;

        void upd() {
            h = std::max(_get_h(l), _get_h(r)) + 1;
        }

        void clear() {
            l = r = p = nullptr;
        }

        Node* rotate_right() {
            Node* q = l;
            l = q->r;
            if (l != nullptr) {
                l->p = this;
            }
            q->r = this;
            q->p = p;
            p = q;
            this->upd();
            q->upd();
            return q;
        }

        Node* rotate_left() {
            Node* q = r;
            r = q->l;
            if (r != nullptr) {
                r->p = this;
            }
            q->l = this;
            q->p = p;
            p = q;
            this->upd();
            q->upd();
            return q;
        }

        Node* _fix_node() {
            upd();
            int dh = _get_h(l) - _get_h(r);
            if (std::abs(dh) <= 1) {
                return this;
            }
            if (dh == 2) {
                if (_get_h(l->l) >= _get_h(l->r)) {
                    return rotate_right();
                } else {
                    l = l->rotate_left();
                    return rotate_right();
                }
            } else {
                if (_get_h(r->r) >= _get_h(r->l)) {
                    return rotate_left();
                } else {
                    r = r->rotate_right();
                    return rotate_left();
                }
            }
        }

        static void _set_child(Node* par, Node* old_child, Node* new_child) {
            if (new_child != nullptr) {
                new_child->p = par;
            }
            if (par == nullptr) {
                return;
            }
            if (par->l == old_child) {
                par->l = new_child;
            } else {
                par->r = new_child;
            }
        }

    public:
        ValueType* value;

        Node() : l(nullptr), r(nullptr), p(nullptr), h(0), value(nullptr) {}

        Node(const ValueType& val, Node* _p) : l(nullptr), r(nullptr), p(_p), h(1), value(new ValueType(val)) {}

        ~Node() {
            delete value;
            delete l;
            delete r;
        }

        const Node* get_min() const {
            const Node* v = this;
            while (v->l != nullptr) {
                v = v->l;
            }
            return v;
        }

        const Node* get_max() const {
            const Node* v = this;
            while (v->r != nullptr) {
                v = v->r;
            }
            return v;
        }

        Node* insert(const ValueType& val, bool& flag_added) {
            if (_less(&val, value)) {
                if (l == nullptr) {
                    l = new Node(val, this);
                    flag_added = true;
                } else {
                    l = l->insert(val, flag_added);
                    l->p = this;
                }
            } else if (_less(value, &val)) {
                if (r == nullptr) {
                    r = new Node(val, this);
                    flag_added = true;
                } else {
                    r = r->insert(val, flag_added);
                    r->p = this;
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
            if (v->l != nullptr && v->r != nullptr) {
                Node* nxt = const_cast<Node*>(v->next());
                std::swap(nxt->value, v->value);
                v = nxt;
            }
            Node* q = v->p;
            if (v->l != nullptr) {
                _set_child(q, v, v->l);
                new_root = v->l;
            } else if (v->r != nullptr) {
                _set_child(q, v, v->r);
                new_root = v->r;
            } else {
                _set_child(q, v, nullptr);
            }
            while (q != nullptr) {
                Node* nq = q->p;
                new_root = q->_fix_node();
                _set_child(nq, q, new_root);
                q = nq;
            }
            v->clear();
            return v;
        }



        const Node* next() const {
            if (r != nullptr) {
                return r->get_min();
            }
            auto v = this;
            while (v->p != nullptr && v->p->r == v) {
                v = v->p;
            }
            return v->p;
        }

        const Node* prev() const {
            if (l != nullptr) {
                return l->get_max();
            }
            auto v = this;
            while (v->p != nullptr && v->p->l == v) {
                v = v->p;
            }
            return v->p;
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
                    root = root->l;
                } else {
                    root = root->r;
                }
            }
            return ans;
        }
    };

public:
    Set() : root(new Node()), _beg(root), _end(root), __size(0) {}

    template<class Iter>
    Set(Iter beg, Iter end) : root(new Node()), _beg(root), _end(root), __size(0) {
        while (beg != end) {
            insert(*beg++);
        }
    }

    Set(std::initializer_list<ValueType> l) : root(new Node()), _beg(root), _end(root), __size(0) {
        for (const auto& v : l) {
            insert(v);
        }
    }

    void swap(Set& other) {
        using std::swap;
        swap(root, other.root);
        swap(_beg, other._beg);
        swap(_end, other._end);
        swap(__size, other.__size);
    }

    Set(const Set& other) : root(new Node()), _beg(root), _end(root), __size(0) {
        for (const auto& v : other) {
            insert(v);
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
        return __size;
    }

    bool empty() const {
        return __size == 0;
    }

    void insert(const ValueType& val) {
        bool flag_added = false;
        root = root->insert(val, flag_added);
        if (flag_added) {
            ++__size;
        }
        _beg = iterator(root->get_min());
    }

    void erase(const ValueType& val) {
        Node* v = root->erase(val, root);
        if (v != nullptr) {
            --__size;
        }
        delete v;
        _beg = iterator(root->get_min());
        _end = iterator(root->get_max());
    }

    class iterator {
    private:
        const Node* v;

    public:
        iterator() : v(nullptr) {}

        iterator(const Node* p) : v(p) {}
        
        const ValueType& operator * () const {
            return *v->value;
        }

        bool operator == (const iterator& other) const {
            return v == other.v;
        }

        bool operator != (const iterator& other) const {
            return v != other.v;
        }

        const ValueType* operator -> () const {
            return v->value;
        }

        iterator& operator ++ () {
            v = v->next();
            return *this;
        }

        iterator operator ++ (int) {
            auto tmp = *this;
            ++(*this);
            return tmp;
        }

        iterator& operator -- () {
            v = v->prev();
            return *this;
        }

        iterator operator -- (int) {
            auto tmp = *this;
            --(*this);
            return tmp;
        }
    };
    
    iterator begin() const {
        return _beg;
    }

    iterator end() const {
        return _end;
    }

    iterator find(const ValueType& val) const {
        auto v = root->find(val);
        if (v == nullptr) {
            return _end;
        }
        return iterator(v);
    }

    iterator lower_bound(const ValueType& val) const {
        return iterator(Node::lower_bound(root, val));
    }

private:
        Node* root;
        iterator _beg, _end;
        size_t __size;
};
