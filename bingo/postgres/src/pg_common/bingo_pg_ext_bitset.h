/*
 */

#ifndef _BINGO_PG_EXT_BITSET_H__
#define _BINGO_PG_EXT_BITSET_H__

#include "base_cpp/array.h"
#include "base_cpp/exception.h"

class BingoPgExternalBitset
{
public:
    BingoPgExternalBitset();
    BingoPgExternalBitset(int nbits);

    inline int begin() const
    {
        return nextSetBit(0);
    }
    inline int end() const
    {
        return -1;
    }
    inline int next(int idx) const
    {
        return nextSetBit(idx + 1);
    }

    virtual ~BingoPgExternalBitset()
    {
    }

    void* serialize(int& size);
    void deserialize(void* data, int data_len, bool ext);

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
        return (*_lastWordPtr) == 0;
    };
    // Returns true if the specified BitSet has any bits set to true
    // that are also set to true in this BitSet
    bool intersects(const BingoPgExternalBitset& set) const;
    // Performs a logical AND of this target BitSet with the argument BitSet
    void andWith(const BingoPgExternalBitset& set);
    // Performs a logical OR of this target BitSet with the argument BitSet
    void orWith(const BingoPgExternalBitset& set);
    // Performs a logical XOR of this target BitSet with the argument BitSet
    void xorWith(const BingoPgExternalBitset& set);
    // Clears all of the bits in this BitSet whose corresponding
    // bit is set in the specified BitSet.
    void andNotWith(const BingoPgExternalBitset& set);
    // Returns the number of bits of space actually in use by this
    // BitSet to represent bit values
    int size() const
    {
        return _bitsNumber;
    };
    // Compares this BitSet against the specified BitSet.
    bool equals(const BingoPgExternalBitset& set) const;
    // Cloning this BitSet produces a new BitSet
    void copy(const BingoPgExternalBitset& set);
    // copy part of BitSet
    void copySubset(const BingoPgExternalBitset& set);
    // resizes this BitSet
    //   void resize(int size);
    // checks if this BitSet is subset of argument BitSet
    bool isSubsetOf(const BingoPgExternalBitset& set) const;
    // checks if this BitSet is proper subset of argument BitSet
    bool isProperSubsetOf(const BingoPgExternalBitset& set) const;

    // fills with false all bits in this BitSet
    void zeroFill();

    void bsOrBs(const BingoPgExternalBitset& set1, const BingoPgExternalBitset& set2);
    // Clears all of the bits in first argument BitSet whose corresponding
    // bits is set in the specified second argument BitSet. Result -> this
    void bsAndNotBs(const BingoPgExternalBitset& set1, const BingoPgExternalBitset& set2);
    // Performs a logical AND of two argument BitSets. result saves to this Bitset
    void bsAndBs(const BingoPgExternalBitset& set1, const BingoPgExternalBitset& set2);
    int bitsNumber() const;
    bool hasBits() const;

    qword shiftOne(int shiftNumber);

private:
    BingoPgExternalBitset(const BingoPgExternalBitset&); // no implicit copy

    // bitsets are packed into arrays of "words."  Currently a word is
    // a long long, which consists of 64 bits, requiring 6 address bits.
    // The choice of word size is determined purely by performance concerns.

    enum
    {
        ADDRESS_BITS_PER_WORD = 6,
        BITS_PER_WORD = 1 << ADDRESS_BITS_PER_WORD,
        MAX_SHIFT_NUMBER = 63
    };
    static const qword WORD_MASK = 0xFFFFFFFFFFFFFFFFULL;

    // the number of words in the logical size of this BitSet.
    qword* _lastWordPtr;
    qword _lastWord;
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

    inline int _wordsInUse()
    {
        return (int)*_lastWordPtr;
    };

    qword* _words;

    indigo::Array<qword> _internalWords;
    indigo::Array<qword> _serializeWords;

public:
    class Iterator
    {
    public:
        Iterator(BingoPgExternalBitset&);

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
        void _fillIndexes(byte buf, indigo::ArrayInt& indexes);
        int _wordsInUse;
        qword* _words;

        int _fromWordIdx;
        int _fromByteIdx;
        int _fromBitIdx;
        qword* _fromWord;
        indigo::ArrayInt* _fromIndexes;

        int _shiftByte;
        int _shiftWord;

    private:
        Iterator(const Iterator&); // no implicit copy
    };
};

#endif /* EXT_BITSET_H */
