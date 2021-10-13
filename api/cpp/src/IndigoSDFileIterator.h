#pragma once

#include <memory>

#include "IndigoMolecule.h"
#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoMolecule;
    class IndigoSession;

    class IndigoSDFileIterator : public IndigoObject
    {
    private:
        IndigoMoleculeSPtr _current = nullptr;

    public:
        class iterator
        {
        private:
            IndigoSDFileIterator* _obj;

        public:
            using value_type = IndigoMoleculeSPtr;
            using reference = value_type&;
            using pointer = const value_type*;
            // using iterator_category = std::input_iterator_tag;

            explicit iterator(IndigoSDFileIterator* obj);

            reference operator*();
            iterator& operator++();

            bool operator==(iterator rhs) const;
            bool operator!=(iterator rhs) const;

        protected:
            void increment();
        };

    private:
        IndigoSDFileIterator(int id, IndigoSessionPtr session);

        friend class IndigoSession;

    public:
        iterator begin();
        static iterator end();

        void next();

        bool valid() const;
    };
}
