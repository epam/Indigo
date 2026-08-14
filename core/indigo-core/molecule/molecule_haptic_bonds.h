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

#ifndef __molecule_haptic_bonds__
#define __molecule_haptic_bonds__

#include <list>
#include <unordered_set>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/exception.h"
#include "base_cpp/ptr_reusable_pool.h"
#include "base_cpp/reusable.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

// The bonds behind haptic connections: feature #3233, model #3837.
// Markush bonds (#3731) reuse the same bond with the other type.

namespace indigo
{
    class MoleculeAttachmentGroups;

    // A bond whose ends are not restricted to single atoms. Never an edge of the
    // graph: it appears in neither vertices() nor edges(), so no traversal of the
    // core has to learn to skip it.
    class DLLEXPORT HapticBond : public Reusable
    {
    public:
        // One end of the bond: a single atom, or an attachment group acting as
        // one. Exactly one of the two is set, which is why this is a type and not
        // a pair of indices.
        class Endpoint
        {
        public:
            static Endpoint atom(int idx);
            static Endpoint group(int idx);

            bool isGroup() const
            {
                return _is_group;
            }
            int index() const // atom index, or attachment group index
            {
                return _index;
            }

        private:
            bool _is_group = false;
            int _index = -1;
        };

        HapticBond();
        ~HapticBond() override;

        HapticBond(const HapticBond&) = delete;
        HapticBond& operator=(const HapticBond&) = delete;

        // Reusable: restore the default-constructed state.
        void reuse() override;

        // Initializing form used by the pool's add(begin, end, type).
        void reuse(Endpoint begin, Endpoint end, int type);

        const Endpoint& begin() const
        {
            return _begin;
        }
        const Endpoint& end() const
        {
            return _end;
        }

        // _BOND_HAPTIC or _BOND_VARIABLE_ATTACHMENT: an internal code, never a
        // format one. The value is validated by BaseMolecule::addHapticBond().
        int type() const
        {
            return _type;
        }
        void setType(int type)
        {
            _type = type;
        }

        bool referencesGroup(int group_idx) const;

        // Renumbers atom endpoints through `atom_mapping`, where -1 means the atom
        // is gone. Returns false if an endpoint atom is gone — the caller must then
        // drop the whole bond, never a half of it.
        bool remapAtoms(const Array<int>& atom_mapping);

        // Renumbers group endpoints through `group_mapping`, with the same rule.
        bool remapGroups(const Array<int>& group_mapping);

    private:
        void _reset(); // the one place the fields are listed; not virtual — called from the constructor

        Endpoint _begin;
        Endpoint _end;
        int _type;
    };

    // The haptic bonds of one molecule.
    //
    // Every removal path of BaseMolecule must reach this class: see
    // onAtomsRemoved() and onGroupRemoved().
    class DLLEXPORT MoleculeHapticBonds
    {
    public:
        DECL_ERROR;

        MoleculeHapticBonds();
        ~MoleculeHapticBonds();

        MoleculeHapticBonds(const MoleculeHapticBonds&) = delete;
        MoleculeHapticBonds& operator=(const MoleculeHapticBonds&) = delete;

        int add(HapticBond::Endpoint begin, HapticBond::Endpoint end, int type);

        HapticBond& at(int idx);
        const HapticBond& at(int idx) const;

        bool has(int idx) const;
        int count() const;
        void remove(int idx);

        // Index walk over the live bonds, mirroring the pool: end() is one past
        // the last slot, not the number of bonds.
        int begin() const;
        int next(int idx) const;
        int end() const;

        // Vertices of one bond: the begin endpoint first, the end endpoint last.
        // A group endpoint contributes its member atoms in insertion order.
        void collectVertices(int idx, const MoleculeAttachmentGroups& groups, std::vector<int>& vertices) const;

        // The first and the last of those vertices — the pair a consumer needs
        // when it wants the bond to look like an ordinary one. -1 when the
        // endpoint resolves to no atom at all (an empty group).
        int firstAtom(int idx, const MoleculeAttachmentGroups& groups) const;
        int lastAtom(int idx, const MoleculeAttachmentGroups& groups) const;

        // Must be called when the molecule removes atoms, with the same mapping
        // BaseMolecule::removeAtoms builds (-1 = removed). A bond that loses an
        // endpoint atom is dropped whole.
        void onAtomsRemoved(const Array<int>& atom_mapping);

        // Must be called for every attachment group the molecule removes: the
        // group pool recycles freed indices, so a bond left behind would silently
        // attach itself to the next group to take the index.
        void onGroupRemoved(int group_idx);

        // Copies the bonds of `other` that survive both mappings (-1 means
        // "dropped"), under the same all-or-nothing rule as remapAtoms().
        void mergeWithSubmolecule(const MoleculeHapticBonds& other, const Array<int>& atom_mapping, const Array<int>& group_mapping);

        // Atom sets that must be treated as connected even though no edge joins
        // them: each bond contributes the vertices of both its endpoints.
        // Feeds Graph::countComponents(external_neighbors).
        void collectConnectivitySets(const MoleculeAttachmentGroups& groups, std::list<std::unordered_set<int>>& neighbors) const;

        void clear();
        bool isEmpty() const;

    private:
        void _checkBond(int idx) const;
        static void _collectEndpointVertices(const HapticBond::Endpoint& endpoint, const MoleculeAttachmentGroups& groups, std::vector<int>& vertices);

        PtrReusablePool<HapticBond> _bonds;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
