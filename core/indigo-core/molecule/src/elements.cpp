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

#include <cctype>
#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <vector>

#include "base_cpp/array.h"
#include "base_cpp/scanner.h"
#include "molecule/elements.h"

using namespace indigo;

IMPL_ERROR(Element, "element");

const Element& Element::_instance()
{
    static Element instance;
    return instance;
}

Element::Element()
{
    _initAllPeriodic();
    _initAllIsotopes();
    _initAromatic();
}

void Element::_initPeriodic(int element, const char* name, int period, int group)
{
    ElementParameters& parameters = _element_parameters.at(element);

    strncpy(parameters.name, name, 3);
    parameters.group = group;
    parameters.period = period;

    _map[name] = element;
}

int Element::radicalElectrons(int radical)
{
    if (radical == RADICAL_DOUBLET)
        return 1;
    if (radical == RADICAL_SINGLET || radical == RADICAL_TRIPLET)
        return 2;
    return 0;
}

int Element::radicalOrbitals(int radical)
{
    if (radical != 0)
        return 1;
    return 0;
}

void Element::_initAllPeriodic()
{
#define INIT(elem, period, group) _initPeriodic(ELEM_##elem, #elem, period, group)

    INIT(H, 1, 1);
    INIT(He, 1, 8);
    INIT(Li, 2, 1);
    INIT(Be, 2, 2);
    INIT(B, 2, 3);
    INIT(C, 2, 4);
    INIT(N, 2, 5);
    INIT(O, 2, 6);
    INIT(F, 2, 7);
    INIT(Ne, 2, 8);
    INIT(Na, 3, 1);
    INIT(Mg, 3, 2);
    INIT(Al, 3, 3);
    INIT(Si, 3, 4);
    INIT(P, 3, 5);
    INIT(S, 3, 6);
    INIT(Cl, 3, 7);
    INIT(Ar, 3, 8);
    INIT(K, 4, 1);
    INIT(Ca, 4, 2);
    INIT(Sc, 4, 3);
    INIT(Ti, 4, 4);
    INIT(V, 4, 5);
    INIT(Cr, 4, 6);
    INIT(Mn, 4, 7);
    INIT(Fe, 4, 8);
    INIT(Co, 4, 8);
    INIT(Ni, 4, 8);
    INIT(Cu, 4, 1);
    INIT(Zn, 4, 2);
    INIT(Ga, 4, 3);
    INIT(Ge, 4, 4);
    INIT(As, 4, 5);
    INIT(Se, 4, 6);
    INIT(Br, 4, 7);
    INIT(Kr, 4, 8);
    INIT(Rb, 5, 1);
    INIT(Sr, 5, 2);
    INIT(Y, 5, 3);
    INIT(Zr, 5, 4);
    INIT(Nb, 5, 5);
    INIT(Mo, 5, 6);
    INIT(Tc, 5, 7);
    INIT(Ru, 5, 8);
    INIT(Rh, 5, 8);
    INIT(Pd, 5, 8);
    INIT(Ag, 5, 1);
    INIT(Cd, 5, 2);
    INIT(In, 5, 3);
    INIT(Sn, 5, 4);
    INIT(Sb, 5, 5);
    INIT(Te, 5, 6);
    INIT(I, 5, 7);
    INIT(Xe, 5, 8);
    INIT(Cs, 6, 1);
    INIT(Ba, 6, 2);
    INIT(La, 6, 3);
    INIT(Ce, 6, 3);
    INIT(Pr, 6, 3);
    INIT(Nd, 6, 3);
    INIT(Pm, 6, 3);
    INIT(Sm, 6, 3);
    INIT(Eu, 6, 3);
    INIT(Gd, 6, 3);
    INIT(Tb, 6, 3);
    INIT(Dy, 6, 3);
    INIT(Ho, 6, 3);
    INIT(Er, 6, 3);
    INIT(Tm, 6, 3);
    INIT(Yb, 6, 3);
    INIT(Lu, 6, 3);
    INIT(Hf, 6, 4);
    INIT(Ta, 6, 5);
    INIT(W, 6, 6);
    INIT(Re, 6, 7);
    INIT(Os, 6, 8);
    INIT(Ir, 6, 8);
    INIT(Pt, 6, 8);
    INIT(Au, 6, 1);
    INIT(Hg, 6, 2);
    INIT(Tl, 6, 3);
    INIT(Pb, 6, 4);
    INIT(Bi, 6, 5);
    INIT(Po, 6, 6);
    INIT(At, 6, 7);
    INIT(Rn, 6, 8);
    INIT(Fr, 7, 1);
    INIT(Ra, 7, 2);
    INIT(Ac, 7, 3);
    INIT(Th, 7, 3);
    INIT(Pa, 7, 3);
    INIT(U, 7, 3);
    INIT(Np, 7, 3);
    INIT(Pu, 7, 3);
    INIT(Am, 7, 3);
    INIT(Cm, 7, 3);
    INIT(Bk, 7, 3);
    INIT(Cf, 7, 3);
    INIT(Es, 7, 3);
    INIT(Fm, 7, 3);
    INIT(Md, 7, 3);
    INIT(No, 7, 3);
    INIT(Lr, 7, 3);
    INIT(Rf, 7, 3);
    INIT(Db, 7, 3);
    INIT(Sg, 7, 3);
    INIT(Bh, 7, 3);
    INIT(Hs, 7, 3);
    INIT(Mt, 7, 3);
    INIT(Ds, 7, 3);
    INIT(Rg, 7, 3);
    INIT(Cn, 7, 3);
    INIT(Nh, 7, 3);
    INIT(Fl, 7, 4);
    INIT(Mc, 7, 5);
    INIT(Lv, 7, 6);
    INIT(Ts, 7, 7);
    INIT(Og, 7, 8);
#undef INIT
}

