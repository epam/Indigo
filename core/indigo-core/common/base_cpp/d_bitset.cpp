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

#include "base_cpp/d_bitset.h"
#include "base_cpp/obj_array.h"
#include <algorithm>

using namespace indigo;

// private---------------------------------------------------------------------------------------

void Dbitset::_recalculateWordsInUse()
{
    // Traverse the bitset until a used word is found
    int i;
    for (i = _length - 1; i >= 0; --i)
        if (_words[i] != 0)
            break;
    _wordsInUse = i + 1; // The new logical size
}

void Dbitset::_initWords(int nbits)
{
    _wordsInUse = 0;
    _length = _wordIndex(nbits - 1) + 1;
    _words.clear_resize(_length);
    _words.zerofill();
    _bitsNumber = nbits;
}

void Dbitset::_expandTo(int wordIndex)
{
    int wordsRequired = wordIndex + 1;
    if (_wordsInUse < wordsRequired)
    {
        _wordsInUse = wordsRequired;
    }
}

int Dbitset::_bitCount(qword b) const
{
    b = (b & 0x5555555555555555LL) + ((b >> 1) & 0x5555555555555555LL);
    b = (b & 0x3333333333333333LL) + ((b >> 2) & 0x3333333333333333LL);
    b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FLL;
    b = b + (b >> 8);
    b = b + (b >> 16);
    b = (b + (b >> 32)) & 0x0000007F;
    return (int)b;
}

int Dbitset::_leastSignificantBitPosition(qword n) const
{
    if (n == 0)
        return -1;

    int pos = 63;
    if (n & 0x00000000FFFFFFFFLL)
    {
        pos -= 32;
    }
    else
    {
        n >>= 32;
    }
    if (n & 0x000000000000FFFFLL)
    {
        pos -= 16;
    }
    else
    {
        n >>= 16;
    }
    if (n & 0x00000000000000FFLL)
    {
        pos -= 8;
    }
    else
    {
        n >>= 8;
    }
    if (n & 0x000000000000000FLL)
    {
        pos -= 4;
    }
    else
    {
        n >>= 4;
    }
    if (n & 0x0000000000000003LL)
    {
        pos -= 2;
    }
    else
    {
        n >>= 2;
    }
    if (n & 0x0000000000000001LL)
    {
        pos -= 1;
    }
    return pos;
}

// public------------------------------------------------------------------------------------------------------

Dbitset::Dbitset()
{
    _initWords(BITS_PER_WORD);
}

Dbitset::Dbitset(int nbits)
{
    _initWords(nbits);
}

Dbitset::~Dbitset()
{
}

void Dbitset::copy(const Dbitset& set)
{
    if (_length != set._length)
    {
        _length = set._length;
        _words.resize(_length);
    }
    _bitsNumber = set._bitsNumber;
    _wordsInUse = set._wordsInUse;
    _words.copy(set._words);
}

void Dbitset::copySubset(const Dbitset& set)
{
    if (_bitsNumber == set._bitsNumber)
        copy(set);
    if (_bitsNumber < set._bitsNumber)
        return;
    _wordsInUse = std::max(_wordsInUse, set._wordsInUse);
    for (int i = 0; i < set._length; ++i)
    {
        _words[i] = set._words[i];
    }
}

void Dbitset::flip()
{
    flip(0, _bitsNumber);
}

void Dbitset::flip(int bitIndex)
{
    int wordindex = _wordIndex(bitIndex);
    bitIndex -= (wordindex << ADDRESS_BITS_PER_WORD);
    _expandTo(wordindex);

    _words[wordindex] ^= ((qword)1 << bitIndex);

    _recalculateWordsInUse();
}

void Dbitset::flip(int fromIndex, int toIndex)
{
    if (fromIndex == toIndex)
        return;

    int start_word_index = _wordIndex(fromIndex);
    int end_word_index = _wordIndex(toIndex - 1);
    _expandTo(end_word_index);

    fromIndex -= (start_word_index << ADDRESS_BITS_PER_WORD);
    toIndex -= (end_word_index << ADDRESS_BITS_PER_WORD);

    qword first_word_mask = WORD_MASK << fromIndex;
    qword last_word_mask = shiftOne(toIndex) - 1;

    if (start_word_index == end_word_index)
    {
        _words[start_word_index] ^= (first_word_mask & last_word_mask);
    }
    else
    {
        _words[start_word_index] ^= first_word_mask;

        for (int i = start_word_index + 1; i < end_word_index; ++i)
            _words[i] ^= WORD_MASK;

        _words[end_word_index] ^= last_word_mask;
    }
    _recalculateWordsInUse();
}

void Dbitset::set(int bitIndex)
{
    int wordindex = _wordIndex(bitIndex);
    bitIndex -= (wordindex << ADDRESS_BITS_PER_WORD);
    _expandTo(wordindex);
    _words[wordindex] |= ((qword)1 << bitIndex); // Restores invariants
}

