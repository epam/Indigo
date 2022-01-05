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

#ifndef __multimap_h__
#define __multimap_h__

#include "base_cpp/red_black.h"

namespace indigo
{

    DECL_EXCEPTION(MultiMapError);

    template <typename K, typename V>
    class MultiMap : public NonCopyable
    {
    public:
        DECL_TPL_ERROR(MultiMapError);

        explicit MultiMap()
        {
        }
        ~MultiMap()
        {
        }

        const RedBlackSet<K>& keys() const;

        const RedBlackSet<V>& get(const K& k) const;
        const RedBlackSet<V>& operator[](const K& k) const;

        int size() const;

        bool find(const K& k, const V& v) const;

        void insert(const K& k, const V& v);
        void insert(const K& k, const Array<V>& vs);
        void insert(const K& k, const RedBlackSet<V>& vs);

        bool remove(const K& k);
        bool remove(const K& k, const V& v);

        void invert(MultiMap<V, K>& target) const;
        void copy(MultiMap<K, V>& target) const;

        void clear();

    protected:
        RedBlackSet<V>& _provide_set(const K& k);

        template <typename L, typename R>
        void _copy(MultiMap<L, R>& target, bool invert) const
        {
            for (auto i = _map.begin(); i != _map.end(); i = _map.next(i))
            {
                const RedBlackSet<V>& set = *_sets[_map.value(i)];
                for (auto j = set.begin(); j != set.end(); j = set.next(j))
                {
                    K& k = _map.key(i);
                    V& v = set.key(j);
                    if (invert)
                    {
                        target.insert(v, k);
                    }
                    else
                    {
                        target.insert(k, v);
                    }
                }
            }
        }

        RedBlackMap<K, int> _map;
        RedBlackSet<K> _keys;
        PtrArray<RedBlackSet<V>> _sets;
        const RedBlackSet<V> _nil;
    };

} // namespace indigo

#ifdef _DEBUG
#include "assert.h"
#define CHECK_KEYS assert(_map.size() == _keys.size());
#else
#define CHECK_KEYS
#endif

using namespace indigo;

template <typename K, typename V>
bool MultiMap<K, V>::find(const K& k, const V& v) const
{
    return get(k).find(v);
}

template <typename K, typename V>
const RedBlackSet<V>& MultiMap<K, V>::get(const K& k) const
{
    int* i = _map.at2(k);
    if (i)
    {
        return *_sets[*i];
    }
    return _nil;
}

template <typename K, typename V>
void MultiMap<K, V>::insert(const K& k, const V& v)
{
    _provide_set(k).insert(v);
}

template <typename K, typename V>
void MultiMap<K, V>::insert(const K& k, const Array<V>& vs)
{
    RedBlackSet<V>& set = _provide_set(k);
    for (auto i = 0; i < vs.size(); i++)
    {
        set.insert(vs[i]);
    }
}

template <typename K, typename V>
void MultiMap<K, V>::insert(const K& k, const RedBlackSet<V>& vs)
{
    RedBlackSet<V>& set = _provide_set(k);
    for (auto i = vs.begin(); i != vs.end(); i = vs.next(i))
    {
        set.insert(vs.key(i));
    }
}

template <typename K, typename V>
bool MultiMap<K, V>::remove(const K& k)
{
    int* i = _map.at2(k);
    if (!i)
    {
        return false;
    }
    _sets.remove(*i);
    _keys.remove(k);
    _map.remove(k);
    CHECK_KEYS;
    return true;
}

template <typename K, typename V>
bool MultiMap<K, V>::remove(const K& k, const V& v)
{
    int* i = _map.at2(k);
    if (!i)
    {
        return false;
    }
    RedBlackSet<V>& set = *_sets[*i];
    set.remove(v);
    if (set.size() < 1)
    {
        _sets.remove(*i);
        _keys.remove(k);
        _map.remove(k);
    }
    CHECK_KEYS;
    return true;
}

template <typename K, typename V>
void MultiMap<K, V>::copy(MultiMap<K, V>& target) const
{
    _copy(target, false);
}

template <typename K, typename V>
void MultiMap<K, V>::invert(MultiMap<V, K>& target) const
{
    _copy(target, true);
}

template <typename K, typename V>
void MultiMap<K, V>::clear()
{
    _map.clear();
    _keys.clear();
    _sets.clear();
}

template <typename K, typename V>
const RedBlackSet<K>& MultiMap<K, V>::keys() const
{
    return _keys;
}

template <typename K, typename V>
int MultiMap<K, V>::size() const
{
    return _map.size();
}

template <typename K, typename V>
const RedBlackSet<V>& MultiMap<K, V>::operator[](const K& k) const
{
    return get(k);
}

template <typename K, typename V>
RedBlackSet<V>& MultiMap<K, V>::_provide_set(const K& k)
{
    int* i = _map.at2(k);
    if (i)
    {
        return *_sets[*i];
    }

    _keys.insert(k);
    _map.insert(k, _sets.size());
    CHECK_KEYS;
    return _sets.add(new RedBlackSet<V>());
}

#endif