int Element::fromString(const char* name)
{
    const auto& map = _instance()._map;
    if (map.count(name) > 0)
    {
        return map.at(name);
    }
    throw Error("fromString(): element %s not supported", name);
}

int Element::fromString2(const char* name)
{
    const auto& map = _instance()._map;
    if (map.count(name) > 0)
    {
        return map.at(name);
    }
    return -1;
}

int Element::fromChar(char c)
{
    char str[2] = {c, 0};

    return fromString(str);
}

int Element::fromTwoChars(char c1, char c2)
{
    char str[3] = {c1, c2, 0};

    return fromString(str);
}

int Element::fromTwoChars2(char c1, char c2)
{
    char str[3] = {c1, c2, 0};

    return fromString2(str);
}

int Element::fromTwoChars2(char c1, int c2)
{
    if (c2 < 0)
        return -1;
    return fromTwoChars2(c1, static_cast<char>(c2));
}

bool Element::isHalogen(int element)
{
    return element == ELEM_F || element == ELEM_Cl || element == ELEM_Br || element == ELEM_I || element == ELEM_At;
}

const char* Element::toString(int element)
{
    if (element < 0 || element > ELEM_MAX)
        throw Error("bad element number: %d", element);

    return _instance()._element_parameters.at(element).name;
}

const char* Element::toString(int element, int isotope)
{
    if (element == ELEM_H)
    {
        if (isotope == DEUTERIUM)
            return "D";
        if (isotope == TRITIUM)
            return "T";
    }
    return toString(element);
}

