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

namespace indigo
{
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
}

#endif
