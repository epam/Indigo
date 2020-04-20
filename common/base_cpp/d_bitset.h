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

#ifndef _d_bitset_
#define _d_bitset_

#include "base_cpp/array.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    DECL_EXCEPTION(DbitsetError);

    class DLLEXPORT Dbitset
    {
        // bitsets are packed into arrays of "words."  Currently a word is
        // a long long, which consists of 64 bits, requiring 6 address bits.
        // The choice of word size is determined purely by performance concerns.

    private:
        enum
        {
            ADDRESS_BITS_PER_WORD = 6,
            BITS_PER_WORD = 1 << ADDRESS_BITS_PER_WORD,
            MAX_SHIFT_NUMBER = 63
        };
        static const qword WORD_MASK = 0xFFFFFFFFFFFFFFFFULL;

        // the number of words in the logical size of this BitSet.
        int _wordsInUse;
        // bits number in bitset
        int _bitsNumber;
        // words size
        int _length;
        // given a bit index, return word index containing it.
        static int _wordIndex(int bitIndex)
        {
            return bitIndex >> ADDRESS_BITS_PER_WORD;
        }
        // set the field wordsInUse with the logical size in words of the bit
        // set.  WARNING:This method assumes that the number of words actually
        // in use is less than or equal to the current value of wordsInUse!
        void _recalculateWordsInUse();
        // creates a new bit set. All bits are initially false.
        void _initWords(int nbits);
        // ensures that the BitSet can accommodate a given wordIndex
        void _expandTo(int wordIndex);

        int _bitCount(qword b) const;

        int _leastSignificantBitPosition(qword n) const;

        Array<qword> _words;

        Dbitset(const Dbitset&);            // no implicit copy
        Dbitset& operator=(const Dbitset&); // no implicit assign
    public:
        Dbitset();
        // creates a bit set whose initial size
        explicit Dbitset(int nbits);
        ~Dbitset();

        // sets the bit at the specified index to the complement of its current value
        void flip(int bitIndex);
        // sets each bit from the specified fromIndex to the specified toIndex (exclusive)
        // to the complement of its current value
        void flip(int fromIndex, int toIndex);
        // sets all bits to the complement values
        void flip();

        // sets all bits to true
        void set();
        // sets the bit at the specified index to true
        void set(int bitIndex);
        // sets the bit at the specified index to the specified value.
        void set(int bitIndex, bool value);
        // sets the bits from the specified fromIndex to the specified toIndex(exclusive)
        void set(int fromIndex, int toIndex);
        // sets the bit specified by the index to false
        void reset(int bitIndex);
        // sets all of the bits in this BitSet to false
        void clear();
        // returns the value of the bit with the specified index
        bool get(int bitIndex) const;
        // returns the index of the first bit that is set to true
        // that occurs on or after the specified starting index. If no such
        // bit exists then -1 is returned
        int nextSetBit(int fromIndex) const;
        // returns true if BitSet contains no bits that are set true
        bool isEmpty() const
        {
            return _wordsInUse == 0;
        };
        // Returns true if the specified BitSet has any bits set to true
        // that are also set to true in this BitSet
        bool intersects(const Dbitset& set) const;
        bool complements(const Dbitset& set) const;
        // Performs a logical AND of this target BitSet with the argument BitSet
        void andWith(const Dbitset& set);
        // Performs a logical OR of this target BitSet with the argument BitSet
        void orWith(const Dbitset& set);
        // Performs a logical XOR of this target BitSet with the argument BitSet
        void xorWith(const Dbitset& set);
        // Clears all of the bits in this BitSet whose corresponding
        // bit is set in the specified BitSet.
        void andNotWith(const Dbitset& set);
        // Returns the number of bits of space actually in use by this
        // BitSet to represent bit values
        int size() const
        {
            return _bitsNumber;
        };
        // Compares this BitSet against the specified BitSet.
        bool equals(const Dbitset& set) const;
        // Cloning this BitSet produces a new BitSet
        void copy(const Dbitset& set);
        // copy part of BitSet
        void copySubset(const Dbitset& set);
        // resizes this BitSet
        void resize(int size);
        // checks if this BitSet is subset of argument BitSet
        bool isSubsetOf(const Dbitset& set) const;
        // checks if this BitSet is proper subset of argument BitSet
        bool isProperSubsetOf(const Dbitset& set) const;

        // fills with false all bits in this BitSet
        void zeroFill();

        void bsOrBs(const Dbitset& set1, const Dbitset& set2);
        // Clears all of the bits in first argument BitSet whose corresponding
        // bits is set in the specified second argument BitSet. Result -> this
        void bsAndNotBs(const Dbitset& set1, const Dbitset& set2);
        // Performs a logical AND of two argument BitSets. result saves to this Bitset
        void bsAndBs(const Dbitset& set1, const Dbitset& set2);
        int bitsNumber() const;

        qword shiftOne(int shiftNumber);

        class Iterator
        {
        public:
            Iterator(Dbitset&);

            ~Iterator()
            {
            }

            int begin();
            int next();
            inline int end()
            {
                return -1;
            }

        private:
            void _fillIndexes(byte buf, Array<int>& indexes);
            int _wordsInUse;
            qword* _words;

            int _fromWordIdx;
            int _fromByteIdx;
            int _fromBitIdx;
            qword* _fromWord;
            Array<int>* _fromIndexes;

            int _shiftByte;
            int _shiftWord;

        private:
            Iterator(const Iterator&); // no implicit copy
        };

        DECL_TPL_ERROR(DbitsetError);
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