void Dbitset::set(int fromIndex, int toIndex)
{
    if (fromIndex == toIndex)
        return;

    int start_word_index = _wordIndex(fromIndex);
    int end_word_index = _wordIndex(toIndex - 1);

    fromIndex -= (start_word_index << ADDRESS_BITS_PER_WORD);
    toIndex -= (end_word_index << ADDRESS_BITS_PER_WORD);

    qword first_word_mask = WORD_MASK << fromIndex;
    qword last_word_mask = shiftOne(toIndex) - 1;

    if (start_word_index == end_word_index)
    {
        _words[start_word_index] |= (first_word_mask & last_word_mask);
    }
    else
    {
        _words[start_word_index] |= first_word_mask;

        for (int i = start_word_index + 1; i < end_word_index; ++i)
            _words[i] |= WORD_MASK;

        _words[end_word_index] |= last_word_mask;
    }
    _recalculateWordsInUse();
}

void Dbitset::set()
{
    set(0, _bitsNumber);
}

void Dbitset::set(int bitIndex, bool value)
{
    if (value)
        set(bitIndex);
    else
        reset(bitIndex);
}

void Dbitset::reset(int bitIndex)
{
    int wordindex = _wordIndex(bitIndex);
    bitIndex -= (wordindex << ADDRESS_BITS_PER_WORD);
    if (wordindex >= _wordsInUse)
        return;

    _words[wordindex] &= ~((qword)1 << bitIndex);

    _recalculateWordsInUse();
}

void Dbitset::clear()
{
    while (_wordsInUse > 0)
        _words[--_wordsInUse] = 0;
}

bool Dbitset::get(int bitIndex) const
{
    int word_index = _wordIndex(bitIndex);
    bitIndex -= (word_index << ADDRESS_BITS_PER_WORD);
    return (word_index < _wordsInUse) && ((_words[word_index] & ((qword)1 << bitIndex)) != 0);
}

int Dbitset::nextSetBit(int fromIndex) const
{
    int u = _wordIndex(fromIndex);
    if (u >= _wordsInUse)
        return -1;
    fromIndex -= (u << ADDRESS_BITS_PER_WORD);

    qword word = _words[u] & (WORD_MASK << fromIndex);
    for (; word == 0; word = _words[u])
    {
        if (++u >= _wordsInUse)
            return -1;
    }
    return _leastSignificantBitPosition(word) + (u << ADDRESS_BITS_PER_WORD);
}

bool Dbitset::intersects(const Dbitset& set) const
{
    for (int i = std::min(_wordsInUse, set._wordsInUse) - 1; i >= 0; --i)
        if ((_words[i] & set._words[i]) != 0)
            return true;
    return false;
}

bool Dbitset::complements(const Dbitset& set) const
{
    if (_wordsInUse > set._wordsInUse)
        for (int i = _wordsInUse - 1; i >= set._wordsInUse; --i)
            if (_words[i])
                return true;
    for (int i = _wordsInUse - 1; i >= 0; --i)
        if ((_words[i] & ~set._words[i]) != 0)
            return true;
    return false;
}

void Dbitset::andWith(const Dbitset& set)
{
    while (_wordsInUse > set._wordsInUse)
        _words[--_wordsInUse] = 0;

    // Perform logical AND on words in common
    for (int i = 0; i < _wordsInUse; ++i)
        _words[i] &= set._words[i];

    _recalculateWordsInUse();
}

void Dbitset::orWith(const Dbitset& set)
{
    if (_wordsInUse < set._wordsInUse)
        _wordsInUse = set._wordsInUse;

    // Perform logical OR on words in common
    for (int i = 0; i < _wordsInUse; ++i)
    {
        _words[i] |= set._words[i];
    }
}

void Dbitset::xorWith(const Dbitset& set)
{
    if (_wordsInUse < set._wordsInUse)
    {
        _wordsInUse = set._wordsInUse;
    }

    // Perform logical XOR on words in common
    for (int i = 0; i < _wordsInUse; ++i)
        _words[i] ^= set._words[i];

    _recalculateWordsInUse();
}

void Dbitset::andNotWith(const Dbitset& set)
{
    for (int i = std::min(_wordsInUse, set._wordsInUse) - 1; i >= 0; --i)
        _words[i] &= ~set._words[i];

    _recalculateWordsInUse();
}

bool Dbitset::equals(const Dbitset& set) const
{
    if (_wordsInUse != set._wordsInUse)
        return false;

    // Check words in use by both BitSets
    for (int i = 0; i < _wordsInUse; ++i)
        if (_words[i] != set._words[i])
            return false;

    return true;
}

void Dbitset::resize(int size)
{
    int new_length = _wordIndex(size - 1) + 1;
    _words.resize(new_length);
    if (new_length > _length)
        for (int i = _length; i < new_length; ++i)
            _words[i] = 0;
    _bitsNumber = size;
    _length = new_length;
}

bool Dbitset::isSubsetOf(const Dbitset& set) const
{
    for (int i = 0; i < _wordsInUse; ++i)
        if (_words[i] & ~set._words[i])
            return false;
    return true;
}

