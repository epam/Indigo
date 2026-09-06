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

#include "molecule/dative_model.h"

#include <array>

#include "molecule/elements.h"

using namespace indigo;

namespace
{
    // Both tables are indexed by element number; index 0 is the pseudo-element slot.
    // Values are taken verbatim from the specification of ticket #3617 (issue body as of
    // 2026-08-04) — the frozen revision this implementation targets.

    // clang-format off
    constexpr std::array<int, ELEM_MAX> VALENCE_ELECTRONS{
        0,  // [0]   pseudo-element
        1,  // [1]   H
        2,  // [2]   He
        1,  // [3]   Li
        2,  // [4]   Be
        3,  // [5]   B
        4,  // [6]   C
        5,  // [7]   N
        6,  // [8]   O
        7,  // [9]   F
        8,  // [10]  Ne
        1,  // [11]  Na
        2,  // [12]  Mg
        3,  // [13]  Al
        4,  // [14]  Si
        5,  // [15]  P
        6,  // [16]  S
        7,  // [17]  Cl
        8,  // [18]  Ar
        1,  // [19]  K
        2,  // [20]  Ca
        3,  // [21]  Sc
        4,  // [22]  Ti
        5,  // [23]  V
        6,  // [24]  Cr
        7,  // [25]  Mn
        8,  // [26]  Fe
        9,  // [27]  Co
        10, // [28]  Ni
        11, // [29]  Cu  d10 counted as valent (Element::getNumOuterElectrons gives 1)
        12, // [30]  Zn  d10 counted as valent
        3,  // [31]  Ga
        4,  // [32]  Ge
        5,  // [33]  As
        6,  // [34]  Se
        7,  // [35]  Br
        8,  // [36]  Kr
        1,  // [37]  Rb
        2,  // [38]  Sr
        3,  // [39]  Y
        4,  // [40]  Zr
        5,  // [41]  Nb
        6,  // [42]  Mo
        7,  // [43]  Tc
        8,  // [44]  Ru
        9,  // [45]  Rh
        10, // [46]  Pd
        11, // [47]  Ag  d10 counted as valent
        12, // [48]  Cd  d10 counted as valent
        3,  // [49]  In
        4,  // [50]  Sn
        5,  // [51]  Sb
        6,  // [52]  Te
        7,  // [53]  I
        8,  // [54]  Xe
        1,  // [55]  Cs
        2,  // [56]  Ba
        3,  // [57]  La
        4,  // [58]  Ce
        5,  // [59]  Pr
        6,  // [60]  Nd
        7,  // [61]  Pm
        8,  // [62]  Sm
        9,  // [63]  Eu
        10, // [64]  Gd
        11, // [65]  Tb
        12, // [66]  Dy
        13, // [67]  Ho
        14, // [68]  Er
        15, // [69]  Tm
        16, // [70]  Yb  f14 counted as valent (Element::getNumOuterElectrons gives 2)
        3,  // [71]  Lu
        4,  // [72]  Hf
        5,  // [73]  Ta
        6,  // [74]  W
        7,  // [75]  Re
        8,  // [76]  Os
        9,  // [77]  Ir
        10, // [78]  Pt
        11, // [79]  Au  d10 counted as valent
        12, // [80]  Hg  d10 counted as valent
        3,  // [81]  Tl
        4,  // [82]  Pb
        5,  // [83]  Bi
        6,  // [84]  Po
        7,  // [85]  At
        8,  // [86]  Rn
        1,  // [87]  Fr
        2,  // [88]  Ra
        3,  // [89]  Ac
        4,  // [90]  Th
        5,  // [91]  Pa
        6,  // [92]  U
        7,  // [93]  Np
        8,  // [94]  Pu
        9,  // [95]  Am
        10, // [96]  Cm
        11, // [97]  Bk
        12, // [98]  Cf
        13, // [99]  Es
        14, // [100] Fm
        15, // [101] Md
        16, // [102] No  f14 counted as valent
        3,  // [103] Lr
        4,  // [104] Rf
        5,  // [105] Db
        6,  // [106] Sg
        7,  // [107] Bh
        8,  // [108] Hs
        9,  // [109] Mt
        10, // [110] Ds
        11, // [111] Rg  homolog of Au
        12, // [112] Cn  homolog of Hg
        3,  // [113] Nh
        4,  // [114] Fl
        5,  // [115] Mc
        6,  // [116] Lv
        7,  // [117] Ts
        8   // [118] Og
    };

