/****************************************************************************
 * Copyright (C) from 2009 to Present EPAM Systems.
 *
 * This file is part of Indigo toolkit.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 ***************************************************************************/

#ifndef __red_black_h__
#define __red_black_h__

#include <string.h>

#include "base_cpp/obj_pool.h"
#include "base_cpp/string_pool.h"

namespace indigo
{

    struct RedBlackNodeBase
    {
        int left;
        int right;
        int parent;
        int color;
    };

    DECL_EXCEPTION(RedBlackTreeError);

    template <typename Key, typename Node>
    class RedBlackTree
    {
    public:
        DECL_TPL_ERROR(RedBlackTreeError);

        enum
        {
            RED = 0,
            BLACK = 1
        };

        RedBlackTree()
        {
            _size = 0;
            _root = -1;
            _nodes = new Pool<Node>();
            _own_nodes = true;
        }

        explicit RedBlackTree(Pool<Node>& nodes)
        {
            _size = 0;
            _root = -1;
            _nodes = &nodes;
            _own_nodes = false;
        }

        virtual ~RedBlackTree()
        {
            clear();

            if (_own_nodes)
            {
                delete _nodes;
                _nodes = nullptr;
            }
        }

        virtual void clear()
        {
            if (_own_nodes)
                _nodes->clear();
            else if (_size > 0)
            {
                int i = beginPost();

                while (1)
                {
                    int inext = nextPost(i);

                    // end() can change after _nodes->remove()
                    if (inext == end())
                    {
                        _nodes->remove(i);
                        break;
                    }

                    _nodes->remove(i);
                    i = inext;
                }
            }
            _root = -1;
            _size = 0;
        }

        int size() const
        {
            return _size;
        }

        int begin() const
        {
            // inorder traversal: find the first element
            int node = _root;
            int parent = end();

            while (node != -1)
            {
                parent = node;
                node = _nodes->at(node).left;
            }

            return parent;
        }

        int beginPost() const
        {
            // postorder traversal: find the first element
            int node = _root;
            int parent = end();

            while (node != -1)
            {
                parent = node;
                if (_nodes->at(node).left != -1)
                    node = _nodes->at(node).left;
                else
                    node = _nodes->at(node).right;
            }

            return parent;
        }

        int next(int node) const
        {
            // inorder traversal: find the next element

            if (_nodes->at(node).right >= 0)
            {
                // go right and then left while possible
                node = _nodes->at(node).right;
                while (_nodes->at(node).left >= 0)
                    node = _nodes->at(node).left;
                return node;
            }

            while (1)
            {
                int parent = _nodes->at(node).parent;

                if (parent == -1)
                    break;

                // go up while going from the right
                if (_nodes->at(parent).left == node)
                    return parent;

                node = parent;
            }

            return end();
        }

        int nextPost(int node) const
        {
            // postorder traversal: find the next element
            int parent = _nodes->at(node).parent;

            if (parent == -1)
                return end();

            int parent_right = _nodes->at(parent).right;

            if (parent_right == node || parent_right == -1)
                return parent;

            node = parent_right;
            parent = node;

            while (node != -1)
            {
                parent = node;
                if (_nodes->at(node).left != -1)
                    node = _nodes->at(node).left;
                else
                    node = _nodes->at(node).right;
            }

            return parent;
        }

        int end() const
        {
            return _nodes->end();
        }

        bool find(Key key) const
        {
            int sign, idx = this->_findClosest(key, sign);

            return idx != -1 && sign == 0;
        }

    protected:
        virtual int _compare(Key key, const Node& node) const = 0;

        int _findClosest(Key key, int& sign) const
        {
            int idx = _root;
            int parent = -1;

            sign = 0;

            while (idx != -1)
            {
                const Node& node = _nodes->at(idx);
                sign = _compare(key, node);

                if (sign == 0)
                    return idx;

                parent = idx;

                if (sign > 0)
                    idx = node.right;
                else // sign < 0
                    idx = node.left;
            }
            return parent;
        }