int Element::calcValenceOfAromaticAtom(int elem, int charge, int n_arom, int min_conn)
{
    if (elem == ELEM_C)
        return 4;
    if (elem == ELEM_N)
        return (charge == 1 ? 4 : 3);
    if (elem == ELEM_O)
        return (charge >= 1 ? 3 : 2);
    if (elem == ELEM_S && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                // There are no cases of implicit hydrogens in that condition
                // (PubChem search [sHD2] gives no hits)
                return 2;      // ergo, valence is 2
            if (min_conn == 3) // one single external bond
                // there can be a radical (see CID 11972190),
                // or an implicit hydrogen (see CID 20611310)
                return 4;      // either way, the valence is 4
            if (min_conn == 4) // two single or one double external bond
                // PubChem has no examples of 6-valent aromatic sulphur
                // (searching [sv6] gives no hits)
                return 4; // ergo, valence is 4
            if (min_conn > 4)
                // OK, suppose we have an case of 6-valent aromatic sulphur here
                return 6;
        }
        else if (n_arom == 3)
        {
            if (min_conn <= 4) // no external bonds or one single external bond
                // For one external bond, see CID 10091381
                // For no external bonds, see CID 20756501, although aromaticity
                // there is questionable. Anyway, no hydrogens are possible.
                return 4;
            else
                // 6-valent aromatic sulphur?
                return 6;
        }
        else if (n_arom == 4)
        {
            if (min_conn == 4)
                // Happened only on CID 10882272 and CID 24829837
                return 4; // Valence = 4 in both structures
            else
                // 6-valent aromatic sulphur?
                return 6;
        }
    }
    else if (elem == ELEM_S && charge == 1)
    {
        if (n_arom == 2)
        {
            if (min_conn == 2) // common case: "=[S+]-" in an aromatic ring
                return 3;
            if (min_conn <= 4) // CID 9922592
                return 5;
        }
    }
    else if (elem == ELEM_P && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                // implicit hydrogen (CID 164575) or radical (CID 10568539) is present
                return 3;      // in any case, the valence is 3
            if (min_conn == 3) // one single external bond
                return 3;
            if (min_conn == 4) // two single on one double external bond
                // two single: CID 140786, CID 341499
                // one double: CID 17776485, CID 20207916
                return 5; // valence is 5 in any case
        }
        if (n_arom == 3) // three aromatic bonds
        {
            if (min_conn == 3) // no external bonds
                return 3;      // CID 15973306; no known examples with valence 5
            if (min_conn == 5) // two single or one double external bond
                return 5;      // the only known example is CID 10887416
        }
        if (n_arom == 4) // four aromatic bonds?
        {
            if (min_conn == 4) // no external bonds
                return 5;      // the only known example is CID 10887416,
                               // yet the aromaticity of the smaller ring is questionable
        }
    }
    else if (elem == ELEM_P && charge == 1)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 3) // one single external bond
                return 4;      // common case: "=[P+]([*])-" in an aromatic ring
        }
    }
    else if (elem == ELEM_P && charge == -1)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 2;      // CID 10932222
        }
    }
    else if (elem == ELEM_Se && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 2;      // common case
            if (min_conn == 3) // one external bond
                return 4;      // CID 10262587
            if (min_conn == 4)
                // CID 21204858, two single external bonds
                // CID 14984497, one double aromatic bond
                return 4;
        }
    }
    else if (elem == ELEM_Se && charge == 1)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 3;      // CID 10872228
            if (min_conn == 3) // one external bond
                return 3;      // CID 11115581
        }
    }
    else if (elem == ELEM_As && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 3;      // CID 136132
            if (min_conn == 3) // one external bond
                return 3;      // CID 237687
                               // no other cases known from PubChem
        }
    }
    else if (elem == ELEM_Te && charge == 0)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 2) // no external bonds
                return 3;      // CID 136053
            if (min_conn == 4)
                // CID 3088544, two single external bonds
                // CID 11457076, one double external bonds
                return 4;
        }
        else if (n_arom == 4)
        {
            if (min_conn == 4)
                // CID 11070061, four aromatic external bonds
                return 4;
        }
        // no other cases known from PubChem
    }
    else if (elem == ELEM_Te && charge == 1)
    {
        if (n_arom == 2) // two aromatic bonds
        {
            if (min_conn == 3) // one external bond
                return 3;      // CID 20802344
        }
        // no other cases known from PubChem
    }
    else if (elem == ELEM_B)
    {
        if (n_arom == 2)
        {
            if (min_conn == 3) // one external bond
                return 3;      // CID 574072
        }
    }
    else if (elem == ELEM_Si)
    {
        if (n_arom == 2)
        {
            if (min_conn == 3) // one external bond
                return 4;      // CID 18943170
        }
    }

    return -1;
}

