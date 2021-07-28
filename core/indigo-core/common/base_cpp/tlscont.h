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

#ifndef __tlscont_h__
#define __tlscont_h__

#include <memory>
#include <typeinfo>
#include <unordered_map>

#include "base_c/defs.h"
#include "base_c/os_tls.h"
#include "base_cpp/array.h"
#include <memory>
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/pool.h"
#include "base_cpp/ptr_array.h"
#include "base_cpp/red_black.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    // Session identifiers manager.
    // Every thread have local session ID that corresponds to the all
    // local session variables.
    class DLLEXPORT _SIDManager
    {
    public:
        static _SIDManager& getInst(void);
        static std::mutex& getLock();

        _SIDManager(void);
        ~_SIDManager(void);

        void setSessionId(qword id);
        qword allocSessionId(void);
        qword getSessionId(void);
        // Add specified SID to the vacant list.
        // This method should be called before thread exit if SID was
        // assigned automatically (not by manual TL_SET_SESSION_ID call)
        void releaseSessionId(qword id);

        DECL_ERROR;

    private:
        qword* _getID() const;

        // Thread local key for storing current session ID
        TLS_IDX_TYPE _tlsIdx;
        RedBlackSet<qword> _allSIDs;
        qword _lastNewSID;
        // Array with vacant SIDs
        Array<qword> _vacantSIDs;

    };

// Macros for managing session IDs for current thread
#define TL_GET_SESSION_ID() _SIDManager::getInst().getSessionId()
#define TL_SET_SESSION_ID(id) _SIDManager::getInst().setSessionId(id)
#define TL_ALLOC_SESSION_ID() _SIDManager::getInst().allocSessionId()
#define TL_RELEASE_SESSION_ID(id) _SIDManager::getInst().releaseSessionId(id)

    // Container that keeps one instance of specified type per session
    template <typename T> class _SessionLocalContainer
    {
    public:
        T& getLocalCopy()
        {
            return getLocalCopy(TL_GET_SESSION_ID());
        }

        T& getLocalCopy(const qword id)
        {
            OsLocker locker(_lock.ref());
            if (!_map.count(id))
            {
                _map[id] = std::unique_ptr<T>(new T());
            }
            return *_map.at(id);
        }

        void removeLocalCopy()
        {
            removeLocalCopy(TL_GET_SESSION_ID());
        }

        void removeLocalCopy(const qword id)
        {
            OsLocker locker(_lock.ref());
            if (_map.count(id))
            {
                _map.erase(id);
            }
        }

    private:
        using _Map = std::unordered_map<qword, std::unique_ptr<T>>;

        _Map _map;
        ThreadSafeStaticObj<std::mutex> _lock;
    };

    // Helpful templates to deal with commas in template type names
    // to be able to write like
    // QS_DEF((std::unordered_map<std::string, int>), atoms_id);
    // See http://stackoverflow.com/a/13842784
    template <typename T> struct ArgumentType;
    template <typename T, typename U> struct ArgumentType<T(U)>
    {
        typedef U Type;
    };
#define _GET_TYPE(t) ArgumentType<void(t)>::Type

// Macros for working with global variables per each session
// By tradition this macros start with TL_, but should start with SL_
#define TL_DECL_EXT(type, name) extern _SessionLocalContainer<_GET_TYPE(type)> TLSCONT_##name
#define TL_DECL(type, name) static _SessionLocalContainer<_GET_TYPE(type)> TLSCONT_##name
#define TL_GET(type, name) _GET_TYPE(type)& name = (TLSCONT_##name).getLocalCopy()
#define TL_DECL_GET(type, name)                                                                                                                                \
    TL_DECL(type, name);                                                                                                                                       \
    TL_GET(type, name)
