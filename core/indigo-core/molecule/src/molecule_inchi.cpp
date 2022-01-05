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

#include "molecule/molecule_inchi.h"

#include "base_cpp/output.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_automorphism_search.h"
#include "molecule/molecule_inchi_utils.h"

using namespace indigo;

using namespace MoleculeInChILayers;

IMPL_ERROR(MoleculeInChI, "InChI canonicalizer");

CP_DEF(MoleculeInChI);

MoleculeInChI::MoleculeInChI(Output& output) : _output(output), CP_INIT, TL_CP_GET(_components), TL_CP_GET(_component_indices)
{
    prefix = "Indigo=1.1";
}

void MoleculeInChI::outputInChI(Molecule& mol)
{
    _output.printf(prefix);

    if (mol.vertexCount() < 1)
        return;

    // Decompose molecule into components
    _components.clear();
    _components.reserve(mol.countComponents());

    QS_DEF(Molecule, component);
    for (int i = 0; i < mol.countComponents(); i++)
    {
        MoleculeInChICompoment& comp = _components.push();
        Filter filt(mol.getDecomposition().ptr(), Filter::EQ, i);

        component.makeSubmolecule(mol, filt, 0, 0);
        _normalizeMolecule(component);

        comp.construct(component);
    }

    // Sort components
    _component_indices.clear_resize(_components.size());
    for (int i = 0; i < _components.size(); i++)
        _component_indices[i] = i;

    _component_indices.qsort(_cmpComponents, this);

    // Print final InChI string
    _printInChI();
}

// Function that call print function in specified layer for component
template <typename Layer>
class MoleculeInChI::_ComponentLayerPrintFunction : public _PrintLayerFuncBase
{
public:
    _ComponentLayerPrintFunction(Layer MoleculeInChICompoment::*layer, void (Layer::*print)(Array<char>&)) : _layer(layer), _print(print)
    {
    }

    void operator()(MoleculeInChICompoment& component, Array<char>& output)
    {
        ((component.*_layer).*_print)(output);
    }

private:
    Layer MoleculeInChICompoment::*_layer;
    void (Layer::*_print)(Array<char>&);
};

void MoleculeInChI::_printInChI()
{
    // Print formula
    _ComponentLayerPrintFunction<MainLayerFormula> print_formula(&MoleculeInChICompoment::main_layer_formula, &MainLayerFormula::printFormula);

    _printInChILayer(print_formula, ".", "", "/");

    // Print connections table
    _ComponentLayerPrintFunction<MainLayerConnections> print_connections(&MoleculeInChICompoment::main_layer_connections,
                                                                         &MainLayerConnections::printConnectionTable);

    _printInChILayer(print_connections, ";", "*", "/c");

    // Print hydrogens
    _ComponentLayerPrintFunction<HydrogensLayer> print_hydrogens(&MoleculeInChICompoment::hydrogens_layer, &HydrogensLayer::print);

    _printInChILayer(print_hydrogens, ";", "*", "/h");

    // Print cis-trans bonds
    _ComponentLayerPrintFunction<CisTransStereochemistryLayer> print_cis_trans(&MoleculeInChICompoment::cistrans_stereochemistry_layer,
                                                                               &CisTransStereochemistryLayer::print);

    _printInChILayer(print_cis_trans, ";", "*", "/b");

    // Print stereocenters
    _ComponentLayerPrintFunction<TetrahedralStereochemistryLayer> print_stereocenters(&MoleculeInChICompoment::tetra_stereochemistry_layer,
                                                                                      &TetrahedralStereochemistryLayer::print);

    bool has_stereo = _printInChILayer(print_stereocenters, ";", "*", "/t");

    if (has_stereo)
    {
        _ComponentLayerPrintFunction<TetrahedralStereochemistryLayer> print_stereocenters_enan(&MoleculeInChICompoment::tetra_stereochemistry_layer,
                                                                                               &TetrahedralStereochemistryLayer::printEnantiomers);

        _printInChILayer(print_stereocenters_enan, "", 0, "/m");

        _output.printf("/s1");
    }
}

