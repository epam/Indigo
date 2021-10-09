#pragma once

#include <memory>

//#include "IndigoMolecule.h"
#include "IndigoObject.h"
//#include "IndigoSession.h"

namespace indigo_cpp
{
    class IndigoMolecule;
    class IndigoSession;

    class IndigoSDFileIterator : public IndigoObject
    {
    private:
        std::unique_ptr<IndigoMolecule> _current = nullptr;

    public:
        class iterator
        {
        private:
            IndigoSDFileIterator* _obj;

        public:
            using value_type = IndigoMolecule;
            using reference = const value_type&;
            using pointer = const value_type*;
            using iterator_category = std::input_iterator_tag;

            explicit iterator(IndigoSDFileIterator* obj);

            reference operator*() const;
            iterator& operator++();

            bool operator==(iterator rhs) const;
            bool operator!=(iterator rhs) const;

        protected:
            void increment();
        };

    private:
        IndigoSDFileIterator(int id, const IndigoSession& session);

        friend class IndigoSession;

    public:
        iterator begin();
        static iterator end();

        void next();

        bool valid() const;
    };
}
