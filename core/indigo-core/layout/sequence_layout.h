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

#ifndef __sequence_layout_h__
#define __sequence_layout_h__

#include "base_cpp/cancellation_handler.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"

#include <deque>
#include <queue>
#include <vector>

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class DLLEXPORT SequenceLayout
    {
    public:
        class SequenceRow
        {
        public:
            int operator[](int index)
            {
                return _atoms[index];
            }

            SequenceRow(int column) : _range(column, column)
            {
            }

            void addLeft(int column, int atom_idx)
            {
                // TODO: check if column already exists, handle fragmentation
                _range.first--;
                _atoms.push_front(atom_idx);
            }

            void addRight(int column, int atom_idx)
            {
                // TODO: check if column already exists, handle fragmentation
                _range.second++;
                _atoms.push_back(atom_idx);
            }

            int leftMost()
            {
                if (_atoms.size() == 0)
                    throw Error("No leftmost element");
                return _range.first;
            }

            int rightMost()
            {
                if (_atoms.size() == 0)
                    throw Error("No rightmost element");
                return _range.second;
            }

            size_t size()
            {
                return _atoms.size();
            }

        private:
            std::deque<int> _atoms;
            std::pair<int, int> _range;
        };

        class SequenceTable
        {
        public:
            SequenceTable() : _range(0, 0)
            {
            }

            SequenceRow& at(int column, int row)
            {
                if (_table.size() == 0)
                {
                    _range.first = row;
                    _range.second = row;
                    _table.emplace_back( column );
                }
                else if (row == _range.first - 1)
                {
                    _range.first--;
                    _table.emplace_front( column );
                }
                else if (row == _range.second + 1)
                {
                    _range.first++;
                    _table.emplace_back( column );
                }
                else
                    throw Error("Range can't be extended with more than one row per request");

                return _table[row];
            }

        private:
            std::pair<int, int> _range;
            std::deque<SequenceRow> _table;
        };

        static constexpr float DEFAULT_BOND_LENGTH = 1.6f;

        explicit SequenceLayout(BaseMolecule& molecule);
        void make();
        void make(int first_atom_idx);
        void setCancellationHandler(CancellationHandler* cancellation);

        DECL_ERROR;

    protected:
        BaseMolecule& _molecule;
        SequenceTable _layout_sequence;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