bool MoleculeInChI::_printInChILayer(_PrintLayerFuncBase& print_func, const char* delim, const char* multiplier, const char* layer_prefix)
{
    QS_DEF(Array<char>, layer_string);

    ArrayOutput output(layer_string);

    QS_DEF(Array<char>, previous_component);
    previous_component.clear();
    QS_DEF(Array<char>, current_component);

    int index = -1;
    int same_components_count = 1;
    bool layer_contain_data = false;

    previous_component.clear();
    previous_component.push(0);
    current_component.clear();
    current_component.push(0);

    do
    {
        index++;

        bool is_last = (index == _components.size());
        if (!is_last)
        {
            int comp_index = _component_indices[index];

            // Call corresponding printing method
            print_func(_components[comp_index], current_component);
            current_component.push(0);

            if (index == 0)
            {
                previous_component.swap(current_component);
                continue;
            }
        }

        // Compare current string and previous
        bool equal = (strcmp(current_component.ptr(), previous_component.ptr()) == 0);
        bool has_value = (strlen(current_component.ptr()) != 0);
        if (!is_last && equal && has_value)
            same_components_count++;
        else
        {
            // Print previous component
            if (output.tell() != 0)
                output.printf("%s", delim);

            if (same_components_count > 1 && multiplier != 0)
                output.printf("%d%s", same_components_count, multiplier);

            output.printf("%s", previous_component.ptr());
            if (same_components_count > 1 && multiplier == 0)
            {
                for (int i = 1; i < same_components_count; i++)
                    output.printf("%s", previous_component.ptr());
            }

            if (previous_component.size() != 0)
                layer_contain_data = true;

            previous_component.swap(current_component);
            same_components_count = 1;
        };

    } while (index < _components.size());

    output.flush();

    if (!layer_contain_data)
        layer_string.clear();
    layer_string.push(0);

    if (layer_string.size() != 0 && layer_string[0] != 0)
    {
        _output.printf(layer_prefix);
        _output.writeString(layer_string.ptr());
        return true;
    }

    return false;
}

int MoleculeInChI::_cmpComponents(int& index1, int& index2, void* context)
{
    MoleculeInChI* self = (MoleculeInChI*)context;
    MoleculeInChICompoment& comp1 = self->_components[index1];
    MoleculeInChICompoment& comp2 = self->_components[index2];

    int ret = 0;
    ret = MainLayerFormula::compareComponentsAtomsCountNoH(comp1.main_layer_formula, comp2.main_layer_formula);
    if (ret != 0)
        return ret;

    ret = MainLayerConnections::compareComponentsConnectionTables(comp1.main_layer_connections, comp2.main_layer_connections);
    if (ret != 0)
        return ret;

    ret = HydrogensLayer::compareComponentsHydrogens(comp1.hydrogens_layer, comp2.hydrogens_layer);
    if (ret != 0)
        return ret;

    ret = CisTransStereochemistryLayer::compareComponents(comp1.cistrans_stereochemistry_layer, comp2.cistrans_stereochemistry_layer);
    if (ret != 0)
        return ret;

    ret = TetrahedralStereochemistryLayer::compareComponents(comp1.tetra_stereochemistry_layer, comp2.tetra_stereochemistry_layer);
    if (ret != 0)
        return ret;

    ret = TetrahedralStereochemistryLayer::compareComponentsEnantiomers(comp1.tetra_stereochemistry_layer, comp2.tetra_stereochemistry_layer);
    if (ret != 0)
        return ret;

    return 0;
}

void MoleculeInChI::_normalizeMolecule(Molecule& mol)
{
    QS_DEF(Array<int>, ignored);

    ignored.clear_resize(mol.vertexEnd());
    ignored.zerofill();

    for (int i = mol.vertexBegin(); i < mol.vertexEnd(); i = mol.vertexNext(i))
        if (mol.convertableToImplicitHydrogen(i))
            ignored[i] = 1;

    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        if (mol.getBondTopology(i) == TOPOLOGY_RING)
            mol.cis_trans.setParity(i, 0);

    MoleculeAutomorphismSearch of;

    of.detect_invalid_cistrans_bonds = true;
    of.detect_invalid_stereocenters = true;
    of.find_canonical_ordering = false;
    of.ignored_vertices = ignored.ptr();
    of.process(mol);

    for (int i = mol.edgeBegin(); i != mol.edgeEnd(); i = mol.edgeNext(i))
        if (mol.cis_trans.getParity(i) != 0 && of.invalidCisTransBond(i))
            mol.cis_trans.setParity(i, 0);

    for (int i = mol.vertexBegin(); i != mol.vertexEnd(); i = mol.vertexNext(i))
        if (mol.stereocenters.getType(i) > MoleculeStereocenters::ATOM_ANY && of.invalidStereocenter(i))
            mol.stereocenters.remove(i);
}