    constexpr std::array<int, ELEM_MAX> VALENCE_ORBITALS{
        0,  // [0]   pseudo-element
        1,  // [1]   H   1s
        1,  // [2]   He  1s
        4,  // [3]   Li  2s 2p
        4,  // [4]   Be
        4,  // [5]   B
        4,  // [6]   C
        4,  // [7]   N
        4,  // [8]   O
        4,  // [9]   F
        4,  // [10]  Ne
        4,  // [11]  Na  3s 3p
        9,  // [12]  Mg  3d 4s 4p
        4,  // [13]  Al  3s 3p
        4,  // [14]  Si  no d-orbitals: hypervalency out of scope (Q2)
        4,  // [15]  P   no d-orbitals
        4,  // [16]  S   no d-orbitals
        4,  // [17]  Cl  no d-orbitals
        4,  // [18]  Ar
        4,  // [19]  K   4s 4p
        9,  // [20]  Ca  3d 4s 4p
        9,  // [21]  Sc
        9,  // [22]  Ti
        9,  // [23]  V
        9,  // [24]  Cr
        9,  // [25]  Mn
        9,  // [26]  Fe
        9,  // [27]  Co
        9,  // [28]  Ni
        9,  // [29]  Cu
        9,  // [30]  Zn
        4,  // [31]  Ga  4s 4p
        4,  // [32]  Ge
        4,  // [33]  As
        4,  // [34]  Se
        4,  // [35]  Br
        4,  // [36]  Kr
        4,  // [37]  Rb  5s 5p
        9,  // [38]  Sr  4d 5s 5p
        9,  // [39]  Y
        9,  // [40]  Zr
        9,  // [41]  Nb
        9,  // [42]  Mo
        9,  // [43]  Tc
        9,  // [44]  Ru
        9,  // [45]  Rh
        9,  // [46]  Pd
        9,  // [47]  Ag
        9,  // [48]  Cd
        4,  // [49]  In  5s 5p
        4,  // [50]  Sn
        4,  // [51]  Sb
        4,  // [52]  Te
        4,  // [53]  I
        4,  // [54]  Xe
        4,  // [55]  Cs  6s 6p
        9,  // [56]  Ba  5d 6s 6p
        16, // [57]  La  4f 5d 6s 6p
        16, // [58]  Ce
        16, // [59]  Pr
        16, // [60]  Nd
        16, // [61]  Pm
        16, // [62]  Sm
        16, // [63]  Eu
        16, // [64]  Gd
        16, // [65]  Tb
        16, // [66]  Dy
        16, // [67]  Ho
        16, // [68]  Er
        16, // [69]  Tm
        16, // [70]  Yb
        9,  // [71]  Lu  5d 6s 6p
        9,  // [72]  Hf
        9,  // [73]  Ta
        9,  // [74]  W
        9,  // [75]  Re
        9,  // [76]  Os
        9,  // [77]  Ir
        9,  // [78]  Pt
        9,  // [79]  Au
        9,  // [80]  Hg
        4,  // [81]  Tl  6s 6p
        4,  // [82]  Pb
        4,  // [83]  Bi
        4,  // [84]  Po
        4,  // [85]  At
        4,  // [86]  Rn
        4,  // [87]  Fr  7s 7p
        9,  // [88]  Ra  6d 7s 7p
        16, // [89]  Ac  5f 6d 7s 7p
        16, // [90]  Th
        16, // [91]  Pa
        16, // [92]  U
        16, // [93]  Np
        16, // [94]  Pu
        16, // [95]  Am
        16, // [96]  Cm
        16, // [97]  Bk
        16, // [98]  Cf
        16, // [99]  Es
        16, // [100] Fm
        16, // [101] Md
        16, // [102] No
        9,  // [103] Lr  6d 7s 7p
        9,  // [104] Rf
        9,  // [105] Db
        9,  // [106] Sg
        9,  // [107] Bh
        9,  // [108] Hs
        9,  // [109] Mt
        9,  // [110] Ds
        9,  // [111] Rg
        9,  // [112] Cn
        4,  // [113] Nh  7s 7p
        4,  // [114] Fl
        4,  // [115] Mc
        4,  // [116] Lv
        4,  // [117] Ts
        4   // [118] Og
    };
    // clang-format on

    inline bool isKnownElement(int elem)
    {
        return elem >= ELEM_MIN && elem < ELEM_MAX;
    }
}

int dative::valenceElectrons(int elem)
{
    return isKnownElement(elem) ? VALENCE_ELECTRONS[elem] : 0;
}

int dative::valenceOrbitals(int elem)
{
    return isKnownElement(elem) ? VALENCE_ORBITALS[elem] : 0;
}