bool Element::calcValence(int elem, int charge, int radical, int conn, int& valence, int& hyd, bool to_throw)
{
    int groupno = Element::group(elem);
    int rad = radicalElectrons(radical);

    valence = conn;
    hyd = 0;

    if (groupno == 1)
    {
        if (elem == ELEM_Li || elem == ELEM_Na || elem == ELEM_K || elem == ELEM_Rb || elem == ELEM_Cs || elem == ELEM_Fr)
        {
            valence = 1;
            hyd = 1 - rad - conn - abs(charge);
        }
        if (elem == ELEM_H)
        {
            valence = 1;
            if (charge == 1 && conn == 0)
                hyd = 0;
            else if (charge == -1 && conn == 0)
                hyd = 0;
            else if (charge == 0 && conn == 1)
                hyd = 0;
            else if (charge == 0 && conn == 0)
                hyd = 1; // elemental hydrogen, hmm... well, OK -- behaviour changed
                         // Allow implicit H for H, so single H is considered as molecule H2 now
                         // in accordance with Biovia Draw model
            else
                hyd = -1;
        }
    }
    else if (groupno == 2)
    {
        if (elem == ELEM_Be || elem == ELEM_Mg || elem == ELEM_Ca || elem == ELEM_Sr || elem == ELEM_Ba || elem == ELEM_Ra)
        {
            valence = 2;
            if (conn != 0)
            {
                if (rad > 0 || abs(charge) > 0)
                    hyd = -1;
                else
                    hyd = 2 - conn;
            }
            else if (rad > 0 || abs(charge) > 0)
            {
                hyd = 2 - rad - abs(charge);
            }
            else
            {
                hyd = 0;
            }

            if (hyd != 0)
                hyd = -1;
        }
    }
    else if (groupno == 3)
    {
        if (elem == ELEM_B || elem == ELEM_Al || elem == ELEM_Ga || elem == ELEM_In)
        {
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
                    hyd = -1;
            }
            else
            {
                valence = 3;
                hyd = 3 - rad - conn - abs(charge);
            }
        }
        else if (elem == ELEM_Tl)
        {
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
            { // ISIS Draw and Marvin allow this
                valence = 6;
                hyd = 0;
            }
            else
            {
                if (rad + conn + abs(charge) <= 1)
                {
                    valence = 1;
                    hyd = 1 - rad - conn - abs(charge);
                }
                else
                {
                    valence = 3;
                    hyd = 3 - rad - conn - abs(charge);
                }
            }
        }
    }
    else if (groupno == 4)
    {
        if (elem == ELEM_C)
        {
            valence = 4;
            hyd = 4 - rad - conn - abs(charge);
        }
        else if (elem == ELEM_Si || elem == ELEM_Ge || elem == ELEM_Sn || elem == ELEM_Pb)
        {
            if (charge == -2 && conn == 6 && rad == 0)
            {
                // Zinc fluorosilicate, hexafluorogermanium
                valence = 6;
                hyd = 0;
            }
            else if (charge == -1 && conn + rad == 5)
            {
                // with radical:    [Ge-]: CID 18503269
                // without radical: [Si-]: CID 358631
                //                  [Ge-]: CID 19891516
                valence = 5;
                hyd = 0;
            }
            else if (charge == -1 && conn + rad == 4 && elem == ELEM_Si)
            {
                valence = 5; // CID 438107
                hyd = 1;
            }
            else if ((elem == ELEM_Sn || elem == ELEM_Pb) && conn + rad + abs(charge) <= 2)
            {
                // [SnH2]: CID 23962
                // [PbH2]: CID 23927
                valence = 2;
                hyd = 2 - rad - conn - abs(charge);
            }
            else
            {
                // 4-valent Pb with H: CID 24003
                // 4-valent Sn with H: CID 5948
                // 4-valent Ge with H2: CID 66239
                // [GeH4]: CID 23984
                valence = 4;
                hyd = 4 - rad - conn - abs(charge);
            }
        }
    }
    else if (groupno == 5)
    {
        if (elem == ELEM_N || elem == ELEM_P)
        {
            if (charge == 1)
            {
                valence = 4;
                hyd = 4 - rad - conn;
            }
            else if (charge == 2)
            {
                valence = 3;
                hyd = 3 - rad - conn;
            }
            else if (charge == -1 && elem == ELEM_P)
            {
                if (rad + conn <= 2) // phosphanide
                {
                    valence = 2;
                    hyd = 2 - rad - conn;
                }
                else if (rad + conn == 3) // no known examples with a hydrogen
                    hyd = -1;
                else if (rad + conn == 4)
                {
                    valence = 4;
                    hyd = 0;
                }
                else if (rad + conn <= 6)
                {
                    // w/ hydrogen: CID 3084356, CID 2784547
                    // w/o hydrogen: hexachlorophosphate
                    valence = 6;
                    hyd = 6 - rad - conn;
                }
            }
            else
            {
                if (elem == ELEM_N || rad + conn + abs(charge) <= 3)
                {
                    valence = 3;
                    hyd = 3 - rad - conn - abs(charge);
                }
                else // ELEM_P && rad + conn + abs(charge) > 3
                {
                    valence = 5;
                    hyd = 5 - rad - conn - abs(charge);
                }
            }
        }
        else if (elem == ELEM_Bi || elem == ELEM_Sb || elem == ELEM_As)
        {
            if (charge == -1 && rad + conn == 6)
            {
                valence = 6;
                hyd = 0;
            }
            else if (charge == 1)
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
            }
            else if (charge == 2)
            {
                valence = 3;
                hyd = 3 - rad - conn;
            }
            else if (charge == -2 && rad + conn == 5)
            {
                // Bi: CID 45158489
                valence = 5;
                hyd = 0;
            }
            else
            {
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
            }
        }
    }
    else if (groupno == 6)
    {
        if (elem == ELEM_O)
        {
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
        }
        else if (elem == ELEM_S || elem == ELEM_Se || elem == ELEM_Po)
        {
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
            }
            else if (charge == -1)
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
                // no real examples for the other two cases, just following ISIS/Draw logic
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
            }
            else
            {
                if (conn + rad + abs(charge) <= 2)
                {
                    valence = 2;
                    hyd = 2 - rad - conn - abs(charge);
                }
                else if (conn + rad + abs(charge) <= 4)
                // See examples in PubChem
                // [S] : CID 16684216
                // [Se]: CID 5242252
                // [Po]: no example, just following ISIS/Draw logic here
                {
                    valence = 4;
                    hyd = 4 - rad - conn - abs(charge);
                }
                else
                // See examples in PubChem
                // [S] : CID 46937044
                // [Se]: CID 59786
                // [Po]: no example, just following ISIS/Draw logic here
                {
                    valence = 6;
                    hyd = 6 - rad - conn - abs(charge);
                }
            }
        }
        else if (elem == ELEM_Te)
        {
            if (charge == -1)
            {
                if (rad + conn == 7) // CID 4191414
                {
                    valence = 7;
                    hyd = 0;
                }
                else if (rad + conn == 5)
                { // no example, but both Marvin and ISIS are OK with this configuration
                    valence = 5;
                    hyd = 0;
                }
                else
                {
                    valence = 1;
                    hyd = 1 - rad - conn;
                }
            }
            else if (charge == 1)
            {
                valence = 3;
                hyd = 3 - rad - conn;
                // no known cases of 5-connected [Te+]
            }
            else if (charge == 2)
            {
                if (conn + rad == 4)
                {
                    valence = conn + rad;
                    hyd = 0;
                }
                else // ISIS Draw logic
                {
                    hyd = 2 - conn - rad;
                    valence = 2;
                }
            }
            else if (charge == 0)
            {
                if (conn + rad <= 2)
                {
                    hyd = 2 - conn - rad;
                    valence = 2;
                }
                else if (conn + rad <= 4)
                {
                    hyd = 4 - conn - rad; // with hydrogen: CID 11968228
                    valence = 4;
                }
                else
                {
                    hyd = 6 - conn - rad; // with hydrogen: CID 5231555, CID 6418860
                    valence = 6;
                }
            }
        }
    }
    else if (groupno == 7)
    {
        if (elem == ELEM_F)
        {
            valence = 1;
            hyd = 1 - rad - conn - abs(charge);
        }
        else if (elem == ELEM_Cl || elem == ELEM_Br || elem == ELEM_I || elem == ELEM_At)
        {
            if (charge == 1)
            {
                if (conn <= 2)
                {
                    valence = 2;
                    hyd = 2 - rad - conn;
                }
                else if (conn == 3 || conn == 5 || conn >= 7)
                    hyd = -1;
            }
            else if (charge == 0)
            {
                if (conn <= 1)
                {
                    valence = 1;
                    hyd = 1 - rad - conn;
                }
                // While the halogens can have valence 3, they can not have
                // hydrogens in that case.
                else if (conn == 2 || conn == 4 || conn == 6)
                {
                    if (rad == 1)
                    {
                        valence = conn;
                        hyd = 0;
                    }
                    else
                        hyd = -1; // will throw an error in the end
                }
                else if (conn > 7)
                    hyd = -1; // will throw an error in the end
            }
        }
    }
    else if (groupno == 8)
    {
        if (elem == ELEM_He || elem == ELEM_Ne || elem == ELEM_Ar || elem == ELEM_Kr || elem == ELEM_Xe || elem == ELEM_Rn || elem == ELEM_Og)
        {
            valence = 0;
            hyd = 0 - rad - conn - abs(charge);
            if (hyd > 0)
                hyd = 0;
        }
    }

    if (hyd < 0)
    {
        if (to_throw)
            throw Error("bad valence on %s having %d drawn bonds, charge %d, and %d radical electrons", toString(elem), conn, charge, rad);
        valence = conn;
        hyd = 0;
        return false;
    }
    return true;
}

