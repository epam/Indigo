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

#include <array>
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

    // Hydrogen uses the BIOVIA-Draw convention: a bare neutral H is treated as
    // elemental H2 (hyd=1), so "M RAD ... 2" on a bare H still serialises as "[HH]".
    // Radical-electron info is intentionally ignored for isolated Hs; the parametrised
    // case H_q0_rD_c0_radical (which would prescribe hyd=0) is a documented spec conflict.
    bool calcValenceHydrogen(int charge, int conn, int& valence, int& hyd)
    {
        valence = 1;
        if ((charge == 1 && conn == 0) || (charge == -1 && conn == 0) || (charge == 0 && conn == 1))
            hyd = 0;
        else if (charge == 0 && conn == 0)
            hyd = 1;
        else
            return false;
        return true;
    }

    bool calcValenceGroup2(int charge, int rad, int conn, int& valence, int& hyd)
    {
        valence = 2;
        hyd = 0;
        if (conn == 0 && (rad + abs(charge) == 0 || rad + abs(charge) == 2) || (conn == 2 && charge == 0 && rad == 0))
            return true;
        return false;
    }

    // Carbon — always val=4, the simplest of the group 4 cases.
    bool calcValenceCarbon(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_C)
            return false;
        valence = 4;
        hyd = 4 - rad - conn - abs(charge);
        return hyd >= 0;
    }

    // Si / Ge / Sn / Pb — mirrors master's groupno==4 non-C branch with its pentafluoro-/
    // hexafluorosilicate/germanate overrides and the Sn/Pb ns² inert-pair fast path.
    bool calcValenceTetrelHeavy(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_Si && elem != ELEM_Ge && elem != ELEM_Sn && elem != ELEM_Pb)
            return false;
        if (charge == -2 && conn == 6 && rad == 0)
        {
            valence = 6;
            hyd = 0;
        }
        else if (charge == -2 && conn + rad == 5)
        {
            valence = 5;
            hyd = 0;
        }
        else if (charge == -1 && conn + rad == 5)
        {
            valence = 5;
            hyd = 0;
        }
        else if (charge == -1 && conn + rad == 4 && elem == ELEM_Si)
        {
            valence = 5;
            hyd = 1;
        }
        else if ((elem == ELEM_Sn || elem == ELEM_Pb) && conn + rad + abs(charge) <= 2)
        {
            valence = 2;
            hyd = 2 - rad - conn - abs(charge);
        }
        else
        {
            valence = 4;
            hyd = 4 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Oxygen — positive charge absorbs into hyd on a fixed val=3 shell; else val=2.
    bool calcValenceOxygen(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_O)
            return false;
        if (charge >= 1)
        {
            valence = 3;
            hyd = 3 - rad - conn;
        }
        else
        {
            valence = 2;
            hyd = 2 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Sb / Bi / As (group 5) charged overrides. Without these the generic ladder
    // picks the post-cation noble-shell base ([Sb+1] eff=4 → val=4) instead of
    // the inert-pair/lone-pair form that InChI and gross-formula tables expect
    // (val=2 for Sb/Bi+, val=3 for Sb/Bi/As²⁺).
    bool calcValencePnictogen(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_Sb && elem != ELEM_Bi && elem != ELEM_As)
            return false;
        if (charge == -1 && rad + conn == 6)
        {
            valence = 6;
            hyd = 0;
            return true;
        }
        if (charge == 1)
        {
            if (rad + conn <= 2 && elem != ELEM_As)
            {
                valence = 2;
                hyd = 2 - rad - conn;
            }
            else
            {
                valence = 4;
                hyd = 4 - rad - conn;
            }
            return hyd >= 0;
        }
        if (charge == 2)
        {
            valence = 3;
            hyd = 3 - rad - conn;
            return hyd >= 0;
        }
        if (charge == -2 && rad + conn == 5)
        {
            valence = 5;
            hyd = 0;
            return true;
        }
        // Default branch mirrors the group-5 heavy-pnictogen rule: absorb charge into hyd
        // and promote to val=5 when the drawn load is too large for val=3.
        if (rad + conn + abs(charge) <= 3)
        {
            valence = 3;
            hyd = 3 - rad - conn - abs(charge);
        }
        else
        {
            valence = 5;
            hyd = 5 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Nitrogen / phosphorus — identical structure to master's groupno==5 N/P branch.
    // Note the phosphorus −1 ladder (phosphanide → hexachlorophosphate) and the P-only
    // val=5 promotion when conn+rad+|q| exceeds 3.
    bool calcValenceNitrogenPhosphorus(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_N && elem != ELEM_P)
            return false;
        if (charge == 1)
        {
            valence = 4;
            hyd = 4 - rad - conn;
            return hyd >= 0;
        }
        if (charge == 2)
        {
            valence = 3;
            hyd = 3 - rad - conn;
            return hyd >= 0;
        }
        if (charge == -1 && elem == ELEM_P)
        {
            if (rad + conn <= 2)
            {
                valence = 2;
                hyd = 2 - rad - conn;
            }
            else if (rad + conn == 3)
            {
                return false; // phosphanide with one extra bond has no known analogues
            }
            else if (rad + conn == 4)
            {
                valence = 4;
                hyd = 0;
            }
            else if (rad + conn <= 6)
            {
                valence = 6;
                hyd = 6 - rad - conn;
            }
            else
            {
                return false;
            }
            return hyd >= 0;
        }
        if (elem == ELEM_N || rad + conn + abs(charge) <= 3)
        {
            valence = 3;
            hyd = 3 - rad - conn - abs(charge);
        }
        else
        {
            valence = 5;
            hyd = 5 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Boron group (B, Al, Ga, In) — Tl is handled separately via calcValenceThallium.
    // Mirrors master's groupno==3 non-Tl branch including the Al²⁻ pentafluoroaluminate
    // exception and the group-wide charge=−3 "bare drawn-shell" override.
    bool calcValenceBoronGroup(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_B && elem != ELEM_Al && elem != ELEM_Ga && elem != ELEM_In)
            return false;
        if (charge == -1)
        {
            valence = 4;
            hyd = 4 - rad - conn;
        }
        else if (charge == -3 && elem != ELEM_B && rad + conn <= 6)
        {
            valence = rad + conn;
            hyd = 0;
        }
        else if (elem == ELEM_Al && charge == -2)
        {
            if (rad + conn == 5)
            {
                valence = 5;
                hyd = 0;
            }
            else
            {
                return false; // Al²⁻ only valid as pentafluoroaluminate(2−)
            }
        }
        else
        {
            valence = 3;
            hyd = 3 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Thallium keeps its own rules: ns² inert-pair gives val=2 at charge ±1 and val=3/5
    // splits at charge=−2, plus the ISIS/Marvin Tl(−3) hexa-coordinate override.
    bool calcValenceThallium(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_Tl)
            return false;
        if (charge == -1)
        {
            if (rad + conn <= 2)
            {
                valence = 2;
                hyd = 2 - rad - conn;
            }
            else
            {
                valence = 4;
                hyd = 4 - rad - conn;
            }
        }
        else if (charge == -2)
        {
            if (rad + conn <= 3)
            {
                valence = 3;
                hyd = 3 - rad - conn;
            }
            else
            {
                valence = 5;
                hyd = 5 - rad - conn;
            }
        }
        else if (charge == -3 && rad + conn == 6)
        {
            valence = 6;
            hyd = 0;
        }
        else if (rad + conn + abs(charge) <= 1)
        {
            valence = 1;
            hyd = 1 - rad - conn - abs(charge);
        }
        else
        {
            valence = 3;
            hyd = 3 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Heavy chalcogens (S, Se, Po) — mirrors master's rule ladder for cations at
    // charge=+1 (val=3 or 5 by conn) and anions at charge=−1 (val=1/3/5/7 steps).
    // The default branch absorbs any non-handled charge into hyd.
    bool calcValenceChalcogenHeavy(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_S && elem != ELEM_Se && elem != ELEM_Po)
            return false;
        if (charge == 1)
        {
            if (conn <= 3)
            {
                valence = 3;
                hyd = 3 - rad - conn;
            }
            else
            {
                valence = 5;
                hyd = 5 - rad - conn;
            }
            return hyd >= 0;
        }
        if (charge == -1)
        {
            if (conn + rad <= 1)
            {
                valence = 1;
                hyd = 1 - rad - conn;
            }
            else if (conn + rad <= 3)
            {
                valence = 3;
                hyd = 3 - rad - conn;
            }
            else if (conn + rad <= 5)
            {
                valence = 5;
                hyd = 5 - rad - conn;
            }
            else
            {
                valence = 7;
                hyd = 7 - rad - conn;
            }
            return hyd >= 0;
        }
        if (conn + rad + abs(charge) <= 2)
        {
            valence = 2;
            hyd = 2 - rad - conn - abs(charge);
        }
        else if (conn + rad + abs(charge) <= 4)
        {
            valence = 4;
            hyd = 4 - rad - conn - abs(charge);
        }
        else
        {
            valence = 6;
            hyd = 6 - rad - conn - abs(charge);
        }
        return hyd >= 0;
    }

    // Tellurium — branch's improved spec over master: charges ±1 use a hypervalent ladder
    // (val ∈ {1,3,5,7} for −1, {3,5,7} for +1) so Te⁻ conn=3 and Te⁺ conn=4 round-trip.
    // Charges 0 and +2 keep master's specific rules; others fall back to bare drawn form.
    bool calcValenceTellurium(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_Te)
            return false;
        // Master's Te branch (groupno==6, ELEM_Te) is INTENTIONALLY narrower than the
        // S/Se/Po ladder. Tracking each charge case keeps bingo's full-periodic-table
        // tests (Q798/Q814) numerically identical to master.
        if (charge == -1)
        {
            // Only the documented PubChem cases: TeF7⁻ (CID 4191414), TeF5⁻ pseudo, and
            // bare/single-coord Te⁻. Anything else master rejects via hyd=-1.
            if (rad + conn == 7)
            {
                valence = 7;
                hyd = 0;
                return true;
            }
            if (rad + conn == 5)
            {
                valence = 5;
                hyd = 0;
                return true;
            }
            valence = 1;
            hyd = 1 - rad - conn;
            return hyd >= 0;
        }
        if (charge == 1)
        {
            // Master allows only val=3 here ("no known cases of 5-connected [Te+]").
            valence = 3;
            hyd = 3 - rad - conn;
            return hyd >= 0;
        }
        if (charge == 2)
        {
            if (conn + rad == 4)
            {
                valence = conn + rad;
                hyd = 0;
            }
            else
            {
                valence = 2;
                hyd = 2 - conn - rad;
            }
            return hyd >= 0;
        }
        if (charge == 0)
        {
            if (conn + rad <= 2)
            {
                valence = 2;
                hyd = 2 - conn - rad;
            }
            else if (conn + rad <= 4)
            {
                valence = 4;
                hyd = 4 - conn - rad;
            }
            else
            {
                valence = 6;
                hyd = 6 - conn - rad;
            }
            return hyd >= 0;
        }
        // No BIOVIA rule for Te at |charge| ≥ 3 or charge=−2 ⇒ keep bare drawn form.
        valence = conn;
        hyd = 0;
        return true;
    }

    // Fluorine: always val=1; every electron accounted for in rad+conn+|charge|.
    bool calcValenceFluorine(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_F)
            return false;
        valence = 1;
        hyd = 1 - rad - conn - abs(charge);
        return hyd >= 0;
    }

    // Heavy halogens (Cl, Br, I, At) — narrow set of allowed {charge, conn} combos.
    // Anything outside keeps the bare drawn form (master returns the init values).
    bool calcValenceHalogenHeavy(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_Cl && elem != ELEM_Br && elem != ELEM_I && elem != ELEM_At)
            return false;
        if (charge == 1)
        {
            if (conn <= 2)
            {
                valence = 2;
                hyd = 2 - rad - conn;
                return hyd >= 0;
            }
            if (conn == 3 || conn == 5 || conn >= 7)
                return false; // master sets hyd=-1 → bad valence
            // conn in {4, 6} — master falls through with valence=conn, hyd=0 default.
            valence = conn;
            hyd = 0;
            return rad == 0;
        }
        if (charge == 0)
        {
            if (conn <= 1)
            {
                valence = 1;
                hyd = 1 - rad - conn;
                return hyd >= 0;
            }
            if (conn == 2 || conn == 4 || conn == 6)
            {
                if (rad == 1)
                {
                    valence = conn;
                    hyd = 0;
                    return true;
                }
                return false; // even-conn halogen without a radical is invalid
            }
            if (conn > 7)
                return false;
            // conn ∈ {3, 5, 7} — master falls through with init values; accept as bare.
            valence = conn;
            hyd = 0;
            return true;
        }
        // |charge| ≥ 2 or any other charge: no BIOVIA rule, keep bare.
        valence = conn;
        hyd = 0;
        return hyd >= 0;
    }

    // Noble gases — neutral atoms are inert; hypervalent even-bonded species (XeF2/4/6,
    // KrF2, etc.) are accepted via the ladder v ∈ {base, base+2, …, eff} with hyd≡0.
    // The hypervalent ladder requires DRAWN bonds (conn > 0): bare radical or charged
    // noble-gas atoms have no chemistry — master's formula `hyd = -rad - conn - |q|`
    // rejects them, and we mirror that by returning false when conn == 0 with any
    // anomaly. checkmolecule then surfaces "bad valence on Ar...2 radical electrons" etc.
    bool calcValenceNobleGas(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem != ELEM_He && elem != ELEM_Ne && elem != ELEM_Ar && elem != ELEM_Kr && elem != ELEM_Xe && elem != ELEM_Rn && elem != ELEM_Og)
            return false;
        if (charge == 0 && rad == 0 && conn == 0)
        {
            valence = 0;
            hyd = 0;
            return true;
        }
        // Bare anomaly (no drawn bonds) — reject so checkmolecule reports it.
        if (conn == 0)
            return false;
        const int eff = Element::electrons(elem, charge);
        if (eff <= 0 || eff > 8)
            return false; // extreme overcharge is handled upstream by tryBareIsolatedIon
        const int base = Element::baseValence(eff);
        if (eff <= base)
            return false; // no room to expand; anything drawn is NONSTD
        for (int v = base; v <= eff; v += 2)
        {
            if (v - rad - conn == 0)
            {
                valence = v;
                hyd = 0;
                return true;
            }
        }
        return false;
    }

    // Invariants computed once per atom before any ladder branch runs.
    struct LadderContext
    {
        int elem;
        int g;
        int charge;
        int rad;
        int conn;
        int eff;
        int base;
        bool expand;
        bool inert_pair;
        bool halogen_no_hyper_h;
        bool noble_no_h;
    };

    LadderContext buildLadderContext(int elem, int g, int charge, int rad, int conn)
    {
        const int p = Element::period(elem);
        const int eff = Element::electrons(elem, charge);
        LadderContext ctx{};
        ctx.elem = elem;
        ctx.g = g;
        ctx.charge = charge;
        ctx.rad = rad;
        ctx.conn = conn;
        ctx.eff = eff;
        ctx.base = (eff > 0 && eff <= 8) ? Element::baseValence(eff) : 0;
        ctx.expand = (p >= 3 && g >= 3);
        ctx.inert_pair = (elem == ELEM_Tl || (elem == ELEM_Te && charge >= 2));
        ctx.halogen_no_hyper_h = (g == 7 && elem != ELEM_F);
        ctx.noble_no_h = (g == 8) || (eff == 8 && charge < 0);
        return ctx;
    }

    // Isolated multi-charged cation (eff ≤ 0) or over-saturated anion (eff > 8) with
    // no bonds. Returns a neutral group-base valence so that fingerprints, comparators
    // and SMILES serialisation see a stable handle (e.g. [In+3] → val=3) instead of 0.
    bool tryIsolatedIonFallback(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (ctx.conn != 0 || ctx.rad != 0 || ctx.charge == 0)
            return false;
        if (ctx.eff > 0 && ctx.eff <= 8)
            return false;
        const int neutral_eff = Element::electrons(ctx.elem, 0);
        valence = (neutral_eff > 0 && neutral_eff <= 8) ? Element::baseValence(neutral_eff) : 0;
        hyd = 0;
        return true;
    }

    // Prefer the ns² lower oxidation state (val = eff − 2) for inert-pair elements whenever
    // the resulting hyd is non-negative. Only Tl and Te²⁺ take this path here; Sn/Pb are
    // handled upstream by calcValenceTetrelInertPair, Sb/Bi by calcValencePnictogen.
    bool tryInertPairPreferred(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (!ctx.inert_pair || ctx.eff < 2)
            return false;
        const int ip_val = ctx.eff - 2;
        const int h = ip_val - ctx.rad - ctx.conn;
        if (h < 0 || (ctx.noble_no_h && h != 0))
            return false;
        valence = ip_val;
        hyd = h;
        return true;
    }

    // Legacy rule: O with positive charge keeps valence 3 and absorbs the charge into the
    // hydrogen count (e.g. [OH2+5]). Reference in basic/buffer_string_load_iterate.py.
    bool tryLegacyOxygenPositive(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (ctx.elem != ELEM_O || ctx.charge < 1)
            return false;
        const int h = 3 - ctx.rad - ctx.conn;
        if (h < 0)
            return false;
        valence = 3;
        hyd = h;
        return true;
    }

    // BIOVIA rule for group-4 dianions (excluding C): conn+rad = 5 or 6 give a direct
    // pentafluoro/hexafluoro shell; otherwise the charge is absorbed into the H count.
    // `handled` signals the caller to stop the ladder regardless of success.
    bool tryGroup4Dianion(const LadderContext& ctx, int& valence, int& hyd, bool& handled)
    {
        handled = (ctx.g == 4 && ctx.elem != ELEM_C && ctx.charge <= -2);
        if (!handled)
            return false;
        if (ctx.charge == -2 && (ctx.conn + ctx.rad == 5 || ctx.conn + ctx.rad == 6))
        {
            valence = ctx.conn + ctx.rad;
            hyd = 0;
            return true;
        }
        const int h = 4 - ctx.rad - ctx.conn - abs(ctx.charge);
        if (h < 0)
            return false;
        valence = 4;
        hyd = h;
        return true;
    }

    // Primary ladder for p ≥ 3 elements: step through base, base+2, base+4 …, picking the
    // first rung where h = v − rad − conn fits the noble/halogen constraints.
    bool pickExpandLadderRung(const LadderContext& ctx, int& valence, int& hyd)
    {
        for (int v = ctx.base; v <= ctx.eff; v += 2)
        {
            const int h = v - ctx.rad - ctx.conn;
            if (h < 0 || (ctx.noble_no_h && h > 0) || (ctx.halogen_no_hyper_h && v > ctx.base && h > 0))
                continue;
            // BIOVIA: halogen radical valence = drawn bonds, not the electronic level.
            valence = (ctx.halogen_no_hyper_h && ctx.rad > 0 && h == 0 && ctx.conn > 0) ? ctx.conn : v;
            hyd = h;
            return true;
        }
        return false;
    }

    // Parity fallback when no ladder rung fits: halogens and noble gases require same
    // parity as base; for halogen radicals the radical electron does not shift parity.
    bool tryExpandParityFallback(const LadderContext& ctx, int& valence, int& hyd)
    {
        const int total = ctx.conn + ctx.rad;
        if (total <= 0 || total > ctx.eff)
            return false;
        const bool needs_parity = (ctx.halogen_no_hyper_h || ctx.noble_no_h);
        if (const int parity_val = (needs_parity && ctx.rad > 0) ? ctx.conn : total; needs_parity && (parity_val - ctx.base) % 2 != 0)
            return false;
        valence = total;
        hyd = 0;
        return true;
    }

    // Full-octet species (eff = 8, charged): halide anions, chalcogenide dianions, etc.
    // Noble-gas electron config — accept any connectivity with h = 0.
    bool tryFullOctetAnion(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (ctx.eff != 8 || ctx.charge == 0 || ctx.conn <= 0 || ctx.conn > 8)
            return false;
        valence = ctx.conn;
        hyd = 0;
        return true;
    }

    // p ≥ 3 expanded path: primary rung → parity fallback → full-octet fallback.
    bool tryExpandedLadder(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (!ctx.expand || ctx.eff <= ctx.base)
            return false;
        if (pickExpandLadderRung(ctx, valence, hyd))
            return true;
        if (tryExpandParityFallback(ctx, valence, hyd))
            return true;
        return tryFullOctetAnion(ctx, valence, hyd);
    }

    // Period 1-2 path: charge is absorbed only when base < neutral (otherwise it just
    // reduces the H count). `tryLegacyOxygenPositive` is consulted first.
    bool tryPeriod12(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (tryLegacyOxygenPositive(ctx, valence, hyd))
            return true;
        const int neutral_base = (ctx.g <= 4) ? ctx.g : (8 - ctx.g);
        const int v = (ctx.base >= neutral_base) ? ctx.base : neutral_base;
        const int h = (ctx.base >= neutral_base) ? (ctx.base - ctx.rad - ctx.conn) : (neutral_base - ctx.rad - ctx.conn - abs(ctx.charge));
        if (h < 0 || (ctx.noble_no_h && h != 0))
            return false;
        valence = v;
        hyd = h;
        return true;
    }

    // p ≥ 3 plain path (when the expand block didn't fit): just base, no octet push.
    bool tryPeriod3PlusPlain(const LadderContext& ctx, int& valence, int& hyd)
    {
        const int h = ctx.base - ctx.rad - ctx.conn;
        if (h < 0 || (ctx.noble_no_h && h != 0))
            return false;
        valence = ctx.base;
        hyd = h;
        return true;
    }

    // Fallback inert-pair form for high-charge / high-connectivity cases that still
    // have a valid ns² shell — e.g. Tl⁻ with 3 bonds — after the main ladder missed.
    bool tryInertPairFallback(const LadderContext& ctx, int& valence, int& hyd)
    {
        if (!ctx.inert_pair || ctx.eff < 2)
            return false;
        const int ip_val = ctx.eff - 2;
        if (ip_val > ctx.base)
            return false;
        const int h = ip_val - ctx.rad - ctx.conn;
        if (h < 0 || (ctx.noble_no_h && h != 0))
            return false;
        valence = ip_val;
        hyd = h;
        return true;
    }

    bool calcValenceLadder(int elem, int g, int charge, int rad, int conn, int& valence, int& hyd)
    {
        const LadderContext ctx = buildLadderContext(elem, g, charge, rad, conn);

        if (tryIsolatedIonFallback(ctx, valence, hyd))
            return true;
        if (ctx.eff <= 0 || ctx.eff > 8)
            return false;
        if (tryInertPairPreferred(ctx, valence, hyd))
            return true;

        bool g4_dianion = false;
        if (const bool g4_result = tryGroup4Dianion(ctx, valence, hyd, g4_dianion); g4_dianion)
            return g4_result;

        // P⁻ conn=3: no known compounds.
        if (elem == ELEM_P && charge == -1 && conn + rad == 3)
            return false;

        if (tryExpandedLadder(ctx, valence, hyd))
            return true;

        const bool plain_fits = ctx.expand ? tryPeriod3PlusPlain(ctx, valence, hyd) : tryPeriod12(ctx, valence, hyd);
        if (plain_fits)
            return true;

        return tryInertPairFallback(ctx, valence, hyd);
    }

    // Bare isolated ion with eff outside [1, 8] (e.g. [N+5], [B-6]) — use the neutral
    // group-base valence as a stable handle so downstream consumers see a well-defined
    // value instead of zero. Noble gases are excluded: master rejects [Ar-]/[Xe+] as
    // bad valence (no chemistry justifies them) and checkmolecule must surface that.
    bool tryBareIsolatedIon(int elem, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (conn != 0 || rad != 0 || charge == 0)
            return false;
        if (Element::group(elem) == 8)
            return false;
        if (const int eff = Element::electrons(elem, charge); eff > 0 && eff <= 8)
            return false;
        const int neutral_eff = Element::electrons(elem, 0);
        valence = (neutral_eff > 0 && neutral_eff <= 8) ? Element::baseValence(neutral_eff) : 0;
        hyd = 0;
        return true;
    }

    // Element-specific rule lookup. Returns true iff a helper matched AND succeeded.
    // A `false` return can mean either "no helper for this element" or "helper rejected
    // the atom"; the caller disambiguates by trying the isolated-ion fallback next.
    bool dispatchElementSpecificRule(int elem, int g, int charge, int rad, int conn, int& valence, int& hyd)
    {
        if (elem == ELEM_H)
            return calcValenceHydrogen(charge, conn, valence, hyd);
        if (g == 1)
            return calcValenceGroup1(charge, rad, conn, valence, hyd);
        if (g == 2)
            return calcValenceGroup2(charge, rad, conn, valence, hyd);
        if (elem == ELEM_B || elem == ELEM_Al || elem == ELEM_Ga || elem == ELEM_In)
            return calcValenceBoronGroup(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_Tl)
            return calcValenceThallium(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_C)
            return calcValenceCarbon(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_Si || elem == ELEM_Ge || elem == ELEM_Sn || elem == ELEM_Pb)
            return calcValenceTetrelHeavy(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_N || elem == ELEM_P)
            return calcValenceNitrogenPhosphorus(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_As || elem == ELEM_Sb || elem == ELEM_Bi)
            return calcValencePnictogen(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_O)
            return calcValenceOxygen(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_S || elem == ELEM_Se || elem == ELEM_Po)
            return calcValenceChalcogenHeavy(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_Te)
            return calcValenceTellurium(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_F)
            return calcValenceFluorine(elem, charge, rad, conn, valence, hyd);
        if (elem == ELEM_Cl || elem == ELEM_Br || elem == ELEM_I || elem == ELEM_At)
            return calcValenceHalogenHeavy(elem, charge, rad, conn, valence, hyd);
        if (g == 8)
            return calcValenceNobleGas(elem, charge, rad, conn, valence, hyd);
        return calcValenceLadder(elem, g, charge, rad, conn, valence, hyd);
    }

    // Main-group dispatch: element-specific rule only. Extreme bare ions (e.g. [N+5],
    // [B-6], [H+2]) intentionally have NO fallback — master rejects them and bingo
    // checkmolecule expects "bad valence on …" to surface. Use indigoSetOption
    // ("ignore-bad-valence", "true") to suppress the throw at higher level if needed.
    bool dispatchMainGroupRule(int elem, int g, int charge, int rad, int conn, int& valence, int& hyd)
    {
        return dispatchElementSpecificRule(elem, g, charge, rad, conn, valence, hyd);
    }

} // anonymous namespace