#define TL_GET2(type, name, realname) _GET_TYPE(type)& name = (TLSCONT_##realname).getLocalCopy()
#define TL_GET_BY_ID(type, name, id) _GET_TYPE(type)& name = (TLSCONT_##name).getLocalCopy(id)
#define TL_DEF(className, type, name) _SessionLocalContainer<_GET_TYPE(type)> className::TLSCONT_##name
#define TL_DEF_EXT(type, name) _SessionLocalContainer<_GET_TYPE(type)> TLSCONT_##name

    // Pool for local variables, reused in consecutive function calls,
    // but not required to preserve their state
    template <typename T> class _ReusableVariablesPool
    {
    public:
        _ReusableVariablesPool()
        {
            is_valid = true;
        }
        ~_ReusableVariablesPool()
        {
            is_valid = false;
        }
        bool isValid() const
        {
            return is_valid;
        }

        T& getVacant(int& idx)
        {
            OsLocker locker(_lock);
            if (_vacant_indices.size() != 0)
            {
                idx = _vacant_indices.pop();
                return *_objects[idx];
            }
            _objects.add(new T);
            idx = _objects.size() - 1;
            _vacant_indices.reserve(idx + 1);
            return *_objects[idx];
        }

        void release(int idx)
        {
            OsLocker locker(_lock);
            _vacant_indices.push(idx);
        }

        T& getByIndex(int idx)
        {
            return *_objects[idx];
        }

    private:
        std::mutex _lock;
        bool is_valid;

        PtrArray<T> _objects;
        Array<int> _vacant_indices;
    };

    // Utility class for automatically release call
    template <typename T> class _ReusableVariablesAutoRelease
    {
    public:
        _ReusableVariablesAutoRelease() : _idx(-1), _var_pool(0)
        {
        }

        void init(int idx, _ReusableVariablesPool<T>* var_pool)
        {
            _idx = idx;
            _var_pool = var_pool;
        }

        ~_ReusableVariablesAutoRelease(void)
        {
            if (_var_pool == 0)
                return;
            // Check if the _var_pool destructor have not been called already
            // (this can happen on program exit)
            if (_var_pool->isValid())
                _var_pool->release(_idx);
        }

    protected:
        int _idx;
        _ReusableVariablesPool<T>* _var_pool;
    };

    // Abstract proxy class to call a destructor for an allocated data
    class Destructor
    {
    public:
        virtual void callDestructor(void* data) = 0;

        virtual ~Destructor(){};
    };

    // Proxy destructor class for a type T
    template <typename T> class DestructorT : public Destructor
    {
    public:
        void callDestructor(void* data) override
        {
            ((T*)data)->~T();
        }
    };

    // Template function to create proxy destructor
    template <typename T> Destructor* createDestructor(T* t)
    {
        return new DestructorT<T>();
    }

    // Variables pool that can reuse objects allocations that are initialized in the same order
    class _LocalVariablesPool
    {
    public:
        _LocalVariablesPool()
        {
            reset();
        }

        ~_LocalVariablesPool()
        {
            for (int i = 0; i < data.size(); i++)
            {
                destructors[i]->callDestructor(data[i]);
                free(data[i]);
            }
        }

        template <typename T> size_t hash()
        {
            return typeid(T).hash_code();
        }

        template <typename T> T& getVacant()
        {
            data.expandFill(index + 1, 0);
            destructors.expand(index + 1);
            type_hash.expandFill(index + 1, 0);

            if (data[index] == 0)
            {
                // Allocate data and destructor
                data[index] = malloc(sizeof(T));
                T* t = new (data[index]) T();
                destructors[index] = createDestructor(t);

                type_hash[index] = hash<T>();
            }

            // Class hash check to verify initialization order
            if (type_hash[index] != hash<T>())
                throw Exception("VariablesPool: invalid initialization order");

            T* t = (T*)data[index];
            index++;
            return *t;
        }

        void reset()
        {
            index = 0;
        }

    private:
        Array<void*> data;
        Array<size_t> type_hash;
        PtrArray<Destructor> destructors;
        int index;
    };

    // Auto release class that additionally calls reset method for LocalVariablesPool
    class _LocalVariablesPoolAutoRelease : public _ReusableVariablesAutoRelease<_LocalVariablesPool>
    {
    public:
        ~_LocalVariablesPoolAutoRelease()
        {
            if (_var_pool == 0)
                return;
            if (_var_pool->isValid())
            {
                _LocalVariablesPool& local_pool = _var_pool->getByIndex(_idx);
                local_pool.reset();
            }
        }
    };

} // namespace indigo