int Element::calcValenceMinusHyd(int elem, int charge, int radical, int conn)
{
    int groupno = Element::group(elem);
    int rad = radicalElectrons(radical);

    if (groupno == 3)
    {
        if (elem == ELEM_B || elem == ELEM_Al || elem == ELEM_Ga || elem == ELEM_In)
        {
            if (charge == -1)
                if (rad + conn <= 4)
                    return rad + conn;
        }
    }
    else if (groupno == 5)
    {
        if (elem == ELEM_N || elem == ELEM_P)
        {
            if (charge == 1)
                return rad + conn;
            if (charge == 2)
                return rad + conn;
        }
        else if (elem == ELEM_Sb || elem == ELEM_Bi || elem == ELEM_As)
        {
            if (charge == 1)
                return rad + conn;
            else if (charge == 2)
                return rad + conn;
        }
    }
    else if (groupno == 6)
    {
        if (elem == ELEM_O)
        {
            if (charge >= 1)
                return rad + conn;
        }
        else if (elem == ELEM_S || elem == ELEM_Se || elem == ELEM_Po)
        {
            if (charge == 1 || charge == -1)
                return rad + conn;
        }
    }
    else if (groupno == 7)
    {
        if (elem == ELEM_Cl || elem == ELEM_Br || elem == ELEM_I || elem == ELEM_At)
        {
            if (charge == 1)
                return rad + conn;
        }
    }

    return rad + conn + abs(charge);
}