// ─── Template Method ─────────────────────────────────────────────────────────

bool ValenceModel::calcValence(int elem, int charge, int radical, int conn, int& valence, int& hyd, bool to_throw, bool* nonStandard) const
{
    const int g = Element::group(elem);
    const int rad = Element::radicalElectrons(radical);

    valence = conn;
    hyd = 0;

    // d/f-block: accept as-is (standard)
    if (isTransitionMetal(elem))
    {
        if (nonStandard)
            *nonStandard = false;
        return true;
    }

    // Virtual hook: BIOVIA_2017 intercepts main-group metals here
    if (interceptMainGroupMetal(elem, charge))
    {
        if (nonStandard)
            *nonStandard = false;
        return true;
    }

    if (const bool valid = dispatchMainGroupRule(elem, g, charge, rad, conn, valence, hyd); !valid)
    {
        if (nonStandard)
            *nonStandard = true;
        if (to_throw)
            throw Element::Error("bad valence on %s having %d drawn bonds, charge %d, and %d radical electrons", Element::toString(elem), conn, charge, rad);
        // Hybrid contract: collapse valence to drawn bonds so downstream serializers
        // still produce a stable, round-trippable structure, but signal the anomaly
        // via the bool return (legacy callers in molecule.cpp/molecule_arom.cpp/
        // query_molecule.cpp depend on `false = bad valence`). Tolerant new callers
        // can opt into `*nonStandard=true` and treat the result as accepted.
        valence = conn;
        hyd = 0;
        return false;
    }

    if (nonStandard)
        *nonStandard = false;
    return true;
}

