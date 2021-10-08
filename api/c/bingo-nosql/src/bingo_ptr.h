#ifndef __bingo_ptr__
#define __bingo_ptr__

#include <new>
#include <string>
#include <thread>

#include "base_cpp/exception.h"
#include "base_cpp/obj_array.h"
#include "base_cpp/os_sync_wrapper.h"
#include "base_cpp/profiling.h"
#include "base_cpp/tlscont.h"
#include "bingo_mmf.h"

using namespace indigo;

namespace bingo
{
    class BingoAllocator;

    struct BingoAddr
    {
        BingoAddr()
        {
            file_id = (size_t)-1;
            offset = (size_t)-1;
        }

        BingoAddr(size_t f_id, size_t off) : file_id(f_id), offset(off)
        {
        }

        size_t file_id;
        size_t offset;

        bool operator==(const BingoAddr& other)
        {
            return (file_id == other.file_id) && (offset == other.offset);
        }

        static const BingoAddr bingo_null;
    };

    template <typename T> class BingoPtr
    {
    public:
        BingoPtr()
        {
        }

        explicit BingoPtr(BingoAddr addr) : _addr(addr)
        {
        }

        explicit BingoPtr(size_t file_id, size_t offset) : _addr(file_id, offset)
        {
        }

        T* ptr();

        const T* ptr() const;

        T& ref()
        {
            return *ptr();
        }

        const T& ref() const
        {
            return *ptr();
        }

        T* operator->()
        {
            return ptr();
        }

        const T* operator->() const
        {
            return ptr();
        }

        BingoPtr<T> operator+(int off)
        {
            return BingoPtr<T>(_addr.file_id, _addr.offset + off * sizeof(T));
        }

        T& operator[](int idx)
        {
            return *(ptr() + idx);
        }

        /*
        qword serialize () const
        {
           return _offset;
        }

        void unserialize (qword data)
        {
           _offset = data;
        }
        */

        bool isNull()
        {
            return (_addr.offset == (size_t)-1) && (_addr.file_id == (size_t)-1);
        }

        void allocate(int count = 1);

        operator BingoAddr() const
        {
            return _addr;
        }

    private:
        BingoAddr _addr;
    };

    template <typename T> class BingoArray
    {
    public:
        BingoArray(int block_size = 10000) : _block_size(block_size)
        {
            _size = 0;
            _block_count = 0;
        }

        void resize(int new_size)
        {
            if (new_size > reserved())
            {
                int blocks_count = (_size + _block_size - 1) / _block_size;
                int new_blocks_count = (new_size + _block_size - 1) / _block_size;

                if (new_blocks_count > _max_block_count)
                    throw Exception("BingoArray: block count limit is exceeded");

                for (int i = blocks_count; i < new_blocks_count; i++)
                {
                    _blocks[i].allocate(_block_size);
                    for (int j = 0; j < _block_size; j++)
                        new ((_blocks[i] + j).ptr()) T();
                }
                _block_count = new_blocks_count;
            }

            _size = new_size;
        }

        T& at(int index)
        {
            if (index < 0 || index >= _size)
                throw Exception("BingoArray: incorrect idx %d (size=%d)", index, _size);

            return *(_blocks[index / _block_size].ptr() + index % _block_size);
        }

        const T& at(int index) const
        {
            if (index < 0 || index >= _size)
                throw Exception("BingoArray: incorrect idx %d (size=%d)", index, _size);

            return *(_blocks[index / _block_size].ptr() + index % _block_size);
        }

        T& operator[](int index)
        {
            return at(index);
        }

        const T& operator[](int index) const
        {
            return at(index);
        }

        T& top()
        {
            int index = _size - 1;

            return *(_blocks[index / _block_size].ptr() + index % _block_size);
        }

        size_t find(const T& elem)
        {
            for (size_t i = 0; i < size(); i++)
            {
                if (at(i) == elem)
                    return i;
            }

            return (size_t)-1;
        }

        T& push()
        {
            if (_size % _block_size == 0)
            {
                int blocks_count = (_size + _block_size - 1) / _block_size;

                _blocks[blocks_count].allocate(_block_size);
            }

            T* arr = _blocks[_size / _block_size].ptr();
            int idx_in_block = _size % _block_size;

            _size++;

            new (arr + idx_in_block) T();

            return arr[idx_in_block];
        }

        template <typename A> T& push(A& a)
        {
            if (_size % _block_size == 0)
            {
                int blocks_count = (_size + _block_size - 1) / _block_size;

                _blocks[blocks_count].allocate(_block_size);
            }

            T* arr = _blocks[_size / _block_size].ptr();
            int idx_in_block = _size % _block_size;

            _size++;

            new (arr + idx_in_block) T(a);

            return arr[idx_in_block];
        }

        void push(T elem)
        {
            T& new_elem = push();

            new_elem = elem;
        }

        int size() const
        {
            return _size;
        }

        int reserved() const
        {
            return _block_count * _block_size;
        }

    private:
        static const int _max_block_count = 40000;

        int _block_size;
        int _block_count;
        int _size;
        BingoPtr<T> _blocks[_max_block_count];
    };

