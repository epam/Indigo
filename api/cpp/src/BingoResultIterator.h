#pragma once

#include "IndigoSession.h"

#include <string>

#include "BingoResult.h"

namespace indigo_cpp
{
    template <typename target_t>
    class BingoResultIterator
    {
    public:
        BingoResultIterator(int id, IndigoSessionPtr session);
        ~BingoResultIterator();

        void next();

        bool valid() const;

        class iterator
        {
        private:
            BingoResultIterator<target_t>* _obj;

        public:
            using iterator_category = std::input_iterator_tag;

            explicit iterator(BingoResultIterator* obj);

            BingoResult<target_t>& operator*();

            iterator& operator++();

            bool operator==(iterator rhs) const;

            bool operator!=(iterator rhs) const;
        };

        iterator begin();
        static iterator end();

    private:
        int id;
        IndigoSessionPtr session;
        std::shared_ptr<BingoResult<target_t>> _current;
    };

    using BingoMoleculeResultIterator = BingoResultIterator<IndigoMolecule>;
}