        void _insertNode(int node_idx, int parent_idx, int sign)
        {
            Node& node = _nodes->at(node_idx);

            node.left = -1;
            node.right = -1;
            node.color = RED;
            node.parent = parent_idx;

            if (parent_idx == -1)
                _root = node_idx;
            else if (sign < 0)
                _nodes->at(parent_idx).left = node_idx;
            else // sign > 0
                _nodes->at(parent_idx).right = node_idx;

            while (node_idx != _root)
            {
                parent_idx = _nodes->at(node_idx).parent;
                Node& parent = _nodes->at(parent_idx);

                if (parent.color == BLACK)
                    break;

                int grandparent_idx = parent.parent;
                Node& grandparent = _nodes->at(grandparent_idx);

                if (parent_idx == grandparent.left)
                {
                    int uncle_color = BLACK;

                    if (grandparent.right >= 0)
                        uncle_color = _nodes->at(grandparent.right).color;

                    if (uncle_color == RED)
                    {
                        parent.color = BLACK;
                        _nodes->at(grandparent.right).color = BLACK;
                        grandparent.color = RED;
                        node_idx = grandparent_idx;
                    }
                    else // uncle_color == BLACK
                    {
                        if (node_idx == parent.right)
                        {
                            node_idx = parent_idx;
                            _rotateLeft(node_idx);
                        }
                        parent_idx = _nodes->at(node_idx).parent;
                        grandparent_idx = _nodes->at(parent_idx).parent;
                        _nodes->at(parent_idx).color = BLACK;
                        _nodes->at(grandparent_idx).color = RED;
                        _rotateRight(grandparent_idx);
                    }
                }
                else // parent_idx == grandparent.right
                {
                    int uncle_color = BLACK;

                    if (grandparent.left >= 0)
                        uncle_color = _nodes->at(grandparent.left).color;

                    if (uncle_color == RED)
                    {
                        parent.color = BLACK;
                        _nodes->at(grandparent.left).color = BLACK;
                        grandparent.color = RED;
                        node_idx = grandparent_idx;
                    }
                    else // uncle_color == BLACK
                    {
                        if (node_idx == parent.left)
                        {
                            node_idx = parent_idx;
                            _rotateRight(node_idx);
                        }
                        parent_idx = _nodes->at(node_idx).parent;
                        grandparent_idx = _nodes->at(parent_idx).parent;
                        _nodes->at(parent_idx).color = BLACK;
                        _nodes->at(grandparent_idx).color = RED;
                        _rotateLeft(grandparent_idx);
                    }
                }
            }
            _nodes->at(_root).color = BLACK;
            _size++;
        }

        void _rotateLeft(int node_idx)
        {
            Node& node = _nodes->at(node_idx);
            int right_idx = node.right;
            Node& right = _nodes->at(right_idx);

            node.right = right.left;

            if (right.left != -1)
                _nodes->at(right.left).parent = node_idx;

            right.parent = node.parent;

            if (node.parent == -1)
                _root = right_idx;
            else
            {
                Node& parent = _nodes->at(node.parent);

                if (node_idx == parent.left)
                    parent.left = right_idx;
                else // node_idx == parent.right
                    parent.right = right_idx;
            }

            right.left = node_idx;
            node.parent = right_idx;
        }

        void _rotateRight(int node_idx)
        {
            Node& node = _nodes->at(node_idx);
            int left_idx = node.left;
            Node& left = _nodes->at(left_idx);

            node.left = left.right;

            if (left.right != -1)
                _nodes->at(left.right).parent = node_idx;

            left.parent = node.parent;

            if (node.parent == -1)
                _root = left_idx;
            else
            {
                Node& parent = _nodes->at(node.parent);

                if (node_idx == parent.left)
                    parent.left = left_idx;
                else // node_idx == parent.right
                    parent.right = left_idx;
            }

            left.right = node_idx;
            node.parent = left_idx;
        }

