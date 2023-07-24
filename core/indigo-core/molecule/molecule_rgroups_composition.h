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

#ifndef __molecule_rgroups_composition__
#define __molecule_rgroups_composition__

#include <memory>

#include "base_cpp/array.h"
#include "base_cpp/multimap.h"
#include "molecule/molecule.h"

namespace indigo
{

    class MoleculeRGroupsComposition : NonCopyable
    {
    public:
        explicit MoleculeRGroupsComposition(BaseMolecule& mol);
        ~MoleculeRGroupsComposition(){};

        // State of search abstracted from chemistry details
        class AttachmentIter : NonCopyable
        {
        public:
            // Default constructor is used to indicate final state
            AttachmentIter() : _limits(nullptr), _end(true), _size(-1)
            {
            }
            ~AttachmentIter()
            {
            }

            // Constructs (0,...,0) state, limits indicates max number on every site
            AttachmentIter(int size, const Array<int>& limits) : _limits(&limits), _end(false), _size(size)
            {
                _fragments.resize(size);
                _fragments.fill(0);
            }

            // Operators *, != and ++ are used for range-loop iteration
            const Array<int>* operator*() const;
            bool operator!=(const AttachmentIter& other) const;
            AttachmentIter& operator++();

            // Output numbers assigned to sites
            void dump(Array<int>& other) const;

            // Move to next state, returns true if the iterator has next element
            bool next();

        protected:
            const Array<int>* _limits;
            Array<int> _fragments;

            bool _end;
            int _size;
        };

        // Collection of search states
        class Attachments : NonCopyable
        {
        public:
            Attachments(int size, const Array<int>& limits) : _limits(limits), _end(), _size(size)
            {
            }
            ~Attachments()
            {
            }

            std::unique_ptr<AttachmentIter> begin_ptr() const
            {
                return std::unique_ptr<AttachmentIter>(_begin());
            }
            AttachmentIter& begin()
            {
                AttachmentIter* ptr = _begin();
                _ptrs.add(ptr);
                return *ptr;
            }
            AttachmentIter& end()
            {
                return _end;
            }

        private:
            AttachmentIter* _begin() const
            {
                return new AttachmentIter(_size, _limits);
            }
            const Array<int>& _limits;

            PtrPool<AttachmentIter> _ptrs;
            AttachmentIter _end;

            const int _size;
        };

        // Searchs all possible assignments of rgroup numbers to sites
        Attachments& attachments() const
        {
            _init();
            return *_ats.get();
        }

        // Assembles result molecule from abstract assigment of numbers to sites
        void decorate(const Array<int>& at, Molecule& out) const;
        void decorate(const AttachmentIter& at, Molecule& out) const;

        std::unique_ptr<Molecule> decorate(const Array<int>& at) const;
        std::unique_ptr<Molecule> decorate(const AttachmentIter& at) const;

        // Iterator for result molecules, essentially wrapper of AttachmentIter
        class MoleculeIter
        {
        public:
            MoleculeIter() = delete;
            MoleculeIter(AttachmentIter& at, const MoleculeRGroupsComposition& parent) : _parent(parent), _at(at)
            {
            }

            // Operators *, != and ++ are used for range-loop iteration
            std::unique_ptr<Molecule> operator*() const
            {
                return _parent.decorate(_at);
            }
            bool operator!=(const MoleculeIter& other) const
            {
                return _at != other._at;
            }
            MoleculeIter& operator++()
            {
                next();
                return *this;
            }

            // Assemble resulting molecule from attachment
            void dump(Molecule& out) const
            {
                _parent.decorate(_at, out);
            }

            // Shift to next molecule, returns true if there is next molecule after
            bool next() const
            {
                return _at.next();
            }

            /* Modify rgroups information according to options:
             * "composed" erase rgroups in resulting molecule (returns empty rgroups)
             * "source" leaves rgroups info as is (returns copy of source rgroups)
             * "ordered" places each attached fragment into its own rgroup
             * "" defaults to "composed" */
            std::unique_ptr<MoleculeRGroups> modifyRGroups(const char* options = "") const;

            enum Option
            {
                ERASE,
                LEAVE,
                ORDER
            };
#define RGCOMP_OPT MoleculeIter::Option
#define RGCOMP_OPT_ENUM                                                                                                                                        \
    {                                                                                                                                                          \
        RGCOMP_OPT::ERASE, RGCOMP_OPT::LEAVE, RGCOMP_OPT::ORDER                                                                                                \
    }
#define RGCOMP_OPT_COUNT 3

            static const char* const OPTION(Option opt)
            {
                switch (opt)
                {
                case ERASE:
                    return "composed";
                case LEAVE:
                    return "source";
                case ORDER:
                    return "ordered";
                default:
                    throw Error("Unknown option %s", opt);
                }
            }

        protected:
            const MoleculeRGroupsComposition& _parent;
            AttachmentIter& _at;

            class OrderedRGroups : public MoleculeRGroups
            {
            public:
                OrderedRGroups(const MoleculeIter& m);
            };

            class SourceRGroups : public MoleculeRGroups
            {
            public:
                SourceRGroups(const MoleculeIter& m);
            };
        };

        MoleculeIter begin() const
        {
            _init();
            return MoleculeIter(_ats->begin(), *this);
        }
        MoleculeIter end() const
        {
            _init();
            return MoleculeIter(_ats->end(), *this);
        }

        DECL_ERROR;

    protected:
        struct Fragment
        {
            int rgroup;
            int fragment;

            bool operator<(const Fragment& other) const
            {
                return std::pair<int, int>{rgroup, fragment} < std::pair<int, int>{other.rgroup, other.fragment};
            }
        };

        inline Fragment _fragment_coordinates(int rsite, int fragment) const
        {
            const RedBlackSet<int>& rs = _rsite2rgroup[_rsite2vertex.at(rsite)];

            int r = -1;
            int f = fragment;
            for (int i = rs.begin(); i != rs.end(); i = rs.next(i))
            {
                r = rs.key(i);
                int size = _rgroup2size[r];
                if (f >= size)
                {
                    f -= size;
                }
                else
                {
                    break;
                }
            }

            return {r, f};
        }

        inline BaseMolecule& _fragment(int rsite, int fragment) const
        {
            auto x = _fragment_coordinates(rsite, fragment);
            return *_rgroups.getRGroup(x.rgroup).fragments[x.fragment];
        }

    private:
        inline void _init() const
        {
            if (_ats.get() == nullptr)
            {
                _ats = std::make_unique<Attachments>(_rsites_count, _limits);
            }
        }

        BaseMolecule& _mol;
        MoleculeRGroups& _rgroups;

        Array<int> _limits;
        Array<int> _rgroup2size;
        MultiMap<int, int> _rsite2rgroup;
        RedBlackMap<int, int> _rsite2vertex;

        mutable std::unique_ptr<Attachments> _ats;

        int _rsites_count, _rgroups_count;
    };

} // namespace indigo

#ifdef _WIN32
#endif

#endif