    template <typename T> class BingoList
    {
    private:
        struct _Link
        {
            BingoPtr<T> data_ptr;
            BingoPtr<_Link> next_link;
            BingoPtr<_Link> prev_link;

            _Link()
            {
                data_ptr = BingoPtr<T>(BingoAddr::bingo_null);
                next_link = BingoPtr<_Link>(BingoAddr::bingo_null);
                prev_link = BingoPtr<_Link>(BingoAddr::bingo_null);
            }
        };

    public:
        struct Iterator
        {
            BingoPtr<_Link> cur_link;

            Iterator() : cur_link(BingoPtr<T>(-1, -1))
            {
            }
            Iterator(BingoPtr<_Link> link) : cur_link(link)
            {
            }
            Iterator(const Iterator& it) : cur_link(it.cur_link)
            {
            }

            T& operator*()
            {
                return cur_link->data_ptr.ref();
            }

            T* operator->()
            {
                return cur_link->data_ptr.ptr();
            }

            Iterator& operator=(const Iterator& it)
            {
                cur_link = it.cur_link;

                return *this;
            }

            bool operator==(const Iterator& it) const
            {
                if ((BingoAddr)cur_link == (BingoAddr)it.cur_link)
                    return true;
                return false;
            }

            bool operator!=(const Iterator& it) const
            {
                return !(*this == it);
            }

            Iterator& operator++(int)
            {
                if ((BingoAddr)(cur_link->next_link) == BingoAddr::bingo_null)
                    throw Exception("BingoList::Iterator:operator++ There's no next link");

                cur_link = cur_link->next_link;

                return *this;
            }

            Iterator& operator--(int)
            {
                if ((BingoAddr)(cur_link->prev_link) == BingoAddr::bingo_null)
                    throw Exception("BingoList::Iterator:operator-- There's no previous link");

                cur_link = cur_link->prev_link;

                return *this;
            }
        };

        BingoList()
        {
            _size = 0;
            _begin_link.allocate();
            new (_begin_link.ptr()) _Link();
            _end_link = _begin_link;
        }

        bool empty() const
        {
            if (_end_link == _begin_link)
                return true;
            return false;
        }

        unsigned int size() const
        {
            return _size;
        }

        void insertBefore(Iterator pos, const BingoPtr<T> x)
        {
            BingoPtr<_Link> new_link;
            new_link.allocate();
            new (new_link.ptr()) _Link();

            new_link->data_ptr = x;

            new_link->next_link = pos.cur_link;
            new_link->prev_link = pos.cur_link->prev_link;

            if (!((BingoAddr)(pos.cur_link->prev_link) == BingoAddr::bingo_null))
                pos.cur_link->prev_link->next_link = new_link;
            pos.cur_link->prev_link = new_link;

            if ((BingoAddr)(pos.cur_link) == (BingoAddr)(_begin_link))
                _begin_link = new_link;

            _size++;
        }

        void insertBefore(Iterator pos, const T& x)
        {
            BingoPtr<T> data;
            data.allocate();
            data.ref() = x;

            insertBefore(pos, data);
        }

        void erase(Iterator& pos)
        {
            if (pos.cur_link->prev_link != -1)
                pos.cur_link->prev_link->next_link = pos.cur_link->next_link;
            if (pos.cur_link->next_link != -1)
                pos.cur_link->next_link->prev_link = pos.cur_link->prev_link;

            if (pos.cur_link == _begin_link)
                _begin_link = pos.cur_link->next_link;

            if (pos.cur_link == _end_link)
                throw Exception("BingoList:erase End link can't be removed");

            _size--;
        }

        void pushBack(const T& x)
        {
            insertBefore(end(), x);
        }

        void pushBack(const BingoPtr<T> x)
        {
            insertBefore(end(), x);
        }

        Iterator begin() const
        {
            return Iterator(_begin_link);
        }

        Iterator top() const
        {
            Iterator end_it(_end_link);
            end_it--;
            return end_it;
        }

        Iterator end() const
        {
            return Iterator(_end_link);
        }

    private:
        BingoPtr<_Link> _begin_link;
        BingoPtr<_Link> _end_link;
        int _size;
    };