        void _removeNode(int z)
        {
            int x, y;

            if (_nodes->at(z).left == -1)
            {
                y = z;
                x = _nodes->at(y).right;
            }
            else if (_nodes->at(z).right == -1)
            {
                y = z;
                x = _nodes->at(z).left;
            }
            else
            {
                y = _nodes->at(z).right;

                while (_nodes->at(y).left != -1)
                    y = _nodes->at(y).left;

                x = _nodes->at(y).right;
            }

            // replace y with x
            int yparent = _nodes->at(y).parent;

            if (x != -1)
                _nodes->at(x).parent = yparent;

            if (yparent != -1)
            {
                if (y == _nodes->at(yparent).left)
                    _nodes->at(yparent).left = x;
                else
                    _nodes->at(yparent).right = x;
            }
            else
                _root = x;

            int ycolor = _nodes->at(y).color;

            if (y != z)
            {
                // replace z with y
                if (z == yparent)
                    yparent = y;
                int zparent = _nodes->at(z).parent;
                _nodes->at(y).parent = zparent;
                if (zparent != -1)
                {
                    if (_nodes->at(zparent).left == z)
                        _nodes->at(zparent).left = y;
                    else
                        _nodes->at(zparent).right = y;
                }
                else
                    _root = y;
                _nodes->at(y).left = _nodes->at(z).left;
                _nodes->at(y).right = _nodes->at(z).right;
                if (_nodes->at(z).left != -1)
                    _nodes->at(_nodes->at(z).left).parent = y;
                if (_nodes->at(z).right != -1)
                    _nodes->at(_nodes->at(z).right).parent = y;
                _nodes->at(y).color = _nodes->at(z).color;
            }

            if (ycolor == BLACK)
                _removeFixup(x, yparent);

            _nodes->remove(z);
            _size--;
        }

        void _removeFixup(int x, int xparent)
        {
            while (x != _root && (x == -1 || _nodes->at(x).color == BLACK))
            {
                if (_nodes->at(xparent).left == x)
                {
                    int w = _nodes->at(xparent).right;

                    if (_nodes->at(w).color == RED)
                    {
                        _nodes->at(w).color = BLACK;
                        _nodes->at(xparent).color = RED;
                        _rotateLeft(xparent);
                        w = _nodes->at(xparent).right;
                    }

                    Node& wnode = _nodes->at(w);

                    if ((wnode.left == -1 || _nodes->at(wnode.left).color == BLACK) && (wnode.right == -1 || _nodes->at(wnode.right).color == BLACK))
                    {
                        wnode.color = RED;
                        x = xparent;
                        xparent = _nodes->at(x).parent;
                    }
                    else
                    {
                        if (wnode.right == -1 || _nodes->at(wnode.right).color == BLACK)
                        {
                            if (wnode.left != -1)
                                _nodes->at(wnode.left).color = BLACK;
                            wnode.color = RED;
                            _rotateRight(w);
                            w = _nodes->at(xparent).right;
                        }

                        _nodes->at(w).color = _nodes->at(xparent).color;
                        _nodes->at(xparent).color = BLACK;
                        if (_nodes->at(w).right != -1)
                            _nodes->at(_nodes->at(w).right).color = BLACK;
                        _rotateLeft(xparent);
                        x = _root;
                    }
                }
                else
                {
                    int w = _nodes->at(xparent).left;

                    if (_nodes->at(w).color == RED)
                    {
                        _nodes->at(w).color = BLACK;
                        _nodes->at(xparent).color = RED;
                        _rotateRight(xparent);
                        w = _nodes->at(xparent).left;
                    }

                    Node& wnode = _nodes->at(w);

                    if ((wnode.left == -1 || _nodes->at(wnode.left).color == BLACK) && (wnode.right == -1 || _nodes->at(wnode.right).color == BLACK))
                    {
                        wnode.color = RED;
                        x = xparent;
                        xparent = _nodes->at(x).parent;
                    }
                    else
                    {
                        if (wnode.left == -1 || _nodes->at(wnode.left).color == BLACK)
                        {
                            _nodes->at(wnode.right).color = BLACK;
                            wnode.color = RED;
                            _rotateLeft(w);
                            w = _nodes->at(xparent).left;
                        }

                        _nodes->at(w).color = _nodes->at(xparent).color;
                        _nodes->at(xparent).color = BLACK;
                        if (_nodes->at(w).left != -1)
                            _nodes->at(_nodes->at(w).left).color = BLACK;
                        _rotateRight(xparent);
                        x = _root;
                    }
                }
            }
            if (x != -1)
                _nodes->at(x).color = BLACK;
        }

