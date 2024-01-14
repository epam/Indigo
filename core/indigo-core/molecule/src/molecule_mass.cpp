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

#include "base_cpp/output.h"

#include "molecule/molecule_mass.h"

#include "molecule/elements.h"
#include "molecule/molecule.h"

using namespace indigo;

IMPL_ERROR(MoleculeMass, "calculation error");

MoleculeMass::MoleculeMass()
{
    relative_atomic_mass_map = nullptr;
}

double MoleculeMass::molecularWeight(Molecule& mol)
{
    std::set<int> selected_atoms;
    mol.getAtomSelection(selected_atoms);

    if (mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) > 0)
    {
        throw Error("Cannot calculate mass for structure with repeating units");
    }

    mol.restoreAromaticHydrogens();

    double molmass = 0;
    int impl_h = 0;
    int elements_count[ELEM_MAX] = {0};

    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        if (mol.isPseudoAtom(v) || mol.isRSite(v) || mol.isTemplateAtom(v))
        {
            if (mass_options.skip_error_on_pseudoatoms)
            {
                continue;
            }
            else
            {
                throw Error("Cannot calculate mass for structure with pseudoatoms, template atoms or RSites");
            }
        }

        if (selected_atoms.size() && selected_atoms.find(v) == selected_atoms.end())
            continue;

        int number = mol.getAtomNumber(v);
        int isotope = mol.getAtomIsotope(v);

        if (isotope == 0)
        {
            auto* value = _relativeAtomicMassByNumber(number);

            if (value == nullptr)
            {
                elements_count[number]++;
            }
            else
            {
                molmass += *value;
            }
        }
        else
        {
            molmass += Element::getRelativeIsotopicMass(number, isotope);
        }

        // Add hydrogens
        impl_h += mol.getImplicitH(v);
    }

    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        if (elements_count[i])
        {
            molmass += Element::getStandardAtomicWeight(i) * (double)elements_count[i];
        }
    }

    molmass += Element::getStandardAtomicWeight(ELEM_H) * impl_h;

    return molmass;
}

static int _isotopesCmp(int i1, int i2, void* context)
{
    int element = *(int*)context;
    double c1, c2;
    Element::getIsotopicComposition(element, i1, c1);
    Element::getIsotopicComposition(element, i2, c2);
    if (c1 < c2)
        return 1;
    else if (c1 > c2)
        return -1;
    return 0;
}

double MoleculeMass::mostAbundantMass(Molecule& mol)
{
    std::set<int> selected_atoms;
    mol.getAtomSelection(selected_atoms);

    if (mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) > 0)
    {
        throw Error("Cannot calculate mass for structure with repeating units");
    }

    mol.restoreAromaticHydrogens();

    double molmass = 0;

    // Count elements without explicit isotope marks
    int elements_counts[ELEM_MAX] = {0};

    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        if (mol.isPseudoAtom(v) || mol.isTemplateAtom(v) || mol.isRSite(v))
        {
            if (mass_options.skip_error_on_pseudoatoms)
            {
                continue;
            }
            else
            {
                throw Error("Cannot calculate mass for structure with pseudoatoms, template atoms or RSites");
            }
        }

        if (selected_atoms.size() && selected_atoms.find(v) == selected_atoms.end())
            continue;

        int number = mol.getAtomNumber(v);
        int isotope = mol.getAtomIsotope(v);
        int impl_h = mol.getImplicitH(v);

        if (isotope == 0)
            elements_counts[number]++;
        else
            molmass += Element::getRelativeIsotopicMass(number, isotope);

        // Add hydrogens
        elements_counts[ELEM_H] += impl_h;
    }

    QS_DEF(Array<int>, isotopes);

    // Compute mass of the most abunant composition
    for (int i = ELEM_MIN; i < ELEM_MAX; i++)
    {
        int count = elements_counts[i];
        if (count == 0)
            continue;

        int count_left = count;
        int min_iso, max_iso;

        // Sort by isotope abundance
        isotopes.clear();
        Element::getMinMaxIsotopeIndex(i, min_iso, max_iso);
        for (int j = min_iso; j <= max_iso; j++)
        {
            double composition;
            if (!Element::getIsotopicComposition(i, j, composition))
                continue;
            if (composition > 0)
                isotopes.push(j);
        }
        isotopes.qsort(_isotopesCmp, (void*)&i);

        for (int k = 0; k < isotopes.size(); k++)
        {
            int j = isotopes[k];

            double composition;
            if (!Element::getIsotopicComposition(i, j, composition))
                continue;

            int such_isotope_count = (int)(composition * count / 100 + 0.5f);

            molmass += Element::getRelativeIsotopicMass(i, j) * such_isotope_count;
            count_left -= such_isotope_count;
            if (count_left == 0)
                break;
        }

        if (count_left != 0)
        {
            // Corrections in case of rounding errors
            int default_iso = Element::getMostAbundantIsotope(i);
            molmass += Element::getRelativeIsotopicMass(i, default_iso) * count_left;
        }
    }

    return molmass;
}

