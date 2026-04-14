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

#include <cstdlib>

#include "molecule/elements.h"
#include "molecule/valence_model.h"

using namespace indigo;

// ─── Shared helpers (moved from Element private methods) ─────────────────────

namespace
{

    bool isTransitionMetal(int elem)
    {
        return (elem >= ELEM_Sc && elem <= ELEM_Zn) || (elem >= ELEM_Y && elem <= ELEM_Cd) || (elem >= ELEM_La && elem <= ELEM_Hg) ||
               (elem >= ELEM_Ac && elem <= ELEM_Cn);
    }

    bool calcValenceGroup1(int charge, int rad, int conn, int& valence, int& hyd)
    {
        valence = 1;
        hyd = 1 - rad - conn - abs(charge);
        return hyd >= 0;
    }

    bool calcValenceGroup2(int charge, int rad, int conn, int& valence, int& hyd)
    {
        valence = 2;
        hyd = 0;
        if (conn == 0 && (rad + abs(charge) == 0 || rad + abs(charge) == 2))
            return true;
        if (conn == 2 && charge == 0 && rad == 0)
            return true;
        return false;
    }

    bool calcValenceLadder(int elem, int g, int charge, int rad, int conn, int& valence, int& hyd)
    {
        const int p = Element::period(elem);
        const int eff = Element::electrons(elem, charge);

        if (conn == 0 && rad == 0 && charge != 0 && (eff <= 0 || eff > 8))
        {
            valence = 0;
            hyd = 0;
            return true;
        }

        if (eff <= 0 || eff > 8)
            return false;

        const bool expand = (p >= 3 && g >= 3);
        const bool inert_pair = (elem == ELEM_Tl || elem == ELEM_Sn || elem == ELEM_Pb || elem == ELEM_Sb || elem == ELEM_Bi || elem == ELEM_Te);
        const bool halogen_no_hyper_h = (g == 7 && elem != ELEM_F);
        const bool noble_no_h = (g == 8);

        const int base = Element::baseValence(eff);

        // Inert pair: try (eff − 2) level before the main ladder
        if (inert_pair && eff >= 2)
        {
            const int ip_val = eff - 2;
            const int h = ip_val - rad - conn;
            if (h >= 0 && ip_val <= base && (!noble_no_h || h == 0))
            {
                valence = ip_val;
                hyd = h;
                return true;
            }
        }

        // BIOVIA: only pentafluoro/hexafluoro valid for group-4 dianions
        if (g == 4 && elem != ELEM_C && charge <= -2)
        {
            if (charge == -2 && (conn + rad == 5 || conn + rad == 6))
            {
                valence = conn + rad;
                hyd = 0;
                return true;
            }
            const int h = 4 - rad - conn - abs(charge);
            if (h >= 0)
            {
                valence = 4;
                hyd = h;
                return true;
            }
            return false;
        }

        // P⁻ conn=3: no known compounds
        if (elem == ELEM_P && charge == -1 && conn + rad == 3)
            return false;

        if (expand && eff > base)
        {
            for (int v = base; v <= eff; v += 2)
            {
                const int h = v - rad - conn;
                if (h < 0 || (noble_no_h && h > 0) || (halogen_no_hyper_h && v > base && h > 0))
                    continue;
                // BIOVIA: halogen radical valence = drawn bonds, not electronic level
                valence = (halogen_no_hyper_h && rad > 0 && h == 0) ? conn : v;
                hyd = h;
                return true;
            }

            // Fallback: same parity as base required for halogens/nobles
            const int total = conn + rad;
            if (total > 0 && total <= eff)
            {
                const bool needs_parity = (halogen_no_hyper_h || noble_no_h);
                if (!needs_parity || (total - base) % 2 == 0)
                {
                    valence = total;
                    hyd = 0;
                    return true;
                }
            }
        }

        if (!expand)
        {
            // Period 1-2: charge absorbed when base ≥ neutral, else only reduces H count
            const int neutral_base = (g <= 4) ? g : (8 - g);
            int v, h;
            if (base >= neutral_base)
            {
                v = base;
                h = base - rad - conn;
            }
            else
            {
                v = neutral_base;
                h = neutral_base - rad - conn - abs(charge);
            }
            if (h >= 0 && (!noble_no_h || h == 0))
            {
                valence = v;
                hyd = h;
                return true;
            }
        }
        else
        {
            const int h = base - rad - conn;
            if (h >= 0 && (!noble_no_h || h == 0))
            {
                valence = base;
                hyd = h;
                return true;
            }
        }

        return false;
    }

} // anonymous namespace

// ─── Template Method ─────────────────────────────────────────────────────────

bool ValenceModel::calcValence(int elem, int charge, int radical, int conn, int& valence, int& hyd, bool to_throw) const
{
    const int g = Element::group(elem);
    const int rad = Element::radicalElectrons(radical);

    valence = conn;
    hyd = 0;

    // d/f-block: accept as-is
    if (isTransitionMetal(elem))
        return true;

    // Virtual hook: Post-2014 intercepts main-group metals here
    if (interceptMainGroupMetal(elem, charge))
        return true;

    bool valid = false;
    if (g == 1)
        valid = calcValenceGroup1(charge, rad, conn, valence, hyd);
    else if (g == 2)
        valid = calcValenceGroup2(charge, rad, conn, valence, hyd);
    else
        valid = calcValenceLadder(elem, g, charge, rad, conn, valence, hyd);

    if (!valid)
    {
        if (to_throw)
            throw Element::Error("bad valence on %s having %d drawn bonds, charge %d, and %d radical electrons", Element::toString(elem), conn, charge, rad);
        valence = conn;
        hyd = 0;
    }
    return valid;
}

