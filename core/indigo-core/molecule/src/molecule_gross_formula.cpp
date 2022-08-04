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

#include <ctype.h>

#include "base_cpp/output.h"
#include "base_cpp/scanner.h"

#include "molecule/base_molecule.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_gross_formula.h"
#include "molecule/query_molecule.h"

using namespace indigo;

int MoleculeGrossFormula::_cmp(_ElemCounter& ec1, _ElemCounter& ec2, void* context)
{
    if (ec1.counter == 0)
        return 1;
    if (ec2.counter == 0)
        return -1;

    if (ec1.elem == ec2.elem)
    {
        if (ec1.isotope == 0)
            return -1;
        if (ec2.isotope == 0)
            return 1;

        return ec1.isotope - ec2.isotope;
    }
    else
    {
        if (ec2.elem == ELEM_H) // move hydrogen to the end
            return -1;
        if (ec1.elem == ELEM_H)
            return 1;

        return ec1.elem - ec2.elem;
    }
}

// comparator implementing the Hill system without carbon:
// <all atoms in alphabetical order>
int MoleculeGrossFormula::_cmp_hill_no_carbon(_ElemCounter& ec1, _ElemCounter& ec2, void* context)
{
    if (ec1.counter == 0)
        return 1;
    if (ec2.counter == 0)
        return -1;
    // all elements are compared lexicographically
    if (ec1.elem != ec2.elem)
        return strncmp(Element::toString(ec1.elem), Element::toString(ec2.elem), 3);
    else
    {
        if (ec1.isotope == 0)
            return -1;
        if (ec2.isotope == 0)
            return 1;

        return ec1.isotope - ec2.isotope;
    }
}

// comparator implementing the Hill system with carbon:
// C H <other atoms in alphabetical order>
int MoleculeGrossFormula::_cmp_hill(_ElemCounter& ec1, _ElemCounter& ec2, void* context)
{
    if (ec1.counter == 0)
        return 1;
    if (ec2.counter == 0)
        return -1;

    if (ec1.elem != ec2.elem)
    {
        // carbon has the highest priority
        if (ec2.elem == ELEM_C)
            return 1;
        if (ec1.elem == ELEM_C)
            return -1;

        // hydrogen has the highest priority after carbon
        if (ec2.elem == ELEM_H)
            return 1;
        if (ec1.elem == ELEM_H)
            return -1;

        // RSites have lowest priority
        if (ec2.elem == ELEM_RSITE)
            return -1;
        if (ec1.elem == ELEM_RSITE)
            return 1;
    }
    return _cmp_hill_no_carbon(ec1, ec2, context);
}

void MoleculeGrossFormula::collect(BaseMolecule& molecule, Array<int>& gross_out)
{
    auto res = collect(molecule);
    auto& gross = *res;
    gross_out.clear_resize(ELEM_RSITE + 1);
    gross_out.zerofill();

    auto& unit = gross[0];
    int number = 0;

    for (auto it = unit.isotopes.begin(); it != unit.isotopes.end(); it++)
    {
        number = it->first & 0xFF;
        if (number < ELEM_RSITE + 1)
            gross_out[number] += it->second;
    }
}