    class MMFStorage;

    class BingoAllocator
    {
    public:
        template <typename T> friend class BingoPtr;
        friend class MMFStorage;

        static int getAllocatorDataSize();

    private:
        struct _BingoAllocatorData
        {
            _BingoAllocatorData() : _min_file_size(0), _max_file_size(0), _cur_file_id(0), _existing_files(0), _free_off(0)
            {
            }

            size_t _min_file_size;
            size_t _max_file_size;
            size_t _cur_file_id;
            dword _existing_files;
            size_t _free_off;
        };

        ObjArray<MMFile>* _mm_files;

        size_t _data_offset;

        static PtrArray<BingoAllocator> _instances;
        static std::mutex _instances_lock;

        std::string _filename;
        int _index_id;

        static void _create(const char* filename, size_t min_size, size_t max_size, size_t alloc_off, ObjArray<MMFile>* mm_files, int index_id);

        static void _load(const char* filename, size_t alloc_off, ObjArray<MMFile>* mm_files, int index_id, bool read_only);

        template <typename T> BingoAddr allocate(int count = 1)
        {
            byte* mmf_ptr = (byte*)_mm_files->at(0).ptr();

            _BingoAllocatorData* allocator_data = (_BingoAllocatorData*)(mmf_ptr + _data_offset);

            size_t alloc_size = sizeof(T) * count;

            size_t file_idx = allocator_data->_cur_file_id;
            size_t file_off = allocator_data->_free_off;
            size_t file_size = _mm_files->at((int)file_idx).size();

            if (alloc_size > file_size - file_off)
                _addFile(alloc_size);

            file_idx = allocator_data->_cur_file_id;
            file_size = _mm_files->at((int)file_idx).size();

            size_t res_off = allocator_data->_free_off;
            size_t res_id = allocator_data->_cur_file_id;
            allocator_data->_free_off += alloc_size;

            if (allocator_data->_free_off == file_size)
                _addFile(0);

            return BingoAddr(res_id, res_off);
        }

        static BingoAllocator* _getInstance();

        byte* _get(size_t file_id, size_t offset);

        BingoAllocator();

        void _addFile(size_t alloc_size);

        static size_t _getFileSize(size_t idx, size_t min_size, size_t max_size, dword sizes);

        static void _genFilename(int idx, const char* filename, std::string& out_name);
    };

    // Implementations for BingoPtr and BingoAllocator are dependent and thus implementation is here
    template <typename T> T* BingoPtr<T>::ptr()
    {
        BingoAllocator* _allocator = BingoAllocator::_getInstance();

        return (T*)(_allocator->_get(_addr.file_id, _addr.offset));
    }

    template <typename T> const T* BingoPtr<T>::ptr() const
    {
        BingoAllocator* _allocator = BingoAllocator::_getInstance();

        return (T*)(_allocator->_get(_addr.file_id, _addr.offset));
    }

    template <typename T> void BingoPtr<T>::allocate(int count)
    {
        BingoAllocator* _allocator = BingoAllocator::_getInstance();

        _addr = _allocator->allocate<T>(count);
    }
}; // namespace bingo

#endif //__bingo_ptr__