// ─── Factory ─────────────────────────────────────────────────────────────────

const ValenceModel& ValenceModel::instance(ValenceMode mode)
{
    static const Pre2014ValenceModel pre2014;
    static const Post2014ValenceModel post2014;
    switch (mode)
    {
    case ValenceMode::BIOVIA_2017:
        return post2014;
    default:
        return pre2014;
    }
}

// ─── Pre-2014: never intercept main-group metals ─────────────────────────────

bool Pre2014ValenceModel::interceptMainGroupMetal(int /*elem*/, int /*charge*/) const
{
    return false;
}

// ─── Post-2014: intercept elements NOT in BIOVIA valence table ───────────────

bool Post2014ValenceModel::interceptMainGroupMetal(int elem, int charge) const
{
    // BIOVIA post-2014 valence table: 22 non-metal elements that receive implicit H.
    // Elements marked true proceed with normal valence calculation;
    // elements marked false (main-group metals) get valence=conn, hyd=0.
    // clang-format off
    static constexpr bool kHasValence[ELEM_MAX] = {
        false,  // [0]   pseudo
        true,   // [1]   H
        true,   // [2]   He
        false,  // [3]   Li
        false,  // [4]   Be
        true,   // [5]   B
        true,   // [6]   C
        true,   // [7]   N
        true,   // [8]   O
        true,   // [9]   F
        true,   // [10]  Ne
        false,  // [11]  Na
        false,  // [12]  Mg
        false,  // [13]  Al  (exception: Al⁻ handled below)
        true,   // [14]  Si
        true,   // [15]  P
        true,   // [16]  S
        true,   // [17]  Cl
        true,   // [18]  Ar
        false,  // [19]  K
        false,  // [20]  Ca
        false,  // [21]  Sc  (transition metals — already handled upstream)
        false,  // [22]  Ti
        false,  // [23]  V
        false,  // [24]  Cr
        false,  // [25]  Mn
        false,  // [26]  Fe
        false,  // [27]  Co
        false,  // [28]  Ni
        false,  // [29]  Cu
        false,  // [30]  Zn
        false,  // [31]  Ga
        false,  // [32]  Ge
        true,   // [33]  As
        true,   // [34]  Se
        true,   // [35]  Br
        true,   // [36]  Kr
        false,  // [37]  Rb
        false,  // [38]  Sr
        false,  // [39]  Y   (transition metals)
        false,  // [40]  Zr
        false,  // [41]  Nb
        false,  // [42]  Mo
        false,  // [43]  Tc
        false,  // [44]  Ru
        false,  // [45]  Rh
        false,  // [46]  Pd
        false,  // [47]  Ag
        false,  // [48]  Cd
        false,  // [49]  In
        false,  // [50]  Sn
        false,  // [51]  Sb
        true,   // [52]  Te
        true,   // [53]  I
        true,   // [54]  Xe
        false,  // [55]  Cs
        false,  // [56]  Ba
        false,  // [57]  La  (transition metals / lanthanides)
        false,  // [58]  Ce
        false,  // [59]  Pr
        false,  // [60]  Nd
        false,  // [61]  Pm
        false,  // [62]  Sm
        false,  // [63]  Eu
        false,  // [64]  Gd
        false,  // [65]  Tb
        false,  // [66]  Dy
        false,  // [67]  Ho
        false,  // [68]  Er
        false,  // [69]  Tm
        false,  // [70]  Yb
        false,  // [71]  Lu
        false,  // [72]  Hf
        false,  // [73]  Ta
        false,  // [74]  W
        false,  // [75]  Re
        false,  // [76]  Os
        false,  // [77]  Ir
        false,  // [78]  Pt
        false,  // [79]  Au
        false,  // [80]  Hg
        false,  // [81]  Tl
        false,  // [82]  Pb
        false,  // [83]  Bi
        false,  // [84]  Po
        true,   // [85]  At
        true,   // [86]  Rn
        false,  // [87]  Fr
        false,  // [88]  Ra
        false,  // [89]  Ac  (transition metals / actinides)
        false,  // [90]  Th
        false,  // [91]  Pa
        false,  // [92]  U
        false,  // [93]  Np
        false,  // [94]  Pu
        false,  // [95]  Am
        false,  // [96]  Cm
        false,  // [97]  Bk
        false,  // [98]  Cf
        false,  // [99]  Es
        false,  // [100] Fm
        false,  // [101] Md
        false,  // [102] No
        false,  // [103] Lr
        false,  // [104] Rf
        false,  // [105] Db
        false,  // [106] Sg
        false,  // [107] Bh
        false,  // [108] Hs
        false,  // [109] Mt
        false,  // [110] Ds
        false,  // [111] Rg
        false,  // [112] Cn
        false,  // [113] Nh
        false,  // [114] Fl
        false,  // [115] Mc
        false,  // [116] Lv
        false,  // [117] Ts
        false,  // [118] Og
    };
    // clang-format on

    // Exception: Al⁻ gets default valence 4 per BIOVIA spec
    if (elem == ELEM_Al && charge == -1)
        return false;

    // Non-metals in BIOVIA table (neutral or charged) → normal valence calculation
    if (elem > 0 && elem < ELEM_MAX && kHasValence[elem])
        return false;

    return true; // metal or unknown → intercept (valence=conn, hyd=0)
}