int Element::group(int elem)
{
    return _instance()._element_parameters.at(elem).group;
}

int Element::period(int elem)
{
    return _instance()._element_parameters.at(elem).period;
}

int Element::read(Scanner& scanner)
{
    char str[3] = {0, 0, 0};

    str[0] = scanner.readChar();

    if (islower(scanner.lookNext()))
        str[1] = scanner.readChar();

    return fromString(str);
}

void Element::_setStandardAtomicWeightIndex(int element, int index)
{
    ElementParameters& p = _element_parameters.at(element);
    p.natural_isotope_index = index;
}

void Element::_addElementIsotope(int element, int isotope, double mass, double isotopic_composition)
{
    auto key = IsotopeKey{element, isotope};
    auto value = IsotopeValue{mass, isotopic_composition};
    _isotope_parameters_map[key] = value;
}

void Element::_initAllIsotopes()
{
#define ADD _addElementIsotope
#define SET _setStandardAtomicWeightIndex
#define NATURAL IsotopeKey::NATURAL

#include "elements_isotopes.inc"

#undef ADD
#undef SET
#undef NATURAL

    _initDefaultIsotopes();
}

double Element::getStandardAtomicWeight(int element)
{
    return _instance()._getStandardAtomicWeight(element);
}

int Element::getDefaultIsotope(int element)
{
    const ElementParameters& p = _instance()._element_parameters.at(element);
    return p.default_isotope;
}

