#ifndef __bingo_ptr__
#define __bingo_ptr__

#include "base_cpp/obj_array.h"
#include "base_cpp/exception.h"
#include "base_cpp/tlscont.h"
#include "bingo_mmf.h"
#include "base_cpp/profiling.h"
#include <new>
#include <string>

using namespace indigo;

namespace bingo
{
   class BingoAllocator;

   template<typename T> class BingoPtr
   {
   public:
      BingoPtr ()
      {
         _offset = (size_t)-1;
      }

      explicit BingoPtr (size_t offset) : _offset(offset)
      {
      }

      T * ptr();

      const T * ptr() const;

      T & ref()
      {
         return *ptr();
      }

      const T & ref() const
      {
         return *ptr();
      }

      T * operator->()
      {
         return ptr();
      }

      const T * operator->() const
      {
         return ptr();
      }

      T & operator [] (int index)
      {
         return ptr()[index];
      }

      BingoPtr<T> operator+ (int off)
      {
         return BingoPtr<T>(_offset + off * sizeof(T));
      }

      qword serialize () const
      {
         return _offset;
      }

      void unserialize (qword data)
      {
         _offset = data;
      }

      bool isNull ()
      {
         return _offset == -1;
      }

      void allocate ( int count = 1 );

      operator size_t() { return _offset; }

   private:
      size_t _offset;
   };

   template<typename T> class BingoArray
   {
   public:
      BingoArray()
      {
         _size = 0;
         _block_size = 10000;
         _block_count = 0;
      }

      void resize (int new_size)
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
            }
            _block_count = new_blocks_count;
         }

         _size = new_size;
      }

      T & at (int index)
      {
         if (index < 0 || index >= _size)
            throw Exception("BingoArray: incorrect idx");

         return *(_blocks[index / _block_size].ptr() + index % _block_size);
      }

      T & operator [] (int index)
      {
         return at(index);
      }

      T & push ()
      {
         if (_size % _block_size == 0)
         {
            int blocks_count = (_size + _block_size - 1) / _block_size;

            _blocks[blocks_count].allocate(_block_size);
         }

         T * arr =  _blocks[_size / _block_size].ptr();
         int idx_in_block = _size % _block_size;

         _size++;

         new(arr + idx_in_block) T();

         return arr[idx_in_block];
      }

      template <typename A> T & push (A &a)
      {
         if (_size % _block_size == 0)
         {
            int blocks_count = (_size + _block_size - 1) / _block_size;

            _blocks[blocks_count].allocate(_block_size);
         }

         T * arr =  _blocks[_size / _block_size].ptr();
         int idx_in_block = _size % _block_size;

         _size++;

         new(arr + idx_in_block) T(a);

         return arr[idx_in_block];
      }

      void push (T elem)
      {
         T & new_elem = push();

         new_elem = elem;
      }

      int size () const
      {
         return _size;
      }

      int reserved () const
      {
         return _block_count * _block_size;
      }

   private:
      static const int _max_block_count = 10000;

      BingoPtr<T>  _blocks[_max_block_count];
      int _block_size;
      int _block_count;
      int _size;
   };

   class MMFStorage;

   class BingoAllocator
   {
   public:
      template<typename T> friend class BingoPtr;
      friend class MMFStorage;

      static void close ();

   private:
      static void _create (const char *filename, size_t size, ObjArray<MMFile> *mm_files);

      static void _load (const char *filename, ObjArray<MMFile> *mm_files);

      template<typename T> size_t allocate ( int count = 1 )
      {
         int alloc_size = sizeof(T) * count;

         if (alloc_size > _file_size)
            throw Exception("BingoAllocator: mmf size is too small to allocate memory");

         size_t file_idx = _free_off / _file_size;
         size_t file_off = _free_off % _file_size;

         if (alloc_size > _file_size - file_off)
            _addFile(alloc_size);

         size_t res_off = _free_off;
         _free_off += alloc_size;

         return res_off;
      }

      static BingoAllocator *_getInstance ();

      byte * _get (size_t offset);

      BingoAllocator ();

      void _addFile (int alloc_size);

      void _genFilename (int idx, std::string &name);

      ObjArray<MMFile> *_mm_files;
      size_t _file_size;
      size_t _free_off;

      static BingoAllocator * _instance;
   };

   // Implementations for BingoPtr and BingoAllocator are dependent and thus implementation is here
   template <typename T>
   T * BingoPtr<T>::ptr()
   {
      BingoAllocator *_allocator = BingoAllocator::_getInstance();

      return (T *)(_allocator->_get(_offset));
   }
   template <typename T>
   const T * BingoPtr<T>::ptr() const
   {
      BingoAllocator *_allocator = BingoAllocator::_getInstance();

      return (T *)(_allocator->_get(_offset));
   }

   template <typename T>
   void BingoPtr<T>::allocate ( int count )
   {
      BingoAllocator *_allocator = BingoAllocator::_getInstance();

      _offset = _allocator->allocate<T>(count);
   }
};

#endif //__bingo_ptr__