bool Dbitset::isProperSubsetOf(const Dbitset& set) const
{
    bool proper = false;
    for (int i = 0; i < _wordsInUse; ++i)
    {
        if (set._words[i] & ~_words[i])
            proper = true;
        if (_words[i] & ~set._words[i])
            return false;
    }
    return proper;
}

void Dbitset::zeroFill()
{
    _words.zerofill();
    _recalculateWordsInUse();
}

void Dbitset::bsOrBs(const Dbitset& set1, const Dbitset& set2)
{
    int max_words = std::max(set1._wordsInUse, set2._wordsInUse);
    for (int i = 0; i < max_words; ++i)
    {
        _words[i] = set1._words[i] | set2._words[i];
    }
    for (int i = max_words; i < _wordsInUse; ++i)
    {
        _words[i] = 0;
    }
    _wordsInUse = max_words;
}

void Dbitset::bsAndNotBs(const Dbitset& set1, const Dbitset& set2)
{
    for (int i = 0; i < set1._wordsInUse; ++i)
    {
        _words[i] = set1._words[i] & ~set2._words[i];
    }
    for (int i = set1._wordsInUse; i < _wordsInUse; ++i)
    {
        _words[i] = 0;
    }
    _recalculateWordsInUse();
}

void Dbitset::bsAndBs(const Dbitset& set1, const Dbitset& set2)
{
    for (int i = 0; i < set1._wordsInUse; ++i)
    {
        _words[i] = set1._words[i] & set2._words[i];
    }
    for (int i = set1._wordsInUse; i < _wordsInUse; ++i)
    {
        _words[i] = 0;
    }
    _recalculateWordsInUse();
}

int Dbitset::bitsNumber() const
{
    int bits_num = 0;
    for (int i = 0; i < _wordsInUse; ++i)
        bits_num += _bitCount(_words[i]);
    return bits_num;
}

// some 64-bit compilators can not correctly work with big values shift. So it must be processed manually
qword Dbitset::shiftOne(int shiftNumber)
{
    qword result = (qword)1;
    while (shiftNumber > MAX_SHIFT_NUMBER)
    {
        result = result << MAX_SHIFT_NUMBER;
        shiftNumber -= MAX_SHIFT_NUMBER;
    }
    result = result << shiftNumber;
    return result;
}

Dbitset::Iterator::Iterator(Dbitset& self) : _fromWordIdx(0), _fromByteIdx(-1), _fromBitIdx(-1), _fromIndexes(0)
{
    _words = self._words.ptr();
    _wordsInUse = self._wordsInUse;
}

static ObjArray<Array<int>> all_indexes;

int Dbitset::Iterator::begin()
{
    if (all_indexes.size() == 0)
    {
        for (unsigned int buf = 0; buf < 256; ++buf)
        {
            Array<int>& indexes = all_indexes.push();
            _fillIndexes(buf, indexes);
        }
    }

    if (_wordsInUse == 0)
        return -1;

    _fromWordIdx = -1;
    _fromBitIdx = -1;
    _fromByteIdx = -1;
    _fromIndexes = 0;
    _fromWord = 0;

    return next();
}

int Dbitset::Iterator::next()
{
    if (_fromIndexes)
    {
        ++_fromBitIdx;
        if (_fromBitIdx < _fromIndexes->size())
            return _fromIndexes->at(_fromBitIdx) + _shiftByte + _shiftWord;
    }
    _fromIndexes = 0;
    if (_fromWord)
    {
        for (++_fromByteIdx; _fromByteIdx < 8; ++_fromByteIdx)
        {
            int from_byte = ((byte*)_fromWord)[_fromByteIdx];
            if (from_byte == 0)
                continue;

            _fromIndexes = &(all_indexes.at(from_byte));

            _fromBitIdx = 0;
            _shiftByte = _fromByteIdx << 3;
            return _fromIndexes->at(_fromBitIdx) + _shiftByte + _shiftWord;
        }
    }
    _fromWord = 0;
    for (++_fromWordIdx; _fromWordIdx < _wordsInUse; ++_fromWordIdx)
    {
        _fromWord = &_words[_fromWordIdx];
        if (*_fromWord == 0)
            continue;

        for (_fromByteIdx = 0; _fromByteIdx < 8; ++_fromByteIdx)
        {
            int from_byte = ((byte*)_fromWord)[_fromByteIdx];
            if (from_byte == 0)
                continue;

            _fromIndexes = &(all_indexes.at(from_byte));

            _fromBitIdx = 0;
            _shiftByte = _fromByteIdx << 3;
            _shiftWord = _fromWordIdx << 6;
            return _fromIndexes->at(_fromBitIdx) + _shiftByte + _shiftWord;
        }
    }

    return -1;
}

void Dbitset::Iterator::_fillIndexes(byte buf, Array<int>& indexes)
{
    byte test_buf;
    for (int buf_idx = 0; buf_idx < 8; ++buf_idx)
    {
        test_buf = (byte)1 << buf_idx;
        if (buf & test_buf)
        {
            indexes.push(buf_idx);
        }
    }
}