// "Quasi-static" variable definition. Calls clear() at the end
#define QS_DEF(TYPE, name)                                                                                                                                     \
    static ThreadSafeStaticObj<_ReusableVariablesPool<_GET_TYPE(TYPE)>> _POOL_##name;                                                                          \
    int _POOL_##name##_idx;                                                                                                                                    \
    _GET_TYPE(TYPE)& name = _POOL_##name->getVacant(_POOL_##name##_idx);                                                                                       \
    _ReusableVariablesAutoRelease<_GET_TYPE(TYPE)> _POOL_##name##_auto_release;                                                                                \
    _POOL_##name##_auto_release.init(_POOL_##name##_idx, _POOL_##name.ptr());                                                                                  \
    name.clear();
// Use this for debug purposes if you suspect QS_DEF in something bad
// #define QS_DEF(TYPE, name) TYPE name;

// "Quasi-static" variable definition. Calls clear_resize() at the end
#define QS_DEF_RES(TYPE, name, len)                                                                                                                            \
    static ThreadSafeStaticObj<_ReusableVariablesPool<_GET_TYPE(TYPE)>> _POOL_##name;                                                                          \
    int _POOL_##name##_idx;                                                                                                                                    \
    _GET_TYPE(TYPE)& name = _POOL_##name->getVacant(_POOL_##name##_idx);                                                                                       \
    _ReusableVariablesAutoRelease<_GET_TYPE(TYPE)> _POOL_##name##_auto_release;                                                                                \
    _POOL_##name##_auto_release.init(_POOL_##name##_idx, _POOL_##name.ptr());                                                                                  \
    name.clear_resize(len);
// Use this for debug purposes if you suspect QS_DEF in something bad
// #define QS_DEF_RES(TYPE, name, len) TYPE name; name.clear_resize(len);

// Reusable class members definition
// By tradition this macros start with TL_, but should start with SL_
// To work with them you should first define commom pool with CP_DECL,
// then define it in the source class with CP_DEF(cls), and initialize
// in the constructor via CP_INIT before any TL_CP_ initializations
//

// Add this to class definition
#define TL_CP_DECL(TYPE, name)                                                                                                                                 \
    typedef _GET_TYPE(TYPE) _##name##_TYPE;                                                                                                                    \
    _GET_TYPE(TYPE) & name

// Add this to constructor initialization list
#define TL_CP_GET(name) name(_local_pool.getVacant<_##name##_TYPE>())

#define CP_DECL                                                                                                                                                \
    _LocalVariablesPoolAutoRelease _local_pool_autorelease;                                                                                                    \
    static _LocalVariablesPool& _getLocalPool(_LocalVariablesPoolAutoRelease& auto_release);                                                                   \
    _LocalVariablesPool& _local_pool

#define CP_INIT _local_pool(_getLocalPool(_local_pool_autorelease))

#define CP_DEF(cls)                                                                                                                                            \
    _LocalVariablesPool& cls::_getLocalPool(_LocalVariablesPoolAutoRelease& auto_release)                                                                      \
    {                                                                                                                                                          \
        static ThreadSafeStaticObj<_ReusableVariablesPool<_LocalVariablesPool>> _shared_pool;                                                                  \
                                                                                                                                                               \
        int idx;                                                                                                                                               \
        _LocalVariablesPool& var = _shared_pool->getVacant(idx);                                                                                               \
        auto_release.init(idx, _shared_pool.ptr());                                                                                                            \
        return var;                                                                                                                                            \
    }

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif // __tlscont_h__