// ─── Factory ─────────────────────────────────────────────────────────────────

const ValenceModel& ValenceModel::instance(ValenceMode mode)
{
    static const DefaultValenceModel defaultModel;
    static const Biovia2017ValenceModel biovia2017;
    return (mode == ValenceMode::BIOVIA_2017) ? static_cast<const ValenceModel&>(biovia2017) : static_cast<const ValenceModel&>(defaultModel);
}

// ─── BIOVIA_2009: never intercept main-group metals ─────────────────────────

bool DefaultValenceModel::interceptMainGroupMetal(int elem, int /*charge*/) const
{
    // BIOVIA_2009 compatibility: these post-2014 main-group elements have no
    // legacy implicit-hydrogen valence rules in BIOVIA_2009 mode.
    switch (elem)
    {
    case ELEM_Nh:
    case ELEM_Fl:
    case ELEM_Mc:
    case ELEM_Lv:
    case ELEM_Ts:
        return true;
    default:
        break;
    }
    return false;
}

// ─── BIOVIA_2017: intercept elements NOT in BIOVIA valence table ─────────────

bool Biovia2017ValenceModel::interceptMainGroupMetal(int elem, int charge) const
{
    // BIOVIA 2017 valence table: 22 non-metal elements that receive implicit H.
    // Elements marked true proceed with normal valence calculation;
    // elements marked false (main-group metals) get valence=conn, hyd=0.
    // clang-format off
    static constexpr std::array<bool, ELEM_MAX> kHasValence = {
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