        Pool<Node>* _nodes;
        int _root;
        bool _own_nodes;
        int _size;

    private:
        RedBlackTree(const RedBlackTree&); // no implicit copy
    };

    template <typename Key>
    struct RedBlackSetNode : public RedBlackNodeBase
    {
        Key key;
    };

    template <typename Key>
    class RedBlackSet : public RedBlackTree<Key, RedBlackSetNode<Key>>
    {
        typedef RedBlackTree<Key, RedBlackSetNode<Key>> Parent;

    public:
        typedef RedBlackSetNode<Key> Node;

        RedBlackSet()
        {
        }

        RedBlackSet(Pool<Node>& pool) : Parent(pool)
        {
        }

        ~RedBlackSet() override
        {
        }

        bool find(Key key) const
        {
            int sign, idx = this->_findClosest(key, sign);

            return idx != -1 && sign == 0;
        }

        int insert(Key key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key already present");

            return _insert(key, idx, sign);
        }

        bool find_or_insert(Key key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return true;

            _insert(key, idx, sign);

            return false;
        }

        Key& key(int node) const
        {
            return this->_nodes->at(node).key;
        }

        void remove(Key key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                this->_removeNode(idx);
            else
                throw typename Parent::Error("remove(): key not found");
        }

        void remove_if_exists(Key key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                this->_removeNode(idx);
        }

    protected:
        int _compare(Key key, const Node& node) const override
        {
            return key > node.key ? 1 : (key < node.key ? -1 : 0);
        }

        int _insert(Key key, int parent, int sign)
        {
            int node_idx = this->_nodes->add();
            Node& node = this->_nodes->at(node_idx);

            node.key = key;

            this->_insertNode(node_idx, parent, sign);

            return node_idx;
        }

    private:
        RedBlackSet(const RedBlackSet&); // no implicit copy
    };

    template <typename Key, typename Value>
    struct RedBlackMapNode : public RedBlackNodeBase
    {
        Key key;
        Value value;
    };

    template <typename Key, typename Value>
    class RedBlackMap : public RedBlackTree<Key, RedBlackMapNode<Key, Value>>
    {
        typedef RedBlackTree<Key, RedBlackMapNode<Key, Value>> Parent;

    public:
        RedBlackMap()
        {
        }

        ~RedBlackMap() override
        {
        }

        typedef RedBlackMapNode<Key, Value> Node;

        Value& at(Key key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return this->_nodes->at(idx).value;

            throw typename Parent::Error("at(): key not found");
        }

        Value* at2(Key key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return &this->_nodes->at(idx).value;

            return 0;
        }

        void insert(Key key, Value value)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key already present");

            _insert(key, value, idx, sign);
        }

        void remove(Key key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                this->_removeNode(idx);
            else
                throw typename Parent::Error("remove(): key not found");
        }

        void copy(const RedBlackMap& other)
        {
            int i;

            this->clear();

            for (i = other.begin(); i != other.end(); i = other.next(i))
                this->insert(other.key(i), other.value(i));
        }

        Key& key(int node) const
        {
            return this->_nodes->at(node).key;
        }

        Value& value(int node) const
        {
            return this->_nodes->at(node).value;
        }

    protected:
        int _compare(Key key, const Node& node) const override
        {
            if (key < node.key)
                return -1;
            if (node.key < key)
                return 1;
            return 0;
        }

        void _insert(Key key, Value value, int parent, int sign)
        {
            int node_idx = this->_nodes->add();
            Node& node = this->_nodes->at(node_idx);

            node.key = key;
            node.value = value;

            this->_insertNode(node_idx, parent, sign);
        }

