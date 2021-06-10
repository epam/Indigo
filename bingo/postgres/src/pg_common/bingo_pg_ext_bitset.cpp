#include "bingo_pg_fix_pre.h"

#include "bingo_pg_fix_post.h"

#include "base_c/bitarray.h"
#include "base_cpp/obj_array.h"
#include "bingo_pg_ext_bitset.h"
#include <algorithm>

BingoPgExternalBitset::BingoPgExternalBitset()
{
    _initWords(BITS_PER_WORD);
}

BingoPgExternalBitset::BingoPgExternalBitset(int nbits)
{
    _initWords(nbits);
}

void* BingoPgExternalBitset::serialize(int& size)
{
    size = (_length + 2) * sizeof(qword);
    _serializeWords.resize(_length + 2);
    _serializeWords[0] = _bitsNumber;
    _serializeWords[1] = (*_lastWordPtr);
    memcpy(&_serializeWords[2], _words, _length * sizeof(qword));
    return _serializeWords.ptr();
}

void BingoPgExternalBitset::deserialize(void* data_ptr, int data_len, bool ext)
{
    qword* data = (qword*)data_ptr;
    _bitsNumber = data[0];
    _lastWordPtr = &(data[1]);
    _length = _wordIndex(_bitsNumber - 1) + 1;
    qword* words_ptr = &(data[2]);
    if (ext)
    {
        _words = words_ptr;
    }
    else
    {
        _internalWords.resize(_length);
        _internalWords.copy(words_ptr, _length);
        _words = _internalWords.ptr();
        _lastWord = data[1];
        _lastWordPtr = &_lastWord;
    }
}

void BingoPgExternalBitset::_recalculateWordsInUse()
{
    // Traverse the bitset until a used word is found
    int i = _length - 1;
    for (; i >= 0; --i)
        if (_words[i] != 0)
            break;
    (*_lastWordPtr) = i + 1; // The new logical size
}

void BingoPgExternalBitset::_initWords(int nbits)
{
    _lastWordPtr = &_lastWord;
    (*_lastWordPtr) = 0;
    _length = _wordIndex(nbits - 1) + 1;
    _internalWords.clear_resize(_length);
    _internalWords.zerofill();
    _bitsNumber = nbits;
    _words = _internalWords.ptr();
}

void BingoPgExternalBitset::_expandTo(int wordIndex)
{
    int wordsRequired = wordIndex + 1;
    if ((*_lastWordPtr) < wordsRequired)
    {
        (*_lastWordPtr) = wordsRequired;
    }
}

int BingoPgExternalBitset::_bitCount(qword b) const
{
    b = (b & 0x5555555555555555LL) + ((b >> 1) & 0x5555555555555555LL);
    b = (b & 0x3333333333333333LL) + ((b >> 2) & 0x3333333333333333LL);
    b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FLL;
    b = b + (b >> 8);
    b = b + (b >> 16);
    b = (b + (b >> 32)) & 0x0000007F;
    return (int)b;
}

int BingoPgExternalBitset::_leastSignificantBitPosition(qword n) const
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

void BingoPgExternalBitset::copy(const BingoPgExternalBitset& set)
{
    if (_length != set._length)
    {
        //      _length = set._length;
        //      _words.resize(_length);
        //      throw Error("different length");
        return;
    }
    _bitsNumber = set._bitsNumber;
    (*_lastWordPtr) = (*set._lastWordPtr);
    memcpy(_words, set._words, _length * sizeof(qword));
}

void BingoPgExternalBitset::copySubset(const BingoPgExternalBitset& set)
{
    if (_bitsNumber == set._bitsNumber)
        copy(set);
    if (_bitsNumber < set._bitsNumber)
        return;
    (*_lastWordPtr) = std::max((*_lastWordPtr), (*set._lastWordPtr));
    for (int i = 0; i < set._length; ++i)
    {
        _words[i] = set._words[i];
    }
}

void BingoPgExternalBitset::flip()
{
    flip(0, _bitsNumber);
}

void BingoPgExternalBitset::flip(int bitIndex)
{
    int wordindex = _wordIndex(bitIndex);
    bitIndex -= (wordindex << ADDRESS_BITS_PER_WORD);
    _expandTo(wordindex);

    _words[wordindex] ^= ((qword)1 << bitIndex);

    _recalculateWordsInUse();
}