int Element::getMostAbundantIsotope(int element)
{
    const ElementParameters& p = _instance()._element_parameters.at(element);
    return p.most_abundant_isotope;
}

bool Element::getIsotopicComposition(int element, int isotope, double& res)
{
    const auto key = IsotopeKey{element, isotope};
    if (_instance()._isotope_parameters_map.count(key))
    {
        res = _instance()._isotope_parameters_map.at(key).isotopic_composition;
        return true;
    }
    return false;
}

void Element::getMinMaxIsotopeIndex(int element, int& min, int& max)
{
    const ElementParameters& p = _instance()._element_parameters.at(element);
    min = p.min_isotope_index;
    max = p.max_isotope_index;
}

double Element::getRelativeIsotopicMass(int element, int isotope)
{
    return _instance()._getRelativeIsotopicMass(element, isotope);
}

void Element::_initDefaultIsotopes()
{
    std::vector<IsotopeKey> def_isotope_index;
    def_isotope_index.resize(_element_parameters.size());

    std::vector<double> most_abundant_isotope_fraction;
    most_abundant_isotope_fraction.resize(_element_parameters.size());

    for (unsigned int i = ELEM_MIN; i < _element_parameters.size(); i++)
    {
        _element_parameters.at(i).default_isotope = IsotopeKey::NATURAL;
        _element_parameters.at(i).most_abundant_isotope = IsotopeKey::NATURAL;
        _element_parameters.at(i).min_isotope_index = 10000;
        _element_parameters.at(i).max_isotope_index = 0;
    }

    for (auto& item : _isotope_parameters_map)
    {
        const auto& key = item.first;
        auto& value = item.second;

        if (key.isotope == IsotopeKey::NATURAL)
        {
            continue;
        }
        double atomic_weight = _getStandardAtomicWeight(key.element);

        double diff_best = 1e6;
        if (def_isotope_index[key.element].isotope != IsotopeKey::NATURAL)
        {
            auto best_iso = def_isotope_index[key.element];
            if (_isotope_parameters_map.count(best_iso) > 0)
            {
                const auto& best = _isotope_parameters_map.at(best_iso);
                diff_best = fabs(best.mass - atomic_weight);
            }
        }
        double diff_cur = fabs(value.mass - atomic_weight);

        if (diff_best > diff_cur)
        {
            def_isotope_index[key.element] = key;
            _element_parameters.at(key.element).default_isotope = key.isotope;
            diff_best = diff_cur;
        }

        int& min_iso = _element_parameters.at(key.element).min_isotope_index;
        int& max_iso = _element_parameters.at(key.element).max_isotope_index;
        if (min_iso > key.isotope)
        {
            min_iso = key.isotope;
        }
        if (max_iso < key.isotope)
        {
            max_iso = key.isotope;
        }
        double most_abundance = 1e6;
        if (_element_parameters.at(key.element).default_isotope != IsotopeKey::NATURAL)
        {
            most_abundance = value.isotopic_composition;
        }

        if (value.isotopic_composition > most_abundant_isotope_fraction[key.element])
        {
            most_abundant_isotope_fraction[key.element] = value.isotopic_composition;
            _element_parameters.at(key.element).most_abundant_isotope = key.isotope;
        }
    }

    for (unsigned int i = ELEM_MIN; i < _element_parameters.size(); i++)
    {
        ElementParameters& element = _element_parameters.at(i);

        if (element.natural_isotope_index != IsotopeKey::NATURAL)
        {
            element.default_isotope = element.natural_isotope_index;
        }
        if (element.most_abundant_isotope == IsotopeKey::NATURAL)
        {
            element.most_abundant_isotope = element.default_isotope;
        }
    }

    // Post-condition
    for (unsigned int i = ELEM_MIN; i < _element_parameters.size(); i++)
        if (_element_parameters.at(i).default_isotope == IsotopeKey::NATURAL)
            // usually you can't catch this as it's being thrown before main()
            throw Error("default isotope is not set on element #%d", i);
}

