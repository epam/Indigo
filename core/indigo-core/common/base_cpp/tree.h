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

#ifndef __tree_h__
#define __tree_h__

#include "non_copyable.h"
#include "obj_array.h"

namespace indigo
{

    class Tree : public NonCopyable
    {
    public:
        explicit Tree(int nlabel)
        {
            this->label = nlabel;
        }

        Tree() : Tree(-1)
        {
        }

        void insert(int nlabel, int parent)
        {
            Tree* present = _find(parent);
            if (present != nullptr)
            {
                present->_insert(nlabel);
            }
            else
            {
                _insert(parent)._insert(nlabel);
            }
        }

        void insert(int nlabel)
        {
            insert(nlabel, -1);
        }

        ObjArray<Tree>& children()
        {
            return _children;
        }

        inline const ObjArray<Tree>& children() const
        {
            return _children;
        }

        int label;

        inline Tree* find(int nlabel)
        {
            return _find(nlabel);
        }

    protected:
        Tree& _insert(int nlabel)
        {
            return _children.push(nlabel);
        }

        Tree* _find(int nlabel)
        {
            if (this->label == nlabel)
            {
                return this;
            }
            for (auto i = 0; i < _children.size(); i++)
            {
                Tree& child = _children[i];
                if (child.label == nlabel)
                {
                    return &child;
                }
                Tree* deeper = child._find(nlabel);
                if (deeper != nullptr)
                {
                    return deeper;
                }
            }
            return nullptr;
        }

        ObjArray<Tree> _children;
    };

} // namespace indigo

#endif