    private:
        RedBlackMap(const RedBlackMap&); // no implicit copy
    };

    template <typename Value>
    struct RedBlackStringMapNode : public RedBlackNodeBase
    {
        int key_idx;
        Value value;
    };

    template <typename Value, bool case_sensitive = true>
    class RedBlackStringMap : public RedBlackTree<const char*, RedBlackStringMapNode<Value>>
    {
        typedef RedBlackTree<const char*, RedBlackStringMapNode<Value>> Parent;

    public:
        typedef RedBlackStringMapNode<Value> Node;

        void clear() override
        {
            RedBlackTree<const char*, Node>::clear();
            _pool.clear();
        }

        void insert(const char* key, Value value)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key %s already present", key);

            _insert(key, value, idx, sign);
        }

        Value& at(const char* key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return this->_nodes->at(idx).value;

            throw typename Parent::Error("at(): key %s not found", key);
        }

        Value& at(const char* key)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return this->_nodes->at(idx).value;

            throw typename Parent::Error("at(): key %s not found", key);
        }

        Value* at2(const char* key)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return &this->_nodes->at(idx).value;

            return 0;
        }

        const char* key(int node) const
        {
            return _pool.at(this->_nodes->at(node).key_idx);
        }

        Value& value(int node) const
        {
            return this->_nodes->at(node).value;
        }

    protected:
        int _compare(const char* key, const RedBlackStringMapNode<Value>& node) const override
        {
            return case_sensitive ? strcmp(key, _pool.at(node.key_idx)) : strcasecmp(key, _pool.at(node.key_idx));
        }

        void _insert(const char* key, Value value, int parent, int sign)
        {
            int string_idx = _pool.add(key);
            int node_idx = this->_nodes->add();
            Node& node = this->_nodes->at(node_idx);

            node.key_idx = string_idx;
            node.value = value;

            this->_insertNode(node_idx, parent, sign);
        }

        StringPool _pool;
    };

    template <typename Key, typename Value>
    class RedBlackObjMap : public RedBlackTree<Key, RedBlackMapNode<Key, Value>>
    {
        typedef RedBlackTree<Key, RedBlackMapNode<Key, Value>> Parent;

    public:
        int dummy; // for VS 2008 to generate correct x64 code

        RedBlackObjMap()
        {
            dummy = 123;
        }

        ~RedBlackObjMap() override
        {
            this->clear();
        }

        typedef RedBlackMapNode<Key, Value> Node;

        Value& at(Key key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return this->_nodes->at(idx).value;

            throw typename Parent::Error("at(): key not found");
        }

        Value* at2(Key key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return &this->_nodes->at(idx).value;

            return 0;
        }

        Value& insert(Key key)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key already present");

            return _insertObj(key, idx, sign);
        }

        template <typename A>
        Value& insert(Key key, A& a)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key already present");

            return _insertObj(key, idx, sign, a);
        }

        Value& findOrInsert(Key key)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
            {
                return this->_nodes->at(idx).value;
            }

            return _insertObj(key, idx, sign);
        }

        template <typename A>
        Value& findOrInsert(Key key, A& a)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
            {
                return this->_nodes->at(idx).value;
            }

            return _insertObj(key, idx, sign, a);
        }

        void remove(Key key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
            {
                this->value(idx).~Value();
                this->_removeNode(idx);
            }
            else
                throw typename Parent::Error("remove(): key not found");
        }

        Key& key(int node) const
        {
            return this->_nodes->at(node).key;
        }

        Value& value(int node) const
        {
            return this->_nodes->at(node).value;
        }

        void clear() override
        {
            int i;

            for (i = this->begin(); i != this->end(); i = this->next(i))
                this->value(i).~Value();

            Parent::clear();
        }

    protected:
        int _compare(Key key, const Node& node) const override
        {
            if (key < node.key)
                return -1;
            if (node.key < key)
                return 1;
            return 0;
        }

        Value* _insert(Key key, int parent, int sign)
        {
            int node_idx = this->_nodes->add();
            Node& node = this->_nodes->at(node_idx);

            node.key = key;

            this->_insertNode(node_idx, parent, sign);

            return &node.value;
        }

        Value& _insertObj(Key key, int parent, int sign)
        {
            Value* value = _insert(key, parent, sign);
            new (value) Value();
            return *value;
        }

        template <typename A>
        Value& _insertObj(Key key, int parent, int sign, A& a)
        {
            Value* value = _insert(key, parent, sign);
            new (value) Value(a);
            return *value;
        }

    private:
        RedBlackObjMap(const RedBlackObjMap&); // no implicit copy
    };

    template <typename Value>
    struct RedBlackStringObjMapNode : public RedBlackNodeBase
    {
        int key_idx;
        Value value;
    };

    template <typename Value>
    class RedBlackStringObjMap : public RedBlackTree<const char*, RedBlackStringObjMapNode<Value>>
    {
        typedef RedBlackStringObjMapNode<Value> Node;
        typedef RedBlackTree<const char*, Node> Parent;
        typedef RedBlackTree<const char*, Node> GrandParent;

    public:
        RedBlackStringObjMap()
        {
        }

        ~RedBlackStringObjMap() override
        {
            this->clear();
        }

        void clear() override
        {
            for (int i = this->begin(); i != this->end(); i = this->next(i))
                this->value(i).~Value();
            GrandParent::clear();
            _pool.clear();
        }

        Value& at(const char* key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return this->_nodes->at(idx).value;

            throw typename Parent::Error("at(): key %s not found", key);
        }

        Value& operator[](const char* key) const
        {
            return at(key);
        }

        Value* at2(const char* key) const
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return &this->_nodes->at(idx).value;

            return 0;
        }

        int insert(const char* key)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key %s already present", key);

            return _insertObj(key, idx, sign);
        }

        template <typename A>
        int insert(const char* key, A& a)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                throw typename Parent::Error("insert(): key %s already present", key);

            return _insertObj(key, idx, sign, a);
        }

        int findOrInsert(const char* key)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return idx;

            return _insertObj(key, idx, sign);
        }

        template <typename A>
        int findOrInsert(const char* key, A& a)
        {
            int sign;
            int idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
                return idx;

            return _insertObj(key, idx, sign, a);
        }

        void remove(const int idx)
        {
            _pool.remove(this->_nodes->at(idx).key_idx);
            this->value(idx).~Value();
            this->_removeNode(idx);
        }

        void remove(const char* key)
        {
            int sign, idx = this->_findClosest(key, sign);

            if (idx != -1 && sign == 0)
            {
                remove(idx);
            }
            else
                throw typename Parent::Error("remove(): key %s not found", key);
        }

        const char* key(int node) const
        {
            return _pool.at(this->_nodes->at(node).key_idx);
        }

        Value& value(int node) const
        {
            return this->_nodes->at(node).value;
        }

        void copy(const RedBlackStringObjMap<Value>& other)
        {
            clear();
            for (int i = other.begin(); i != other.end(); i = other.next(i))
            {
                const char* key = other.key(i);
                int id = insert(key);
                // Use `copy` method if `Value` type
                value(id).copy(other.value(i));
            }
        }

    protected:
        int _compare(const char* key, const Node& node) const override
        {
            return strcmp(key, _pool.at(node.key_idx));
        }

        int _insert(const char* key, int parent, int sign)
        {
            int string_idx = _pool.add(key);
            int node_idx = this->_nodes->add();
            Node& node = this->_nodes->at(node_idx);

            node.key_idx = string_idx;

            this->_insertNode(node_idx, parent, sign);

            return node_idx;
        }

        int _insertObj(const char* key, int parent, int sign)
        {
            int idx = _insert(key, parent, sign);
            Value* value = &this->value(idx);

            new (value) Value();
            return idx;
        }

        template <typename A>
        int _insertObj(const char* key, int parent, int sign, A& a)
        {
            int idx = _insert(key, parent, sign);
            Value* value = &this->value(idx);

            new (value) Value(a);
            return idx;
        }

        StringPool _pool;

    private:
        RedBlackStringObjMap(const RedBlackStringObjMap&); // no implicit copy
    };

} // namespace indigo

#endif