int Element::orbitals(int elem, bool use_d_orbital)
{
    int group = Element::group(elem);
    int period = Element::period(elem);
    switch (group)
    {
    case 1:
        return 1;
    case 2:
        return 2;
    default:
        if (use_d_orbital && period > 2 && group >= 4)
            return 9;
        else
            return 4;
    }
}

int Element::electrons(int elem, int charge)
{
    return Element::group(elem) - charge;
}

int Element::getMaximumConnectivity(int elem, int charge, int radical, bool use_d_orbital)
{
    int rad_electrons = radicalElectrons(radical);
    int electrons = Element::electrons(elem, charge) - rad_electrons;
    int rad_orbitals = radicalOrbitals(radical);
    int vacant_orbitals = Element::orbitals(elem, use_d_orbital) - rad_orbitals;
    if (electrons <= vacant_orbitals)
        return electrons;
    else
        return 2 * vacant_orbitals - electrons;
}

bool Element::IsotopeKey::operator<(const IsotopeKey& right) const
{
    if (element < right.element)
        return true;
    if (element > right.element)
        return false;
    if (isotope < right.isotope)
        return true;
    if (isotope > right.isotope)
        return false;
    return false;
}

bool Element::canBeAromatic(int element)
{
    return _instance()._element_parameters.at(element).can_be_aromatic;
}

void Element::_initAromatic()
{
    int i;

    for (i = ELEM_B; i <= ELEM_F; i++)
        _element_parameters.at(i).can_be_aromatic = true;
    for (i = ELEM_Al; i <= ELEM_Cl; i++)
        _element_parameters.at(i).can_be_aromatic = true;
    for (i = ELEM_Ga; i <= ELEM_Br; i++)
        _element_parameters.at(i).can_be_aromatic = true;
    for (i = ELEM_In; i <= ELEM_I; i++)
        _element_parameters.at(i).can_be_aromatic = true;
    for (i = ELEM_Tl; i <= ELEM_Bi; i++)
        _element_parameters.at(i).can_be_aromatic = true;
}

double Element::_getStandardAtomicWeight(int element) const
{
    const ElementParameters& p = _element_parameters.at(element);
    return _getRelativeIsotopicMass(element, p.natural_isotope_index);
}

double Element::_getRelativeIsotopicMass(int element, int isotope) const
{
    const auto key = IsotopeKey{element, isotope};
    if (_isotope_parameters_map.count(key))
    {
        return _isotope_parameters_map.at(key).mass;
    }
    throw Error("getRelativeIsotopicMass: isotope (%s, %d) not found", toString(element), isotope);
}

int Element::getNumOuterElectrons(int element)
{
    // clang-format off
    constexpr std::array<int, 59> outerElements{
        0, // Pseudo-element
        1, // H
        2, // H3
        1, // Li
        2, // Be
        3, // B
        4, // C
        5, // N
        6, // O
        7, // F
        8, // Ne
        1, // Na
        2, // Mg
        3, // Al
        4, // Si
        5, // P
        6, // S
        7, // Cl
        8, // Ar
        1, // K
        2, // Ca
        3, // Sc
        4, // Ti
        5, // V
        6, // Cr
        7, // Mn
        8, // Fe
        9, // Co
        10, // Ni
        1,  // Cu
        2, // Zn
        3, // Ga
        4, // Ge
        5, // As
        6, // Se
        7, // Br
        8, // Kr
        1, // Rb
        2, // Sr
        3, // Y
        4, // Zr
        5, // Nb
        6, // Mo
        7, // Tc
        8, // Ru
        9, // Rh
        10, // Pd
        1,  // Ag,
        2, // Cd
        3, // In
        4, // Sn
        5, // Sb
        6, // Te
        7, // I
        8, // Xe
        1, // Cs
        2, // Ba
        3, // La
        4 // Ce
    };
    // clang-format on
    if (element > static_cast<int>(outerElements.size()))
    {
        throw Error("outerElements are currently filled only for elements up to lantanoids");
    }
    return outerElements[element];
}