std::unique_ptr<GROSS_UNITS> MoleculeGrossFormula::collect(BaseMolecule& mol, bool add_isotopes)
{
    std::set<int> selected_atoms;
    mol.getAtomSelection(selected_atoms);

    if (!mol.isQueryMolecule())
    {
        mol.asMolecule().restoreAromaticHydrogens();
    }

    std::unique_ptr<GROSS_UNITS> result = std::make_unique<GROSS_UNITS>();
    auto& gross = *result;

    // basic structure and all polymers
    int grossFormulaSize = mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) + 1;
    QS_DEF_RES(ObjArray<Array<int>>, filters, grossFormulaSize);
    QS_DEF_RES(ObjArray<Array<char>>, indices, grossFormulaSize);

    // first element is for old-style gross formula
    indices[0].appendString(" ", true);
    for (int i : mol.vertices())
    {
        filters[0].push(i);
    }

    // then polymer sgroups
    for (int i = 1; i < grossFormulaSize; i++)
    {
        RepeatingUnit* ru = (RepeatingUnit*)&mol.sgroups.getSGroup(i - 1, SGroup::SG_TYPE_SRU);
        filters[i].copy(ru->atoms);
        indices[i].copy(ru->subscript.ptr(), ru->subscript.size() - 1); // Remove '0' symbol at the end
        // Filter polymer atoms
        for (int j = 0; j < filters[i].size(); j++)
        {
            int found_idx = filters[0].find(filters[i][j]);
            if (found_idx > -1)
            {
                filters[0].remove(found_idx);
            }
        }
    }
    // init ObjArray
    gross.resize(grossFormulaSize);

    for (int i = 0; i < grossFormulaSize; i++)
    {
        auto& unit = gross[i];

        unit.multiplier.copy(indices[i]);

        for (int j = 0; j < filters[i].size(); j++)
        {
            if (mol.isPseudoAtom(filters[i][j]) || mol.isTemplateAtom(filters[i][j]))
            {
                continue;
            }

            if (selected_atoms.size() && selected_atoms.find(filters[i][j]) == selected_atoms.end())
                continue;

            int number = mol.getAtomNumber(filters[i][j]);

            int isotope = 0;
            if (add_isotopes)
                isotope = mol.getAtomIsotope(filters[i][j]);

            int key;
            int* val;

            if (number > 0 && isotope > 0)
                key = (isotope << 8) + number;
            else if (number > 0)
                key = number;
            else
                continue;

            auto it = unit.isotopes.find(key);
            val = it != unit.isotopes.end() ? &(it->second) : nullptr;

            if (!val)
                unit.isotopes.emplace(key, 1);
            else
                *val += 1;

            if (!mol.isQueryMolecule() && !mol.isRSite(filters[i][j]))
            {
                int implicit_h = mol.asMolecule().getImplicitH(filters[i][j]);

                if (implicit_h >= 0)
                {
                    key = ELEM_H;
                    auto it = unit.isotopes.find(key);
                    val = it != unit.isotopes.end() ? &(it->second) : nullptr;

                    if (!val)
                        unit.isotopes.emplace(key, implicit_h);
                    else
                        *val += implicit_h;
                }
            }
        }
    }
    return result;
}

void MoleculeGrossFormula::toString(const Array<int>& gross, Array<char>& str, bool add_rsites)
{
    ArrayOutput output(str);
    _toString(gross, output, _cmp, add_rsites);
    output.writeChar(0);
}

void MoleculeGrossFormula::toString(GROSS_UNITS& gross, Array<char>& str, bool add_rsites)
{
    ArrayOutput output(str);

    for (int i = 0; i < gross.size(); i++)
    {
        _toString(gross[i].isotopes, output, _cmp, add_rsites);
    }
    output.writeChar(0);
}

void MoleculeGrossFormula::toString_Hill(GROSS_UNITS& gross, Array<char>& str, bool add_rsites)
{
    ArrayOutput output(str); // clear!

    if (gross.size() == 0)
        return;

    // First base molecule
    auto it = gross[0].isotopes.find(ELEM_C);
    if (it == gross[0].isotopes.end())
        _toString(gross[0].isotopes, output, _cmp_hill_no_carbon, add_rsites);
    else
        _toString(gross[0].isotopes, output, _cmp_hill, add_rsites);

    // Then polymers repeating units
    for (int i = 1; i < gross.size(); i++)
    {
        output.writeChar('(');
        auto it = gross[0].isotopes.find(ELEM_C);
        if (it == gross[0].isotopes.end())
            _toString(gross[i].isotopes, output, _cmp_hill_no_carbon, add_rsites);
        else
            _toString(gross[i].isotopes, output, _cmp_hill, add_rsites);
        output.writeChar(')');
        output.writeArray(gross[i].multiplier);
    }
    output.writeChar(0);
}

void MoleculeGrossFormula::_toString(const Array<int>& gross, ArrayOutput& output, int (*cmp)(_ElemCounter&, _ElemCounter&, void*), bool add_rsites)
{
    QS_DEF(Array<_ElemCounter>, counters);
    int i;

    for (i = 1; i < ELEM_MAX; i++)
    {
        _ElemCounter& ec = counters.push();

        ec.elem = i;
        ec.counter = gross[i];
    }

    counters.qsort(cmp, 0);

    bool first_written = false;

    for (i = 0; i < counters.size(); i++)
    {
        if (counters[i].counter < 1)
            break;

        if (first_written)
            output.printf(" ");

        output.printf(Element::toString(counters[i].elem));
        if (counters[i].counter > 1)
            output.printf("%d", counters[i].counter);
        first_written = true;
    }
    if (add_rsites && gross[ELEM_RSITE] > 0)
    {
        output.writeString(" R#");
        if (gross[ELEM_RSITE] > 1)
        {
            output.printf("%d", gross[ELEM_RSITE]);
        }
    }
}

