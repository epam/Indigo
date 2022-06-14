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

#include <indigo.h>

#include "IndigoObject.h"

namespace indigo_cpp
{
    class IndigoSession;

    template <class T>
    class IndigoIterator : public IndigoObject
    {
    private:
        std::shared_ptr<T> _current = nullptr;

    public:
        class iterator
        {
        private:
            IndigoIterator* _obj;

        public:
            using value_type = std::shared_ptr<T>;
            using reference = value_type&;
            using pointer = const value_type*;

            explicit iterator(IndigoIterator* obj) : _obj{obj}
            {
            }

            reference operator*()
            {
                return _obj->_current;
            }

            iterator& operator++()
            {
                increment();
                return *this;
            }

            bool operator!=(iterator rhs) const
            {
                return this->_obj != rhs._obj;
            }

        protected:
            void increment()
            {
                _obj->next();
                if (!_obj->valid())
                {
                    _obj = nullptr;
                }
            }
        };

        IndigoIterator(int id, IndigoSessionPtr session) : IndigoObject(id, std::move(session))
        {
        }

        friend class IndigoSession;

    public:
        iterator begin()
        {
            return ++iterator{this};
        }

        static iterator end()
        {
            return iterator{nullptr};
        }

        void next()
        {
            session()->setSessionId();
            auto nextId = session()->_checkResult(indigoNext(id()));
            if (nextId == 0)
            {
                _current = nullptr;
            }
            else
            {
                _current = std::make_shared<T>(nextId, session());
            }
        }

        bool valid() const
        {
            return _current != nullptr;
        }
    };
}