void BingoPgExternalBitset::flip(int fromIndex, int toIndex)
{
    if (fromIndex == toIndex)
        return;

    int start_word_index = _wordIndex(fromIndex);
    int end_word_index = _wordIndex(toIndex - 1);
    _expandTo(end_word_index);

    fromIndex -= (start_word_index << ADDRESS_BITS_PER_WORD);
    toIndex -= (end_word_index << ADDRESS_BITS_PER_WORD);

    qword first_word_mask = WORD_MASK << fromIndex;
    qword last_word_mask = ((qword)1 << toIndex) - 1;

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

void BingoPgExternalBitset::set(int bitIndex)
{
    int wordindex = _wordIndex(bitIndex);
    bitIndex -= (wordindex << ADDRESS_BITS_PER_WORD);
    _expandTo(wordindex);
    _words[wordindex] |= ((qword)1 << bitIndex); // Restores invariants
}

void BingoPgExternalBitset::set(int fromIndex, int toIndex)
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

void BingoPgExternalBitset::set()
{
    set(0, _bitsNumber);
}

void BingoPgExternalBitset::set(int bitIndex, bool value)
{
    if (value)
        set(bitIndex);
    else
        reset(bitIndex);
}

void BingoPgExternalBitset::reset(int bitIndex)
{
    int wordindex = _wordIndex(bitIndex);
    bitIndex -= (wordindex << ADDRESS_BITS_PER_WORD);
    if (wordindex >= (*_lastWordPtr))
        return;

    _words[wordindex] &= ~((qword)1 << bitIndex);

    _recalculateWordsInUse();
}

void BingoPgExternalBitset::clear()
{
    while ((*_lastWordPtr) > 0)
        _words[--(*_lastWordPtr)] = 0;
}

bool BingoPgExternalBitset::get(int bitIndex) const
{
    int word_index = _wordIndex(bitIndex);
    bitIndex -= (word_index << ADDRESS_BITS_PER_WORD);
    return (word_index < (*_lastWordPtr)) && ((_words[word_index] & ((qword)1 << bitIndex)) != 0);
}

int BingoPgExternalBitset::nextSetBit(int fromIndex) const
{
    int u = _wordIndex(fromIndex);
    if (u >= (*_lastWordPtr))
        return -1;
    fromIndex -= (u << ADDRESS_BITS_PER_WORD);

    qword word = _words[u] & (WORD_MASK << fromIndex);
    for (; word == 0; word = _words[u])
    {
        if (++u >= (*_lastWordPtr))
            return -1;
    }
    return _leastSignificantBitPosition(word) + (u << ADDRESS_BITS_PER_WORD);
}

bool BingoPgExternalBitset::intersects(const BingoPgExternalBitset& set) const
{
    for (int i = std::min((*_lastWordPtr), (*set._lastWordPtr)) - 1; i >= 0; --i)
        if ((_words[i] & set._words[i]) != 0)
            return true;
    return false;
}

void BingoPgExternalBitset::andWith(const BingoPgExternalBitset& set)
{
    while ((*_lastWordPtr) > (*set._lastWordPtr))
        _words[--(*_lastWordPtr)] = 0;

    // Perform logical AND on words in common
    for (int i = 0; i < (*_lastWordPtr); ++i)
        _words[i] &= set._words[i];

    _recalculateWordsInUse();
}

void BingoPgExternalBitset::orWith(const BingoPgExternalBitset& set)
{
    if ((*_lastWordPtr) < (*set._lastWordPtr))
        (*_lastWordPtr) = (*set._lastWordPtr);

    // Perform logical OR on words in common
    for (int i = 0; i < (*_lastWordPtr); ++i)
    {
        _words[i] |= set._words[i];
    }
}

void BingoPgExternalBitset::xorWith(const BingoPgExternalBitset& set)
{
    if ((*_lastWordPtr) < (*set._lastWordPtr))
    {
        (*_lastWordPtr) = (*set._lastWordPtr);
    }

    // Perform logical XOR on words in common
    for (int i = 0; i < (*_lastWordPtr); ++i)
        _words[i] ^= set._words[i];

    _recalculateWordsInUse();
}

void BingoPgExternalBitset::andNotWith(const BingoPgExternalBitset& set)
{
    for (int i = std::min((*_lastWordPtr), (*set._lastWordPtr)) - 1; i >= 0; --i)
        _words[i] &= ~set._words[i];

    _recalculateWordsInUse();
}

bool BingoPgExternalBitset::equals(const BingoPgExternalBitset& set) const
{
    if ((*_lastWordPtr) != (*set._lastWordPtr))
        return false;

    // Check words in use by both BitSets
    for (int i = 0; i < (*_lastWordPtr); ++i)
        if (_words[i] != set._words[i])
            return false;

    return true;
}

// void ExternalBitset::resize(int size) {
//   int new_length = _wordIndex(size - 1) + 1;
//   _words.resize(new_length);
//   if (new_length > _length)
//      for (int i = _length; i < new_length; ++i)
//         _words[i] = 0;
//   _bitsNumber = size;
//   _length = new_length;
//}

bool BingoPgExternalBitset::isSubsetOf(const BingoPgExternalBitset& set) const
{
    for (int i = 0; i < (*_lastWordPtr); ++i)
        if (_words[i] & ~set._words[i])
            return false;
    return true;
}

bool BingoPgExternalBitset::isProperSubsetOf(const BingoPgExternalBitset& set) const
{
    bool proper = false;
    for (int i = 0; i < (*_lastWordPtr); ++i)
    {
        if (set._words[i] & ~_words[i])
            proper = true;
        if (_words[i] & ~set._words[i])
            return false;
    }
    return proper;
}

void BingoPgExternalBitset::zeroFill()
{
    memset(_words, 0, _length * sizeof(qword));
    _recalculateWordsInUse();
}

void BingoPgExternalBitset::bsOrBs(const BingoPgExternalBitset& set1, const BingoPgExternalBitset& set2)
{
    int max_words = std::max((*set1._lastWordPtr), (*set2._lastWordPtr));
    for (int i = 0; i < max_words; ++i)
    {
        _words[i] = set1._words[i] | set2._words[i];
    }
    for (int i = max_words; i < (*_lastWordPtr); ++i)
    {
        _words[i] = 0;
    }
    (*_lastWordPtr) = max_words;
}

void BingoPgExternalBitset::bsAndNotBs(const BingoPgExternalBitset& set1, const BingoPgExternalBitset& set2)
{
    for (int i = 0; i < (*set1._lastWordPtr); ++i)
    {
        _words[i] = set1._words[i] & ~set2._words[i];
    }
    for (int i = (*set1._lastWordPtr); i < (*_lastWordPtr); ++i)
    {
        _words[i] = 0;
    }
    _recalculateWordsInUse();
}

void BingoPgExternalBitset::bsAndBs(const BingoPgExternalBitset& set1, const BingoPgExternalBitset& set2)
{
    for (int i = 0; i < (*set1._lastWordPtr); ++i)
    {
        _words[i] = set1._words[i] & set2._words[i];
    }
    for (int i = (*set1._lastWordPtr); i < (*_lastWordPtr); ++i)
    {
        _words[i] = 0;
    }
    _recalculateWordsInUse();
}

int BingoPgExternalBitset::bitsNumber() const
{
    int bits_num = 0;
    //   for (int i = 0; i < (*_lastWordPtr); ++i)
    //      bits_num += _bitCount(_words[i]);
    int b_count = 0;
    byte q_size = sizeof(qword), byte_idx;
    byte* cur_word;
    for (int w_idx = 0; w_idx < (*_lastWordPtr); ++w_idx)
    {
        cur_word = (byte*)(_words + w_idx);
        b_count = 0;
        for (byte_idx = 0; byte_idx < q_size; ++byte_idx)
        {
            b_count += bitGetOnesCountByte(cur_word[byte_idx]);
        }

        bits_num += b_count;
    }
    return bits_num;
}

bool BingoPgExternalBitset::hasBits() const
{
    return (*_lastWordPtr) != 0;
}

// some 64-bit compilators can not correctly work with big values shift. So it must be processed manually

qword BingoPgExternalBitset::shiftOne(int shiftNumber)
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

BingoPgExternalBitset::Iterator::Iterator(BingoPgExternalBitset& self) : _fromWordIdx(0), _fromByteIdx(-1), _fromBitIdx(-1), _fromIndexes(0)
{
    _words = self._words;
    _wordsInUse = *self._lastWordPtr;
}

static indigo::ObjArray<indigo::Array<int>> all_indexes;

int BingoPgExternalBitset::Iterator::begin()
{
    if (all_indexes.size() == 0)
    {
        for (unsigned int buf = 0; buf < 256; ++buf)
        {
            indigo::Array<int>& indexes = all_indexes.push();
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

int BingoPgExternalBitset::Iterator::next()
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

void BingoPgExternalBitset::Iterator::_fillIndexes(byte buf, indigo::Array<int>& indexes)
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