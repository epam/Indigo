#pragma once

#include <memory>

#include "IndigoObject.h"
#include "IndigoReaction.h"

namespace indigo_cpp
{
    class IndigoReaction;
    class IndigoSession;

    class IndigoRDFileIterator : public IndigoObject
    {
    private:
        IndigoReactionPtr _current = nullptr;

    public:
        class iterator
        {
        private:
            IndigoRDFileIterator* _obj;

        public:
            using value_type = IndigoReactionPtr;
            using reference = value_type&;
            using pointer = const value_type*;
            // using iterator_category = std::input_iterator_tag;

            explicit iterator(IndigoRDFileIterator* obj);

            reference operator*();
            iterator& operator++();

            bool operator==(iterator rhs) const;
            bool operator!=(iterator rhs) const;

        protected:
            void increment();
        };

    private:
        IndigoRDFileIterator(int id, IndigoSessionPtr session);

        friend class IndigoSession;

    public:
        iterator begin();
        static iterator end();

        void next();

        bool valid() const;
    };
}
