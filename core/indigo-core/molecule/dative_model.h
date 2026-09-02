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

#ifndef __dative_model_h__
#define __dative_model_h__

#include "base_c/defs.h"
#include "base_cpp/array.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{
    class Molecule;

    // Element data for the dative-bond valence model (ticket #3617).
    //
    // These two tables are used ONLY by the dative model. They deliberately disagree
    // with the tables the rest of Indigo uses, and that divergence must be preserved:
    //
    //   electrons of Cu        Element::electrons()  -> 1  (old-style group number)
    //                          Element::getNumOuterElectrons() -> 1  (d10 treated as core)
    //                          dative::valenceElectrons()      -> 11 (d10 counted as valent)
    //
    //   orbitals of a lanthanide  Element::orbitals() -> 4  (no f-orbital concept)
    //                             dative::valenceOrbitals()   -> 16 (4f + 5d + 6s + 6p)
    //
    // The same atom therefore reports different counts to different subsystems, on
    // purpose: valence, hybridization and dative bonding answer different questions.
    // Do NOT "unify" these tables with the existing ones — see Task/3617 ANALYSIS-REPORT
    // section 3.4 for the decision and its rationale.
    namespace dative
    {
        // El0 — number of valence electrons of a neutral atom.
        // Unlike Element::getNumOuterElectrons(), a filled d10/f14 subshell counts as
        // valent here (Cu = 11, Zn = 12, Yb = 16).
        // Returns 0 for element numbers outside [ELEM_MIN, ELEM_MAX).
        DLLEXPORT int valenceElectrons(int elem);

        // Or0 — number of orbitals in the valence layer. One of {1, 4, 9, 16}:
        //   1  — H, He                     (1s)
        //   4  — main group                (ns, np)
        //   9  — d-block, incl. group 2 of period >= 3 and Lu/Lr rows  ((n-1)d, ns, np)
        //   16 — lanthanides and actinides (4f/5f, 5d/6d, 6s/7s, 6p/7p)
        //
        // Note: p-block elements of period >= 3 get 4, not 9 — hypervalency is
        // deliberately out of scope for this model, so Or can legitimately go negative
        // for e.g. [SiF6]2-. That is accepted behaviour: the spec's own `Or >= n > 0`
        // guards then report "cannot participate in dative bonding". Do not clamp it and
        // do not add d-orbitals here without the spec owner's approval (Q2, won't-fix).
        // Returns 0 for element numbers outside [ELEM_MIN, ELEM_MAX).
        DLLEXPORT int valenceOrbitals(int elem);
    }

    // Valence contribution of dative bonds (ticket #3617).
    //
    // Built once per molecule, then queried per atom. The molecule passed in is never
    // modified: requirement 1.1 ("dearomatize, calculate, aromatize again") is satisfied
    // by asking MoleculeDearomatizer for the connectivity the dearomatized structure
    // would have, which needs no copy of the molecule and no mutation of it.
    //
    // Scope rule (see ANALYSIS-REPORT section 3.5): this model applies ONLY to atoms that
    // carry at least one dative bond. Every other atom must keep its existing behaviour
    // byte for byte. The gate that enforces that is Molecule::_atomHasDativeBond, and it
    // deliberately lives there rather than here: it has to be answerable without building
    // a model, because it is asked on every valence query of every molecule.
    //
    // compute() therefore assumes its caller has already passed that gate. It computes
    // what the specification says for the atom it is given and does not second-guess
    // whether the atom should have been asked about.
    class DLLEXPORT DativeModel
    {
    public:
        struct AtomResult
        {
            int el = 0;                 // req 1: electrons eligible for dative bonding
            int orb = 0;                // req 2: orbitals eligible; may be negative (see Q2)
            int max_donor = 0;          // req 4: n_donor, 0 when the Or >= n > 0 gate fails
            int max_acceptor = 0;       // req 5: n_acceptor, 0 when the gate fails
            int donor_bonds = 0;        // req 3: donor bonds left after cancellation
            int acceptor_bonds = 0;     // req 3: acceptor bonds left after cancellation
            bool valence_error = false; // req 6: more dative bonds than allowed
            int implicit_h = -1;        // req 7; -1 when the count does not apply (req 8)
        };

        explicit DativeModel(Molecule& mol);

        // Computes the per-atom result. Returns false when it cannot be computed
        // (kekulization failed, or the atom is a pseudo-atom / R-site / template);
        // the caller then keeps the existing behaviour.
        bool compute(int atom_idx, AtomResult& out) const;

    private:
        // Requirements 7 and 8: the hydrogens implied by what is left once the dative
        // bonds have taken their share of the electrons and orbitals. Returns -1 when the
        // count does not apply to this atom.
        static int _implicitHydrogens(int elem, const AtomResult& result);

        // Sum of the orders of all bonds that count towards `bonds` in req 1 and 2:
        // dative and hydrogen bonds excluded, implicit hydrogens excluded, aromatic
        // bonds resolved to their Kekule orders (req 1.1).
        int _countBonds(int atom_idx) const;

        Molecule& _mol;
        Array<int> _kekule_connectivity; // filled only when the molecule has aromatic bonds
        bool _has_aromatic = false;
        bool _usable = true;
    };
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif
