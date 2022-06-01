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