void MoleculeGrossFormula::_toString(const std::map<int, int>& isotopes, ArrayOutput& output, int (*cmp)(_ElemCounter&, _ElemCounter&, void*), bool add_rsites)
{
    QS_DEF(Array<_ElemCounter>, counters);

    int i;
    int number;
    int isotope;

    for (auto it = isotopes.begin(); it != isotopes.end(); it++)
    {
        if (it->first == ELEM_RSITE)
            continue;

        _ElemCounter& ec = counters.push();

        number = it->first & 0xFF;
        isotope = it->first >> 8;

        ec.elem = number;
        ec.isotope = isotope;
        ec.counter = it->second;
    }

    counters.qsort(cmp, 0);

    bool first_written = false;

    for (i = 0; i < counters.size(); i++)
    {
        if (counters[i].counter < 1)
            break;

        if (first_written)
            output.printf(" ");

        if (counters[i].isotope > 0)
        {
            if ((counters[i].elem == ELEM_H) && (counters[i].isotope == 2))
                output.printf("%s", "D");
            else if ((counters[i].elem == ELEM_H) && (counters[i].isotope == 3))
                output.printf("%s", "T");
            else
            {
                output.printf("%d", counters[i].isotope);
                output.printf(Element::toString(counters[i].elem));
            }
        }
        else
            output.printf(Element::toString(counters[i].elem));

        if (counters[i].counter > 1)
            output.printf("%d", counters[i].counter);

        first_written = true;
    }

    auto it = isotopes.find(ELEM_RSITE);
    const int* val = it != isotopes.end() ? &(it->second) : nullptr;

    if (add_rsites && val)
    {
        output.writeString(" R#");
        if (*val > 1)
        {
            output.printf("%d", *val);
        }
    }
}

int MoleculeGrossFormula::_isotopeCount(BaseMolecule& mol)
{
    std::map<int, int> isotopes;

    int isotope;
    int number;
    int key;

    for (auto i : mol.vertices())
    {
        if (mol.isPseudoAtom(i) || mol.isRSite(i) || mol.isTemplateAtom(i))
            continue;

        number = mol.getAtomNumber(i);
        isotope = mol.getAtomIsotope(i);

        if (isotope > 0 && number > 0)
        {
            key = (isotope << 8) + number;
        }
        else if (number > 0)
            key = number;
        else
            continue;

        auto it = isotopes.find(key);
        int* val = it != isotopes.end() ? &(it->second) : nullptr;

        if (!val)
            isotopes.emplace(key, 1);
        else
            *val += 1;
    }

    return isotopes.size();
}

void MoleculeGrossFormula::fromString(Scanner& scanner, Array<int>& gross)
{
    gross.clear_resize(ELEM_MAX);
    gross.zerofill();

    scanner.skipSpace();
    while (!scanner.isEOF())
    {
        int elem = Element::read(scanner);
        scanner.skipSpace();
        int counter = 1;

        if (isdigit(scanner.lookNext()))
        {
            counter = scanner.readUnsigned();
            scanner.skipSpace();
        }

        gross[elem] += counter;
    }
}

void MoleculeGrossFormula::fromString(const char* str, Array<int>& gross)
{
    BufferScanner scanner(str);

    fromString(scanner, gross);
}

bool MoleculeGrossFormula::leq(const Array<int>& gross1, const Array<int>& gross2)
{
    int i;

    for (i = 1; i < ELEM_MAX; i++)
        if (gross1[i] > gross2[i])
            return false;

    return true;
}

bool MoleculeGrossFormula::geq(const Array<int>& gross1, const Array<int>& gross2)
{
    int i;

    for (i = 1; i < ELEM_MAX; i++)
        if (gross2[i] > 0 && gross1[i] < gross2[i])
            return false;

    return true;
}

bool MoleculeGrossFormula::equal(const Array<int>& gross1, const Array<int>& gross2)
{
    int i;

    for (i = 1; i < ELEM_MAX; i++)
        if (gross2[i] != gross1[i])
            return false;

    return true;
}