double MoleculeMass::monoisotopicMass(Molecule& mol)
{
    std::set<int> selected_atoms;
    mol.getAtomSelection(selected_atoms);

    if (mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) > 0)
    {
        throw Error("Cannot calculate mass for structure with repeating units");
    }

    mol.restoreAromaticHydrogens();

    double molmass = 0;

    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        if (mol.isPseudoAtom(v) || mol.isTemplateAtom(v) || mol.isRSite(v))
        {
            if (mass_options.skip_error_on_pseudoatoms)
            {
                continue;
            }
            else
            {
                throw Error("Cannot calculate mass for structure with pseudoatoms, template atoms or RSites");
            }
        }

        if (selected_atoms.size() && selected_atoms.find(v) == selected_atoms.end())
            continue;

        int number = mol.getAtomNumber(v);
        int isotope = mol.getAtomIsotope(v);
        int impl_h = mol.getImplicitH(v);

        if (isotope == 0)
            isotope = Element::getMostAbundantIsotope(number);

        molmass += Element::getRelativeIsotopicMass(number, isotope);

        // Add hydrogens
        molmass += Element::getRelativeIsotopicMass(ELEM_H, 1) * impl_h;
    }

    return molmass;
}

int MoleculeMass::nominalMass(Molecule& mol)
{
    if (mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) > 0)
    {
        throw Error("Cannot calculate mass for structure with repeating units");
    }

    mol.restoreAromaticHydrogens();

    int molmass = 0;

    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        if (mol.isPseudoAtom(v) || mol.isTemplateAtom(v) || mol.isRSite(v))
            continue;

        int number = mol.getAtomNumber(v);
        int isotope = mol.getAtomIsotope(v);
        int impl_h = mol.getImplicitH(v);

        if (isotope == 0)
            molmass += Element::getDefaultIsotope(number);
        else
            molmass += isotope;

        // Add hydrogens
        molmass += impl_h;
    }

    return molmass;
}

int MoleculeMass::_cmp(_ElemCounter& ec1, _ElemCounter& ec2, void* /*context*/)
{
    if (ec1.weight == 0)
        return 1;
    if (ec2.weight == 0)
        return -1;

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

    // all elements are compared lexicographically
    return strncmp(Element::toString(ec1.elem), Element::toString(ec2.elem), 3);
}

void MoleculeMass::massComposition(Molecule& mol, Array<char>& str)
{
    std::set<int> selected_atoms;
    mol.getAtomSelection(selected_atoms);
    if (mol.sgroups.getSGroupCount(SGroup::SG_TYPE_SRU) > 0)
    {
        throw Error("Cannot calculate mass for structure with repeating units");
    }

    Array<double> relativeMass;
    int impl_h = 0;
    relativeMass.clear_resize(ELEM_MAX);
    relativeMass.zerofill();

    mol.restoreAromaticHydrogens();

    for (int v = mol.vertexBegin(); v != mol.vertexEnd(); v = mol.vertexNext(v))
    {
        if (mol.isPseudoAtom(v) || mol.isTemplateAtom(v) || mol.isRSite(v))
        {
            if (mass_options.skip_error_on_pseudoatoms)
            {
                continue;
            }
            else
            {
                throw Error("Cannot calculate mass for structure with pseudoatoms, template atoms or RSites");
            }
        }

        if (selected_atoms.size() && selected_atoms.find(v) == selected_atoms.end())
            continue;

        int number = mol.getAtomNumber(v);
        int isotope = mol.getAtomIsotope(v);
        impl_h += mol.getImplicitH(v);

        if (isotope)
        {
            relativeMass[number] += Element::getRelativeIsotopicMass(number, isotope);
        }
        else
        {
            auto* value = _relativeAtomicMassByNumber(number);
            if (value)
            {
                relativeMass[number] += *value;
            }
            else
            {
                relativeMass[number] += Element::getStandardAtomicWeight(number);
            }
        }
    }

    relativeMass[ELEM_H] += Element::getStandardAtomicWeight(ELEM_H) * impl_h;

    double totalWeight = molecularWeight(mol);

    ArrayOutput output(str);
    if (totalWeight)
    {
        QS_DEF(Array<_ElemCounter>, counters);
        int i;

        counters.clear();

        for (i = ELEM_MIN; i < ELEM_MAX; i++)
        {
            _ElemCounter& ec = counters.push();

            ec.elem = i;
            ec.weight = (relativeMass[i] / totalWeight) * 100;
        }

        counters.qsort(_cmp, 0);

        bool first_written = false;

        for (i = 0; i < counters.size(); i++)
        {
            if (counters[i].weight == 0)
                break;

            if (first_written)
                output.printf(" ");

            output.printf(Element::toString(counters[i].elem));
            output.printf(" %.2f", counters[i].weight);
            first_written = true;
        }
    }
    output.writeChar(0);
}

const double* MoleculeMass::_relativeAtomicMassByNumber(int number) const
{
    if (!relative_atomic_mass_map)
    {
        return nullptr;
    }

    const auto it = relative_atomic_mass_map->find(number);
    return it != relative_atomic_mass_map->end() ? &(it->second) : nullptr;
}
