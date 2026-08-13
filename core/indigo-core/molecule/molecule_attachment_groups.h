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

#ifndef __molecule_attachment_groups__
#define __molecule_attachment_groups__

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

namespace indigo
{
    // Attachment mode of a multi-endpoint bond, the `ATTACH=` keyword of a
    // CTfile V3000 bond record.
    //
    // The mode is the only thing that separates the two features built on
    // attachment groups, so the group carries it from the start even though
    // only All is acted upon today: All is a haptic bond (#3233), which
    // reaches every atom of the group at once; Any is a Markush bond (#3731),
    // which reaches one unspecified atom of it. Loading a file must be able to
    // record what it saw without deciding whether the feature is implemented.
    enum class AttachmentMode
    {
        All,
        Any
    };

    // A set of atoms acting as one collective endpoint of a bond.
    //
    // The group is deliberately NOT a vertex of the molecular graph: making it
    // one would force every algorithm that walks vertices()/edges() (valence,
    // aromaticity, canonical SMILES, InChI, fingerprints, substructure search)
    // to learn to skip it. Keeping it beside the graph makes those algorithms
    // correct by construction. Its position is likewise not stored — it is
    // always derived from the member atoms, so it cannot drift out of date when
    // the ligand moves.
    class DLLEXPORT AttachmentGroup : public Reusable
    {
    public:
        // A bond from this group to an atom outside it.
        //
        // Bonds live inside the group rather than in a side table because such
        // a bond is meaningless without the collective endpoint it starts
        // from: composition makes "no bond outlives its group" a property of
        // the data layout instead of an invariant every removal path has to
        // remember to enforce.
        struct Bond
        {
            int atom;  // partner atom, index in the owning molecule
            int order; // bond order as carried by the file formats
        };

        AttachmentGroup();
        ~AttachmentGroup() override;

        AttachmentGroup(const AttachmentGroup&) = delete;
        AttachmentGroup& operator=(const AttachmentGroup&) = delete;

        // Reusable: restore the default-constructed state, keeping the buffers.
        void reuse() override;

        // Initializing form used by the pool's add(mode).
        void reuse(AttachmentMode mode);

        AttachmentMode mode() const
        {
            return _mode;
        }
        void setMode(AttachmentMode mode)
        {
            _mode = mode;
        }

        // Member atoms, in insertion order. Exposed read-only: membership is
        // changed through addAtom()/setAtoms() so the group can keep itself
        // duplicate-free.
        const std::vector<int>& atoms() const
        {
            return _atoms;
        }
        bool hasAtom(int atom) const;
        void addAtom(int atom);
        void setAtoms(const std::vector<int>& atoms);

        const std::vector<Bond>& bonds() const
        {
            return _bonds;
        }
        void addBond(int atom, int order);
        void removeBondsToAtom(int atom);

        // Aggregate charge and radical of the group.
        //
        // A pi system carries them as a property of the system as a whole (the
        // position inside it is not significant, per BIOVIA Chemical
        // Representation), and file formats put them on the star atom that
        // denotes the group. That atom is absorbed on load, so without this the
        // charge would be dropped and the total charge of, say, ferrocene would
        // no longer balance.
        int charge() const
        {
            return _charge;
        }
        void setCharge(int charge)
        {
            _charge = charge;
        }
        int radical() const
        {
            return _radical;
        }
        void setRadical(int radical)
        {
            _radical = radical;
        }

    private:
        std::vector<int> _atoms;
        std::vector<Bond> _bonds;
        AttachmentMode _mode;
        int _charge;
        int _radical;
    };

    // The attachment groups of one molecule, plus the marks that tell which
    // ordinary bonds are haptic.
    //
    // The two live together because they are two encodings of one feature and
    // share a lifetime: an atom-to-atom haptic bond is a real edge of the graph
    // (that is what it is chemically, and it keeps the fragment connected for
    // layout, rendering and canonicalization), so it is stored as a mark on
    // that edge rather than as a one-atom group; a group-to-atom haptic bond
    // has no edge to mark and lives in the group. Owning both here keeps
    // clearing, merging and skipping the feature a single operation.
    class DLLEXPORT MoleculeAttachmentGroups
    {
    public:
        DECL_ERROR;

        MoleculeAttachmentGroups();
        ~MoleculeAttachmentGroups();

        MoleculeAttachmentGroups(const MoleculeAttachmentGroups&) = delete;
        MoleculeAttachmentGroups& operator=(const MoleculeAttachmentGroups&) = delete;

        // --- groups ---------------------------------------------------------

        int addGroup(AttachmentMode mode = AttachmentMode::All);

        AttachmentGroup& group(int idx);
        const AttachmentGroup& group(int idx) const;

        bool hasGroup(int idx) const;
        int groupCount() const;

        // Removes the group and, with it, its bonds. Indices of the other
        // groups are unaffected: callers (loaders, savers, the public API)
        // hold group indices across such calls.
        void removeGroup(int idx);

        // Index walk over the live groups, mirroring the pool: end() is one
        // past the last slot, not the number of groups.
        int begin() const;
        int next(int idx) const;
        int end() const;

        // --- haptic marks on ordinary bonds ---------------------------------

        void setBondHaptic(int bond_idx, bool haptic = true);
        bool isBondHaptic(int bond_idx) const;
        int hapticBondCount() const;

        // --- whole-container operations -------------------------------------

        void clear();
        bool isEmpty() const;

        // Copies the groups and marks of `other` that survive a submolecule
        // mapping (-1 in a mapping means "dropped").
        //
        // A group is copied only when EVERY member atom survives: a haptic bond
        // means "all atoms of this group at once", so a truncated group would
        // silently change what the bond says. A bond of a surviving group is
        // copied only when its partner atom survives too.
        void mergeWithSubmolecule(const MoleculeAttachmentGroups& other, const Array<int>& atom_mapping, const Array<int>& bond_mapping);

        // Atom sets that must be treated as connected even though no edge
        // joins them: each group contributes its members plus the partners of
        // its bonds. Feeds Graph::countComponents(external_neighbors), which
        // otherwise would split a haptic complex (ferrocene) into a metal and
        // two loose rings.
        void collectConnectivitySets(std::list<std::unordered_set<int>>& neighbors) const;

    private:
        PtrReusablePool<AttachmentGroup> _groups;
        std::unordered_set<int> _haptic_bonds;
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
