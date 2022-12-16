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

#include <map>

#include "molecule/molecule_cdx_loader.h"

#include "base_cpp/scanner.h"
#include "molecule/elements.h"
#include "molecule/molecule.h"
#include "molecule/molecule_scaffold_detection.h"

using namespace indigo;

IMPL_ERROR(MoleculeCdxLoader, "molecule CDX loader");

CP_DEF(MoleculeCdxLoader);

MoleculeCdxLoader::MoleculeCdxLoader(Scanner& scanner)
    : COORD_COEF(1.0f / 1857710.0f), CP_INIT, TL_CP_GET(properties), TL_CP_GET(_nodes), TL_CP_GET(_bonds), TL_CP_GET(_stereo_care_atoms),
      TL_CP_GET(_stereo_care_bonds), TL_CP_GET(_stereocenter_types), TL_CP_GET(_stereocenter_groups), TL_CP_GET(_sensible_bond_directions),
      TL_CP_GET(_ignore_cistrans)
{
    _scanner = &scanner;
    ignore_bad_valence = false;
}

void MoleculeCdxLoader::loadMolecule(Molecule& mol)
{
    mol.clear();
    _nodes.clear();
    _bonds.clear();

    _bmol = &mol;
    _mol = &mol;

    if (_scanner != 0)
    {
        _checkHeader();
        _loadMolecule();
        mol.setIgnoreBadValenceFlag(ignore_bad_valence);
    }
}

void MoleculeCdxLoader::_checkHeader()
{
    long long pos_saved = _scanner->tell();

    if ((_scanner->length() - pos_saved) < 8LL)
        return;

    char id[8];
    _scanner->readCharsFix(8, id);

    if (strncmp(id, kCDX_HeaderString, kCDX_HeaderStringLen) == 0)
    {
        _scanner->seek(kCDX_HeaderLength - kCDX_HeaderStringLen, SEEK_CUR);
    }
    else
    {
        _scanner->seek(pos_saved, SEEK_SET);
    }
}

void MoleculeCdxLoader::_loadMolecule()
{
    UINT16 tag;
    UINT16 size;
    UINT32 id;

    int level = 1;

    while (!_scanner->isEOF())
    {
        tag = _scanner->readBinaryWord();
        if (tag & kCDXTag_Object)
        {
            id = _scanner->readBinaryDword();
            if (tag == kCDXObj_Fragment)
            {
                _readFragment(id);
            }
            else if ((tag == kCDXObj_Graphic) || (tag == kCDXObj_Text) || (tag == kCDXObj_BracketedGroup) || (tag == kCDXObj_BracketAttachment) ||
                     (tag == kCDXObj_CrossingBond) || (tag == kCDXObj_ReactionStep) || (tag == kCDXObj_Curve) || (tag == kCDXObj_EmbeddedObject))
            {
                _skipObject();
            }
            else if ((tag == kCDXObj_Page) || (tag == kCDXObj_Group))
            {
                level++;
            }
            else
            {
                level++;
            }
        }
        else if (tag == 0)
        {
            level--;
        }
        else
        {
            size = _scanner->readBinaryWord();
            switch (tag)
            {
            case kCDXProp_Name:
                _scanner->seek(size, SEEK_CUR);
                break;
            default:
                _scanner->seek(size, SEEK_CUR);
                break;
            }
        }
        if (level == 0)
            break;
    }

    int idx;
    std::map<int, int> _atom_mapping;

    for (int i = 0; i < _nodes.size(); i++)
    {
        if (_nodes[i].type == kCDXNodeType_Element)
        {
            idx = _mol->addAtom(_nodes[i].label);
            _mol->setAtomCharge_Silent(idx, _nodes[i].charge);
            _mol->setAtomIsotope(idx, _nodes[i].isotope);
            _mol->setAtomRadical(idx, _nodes[i].radical);
            //         _mol->setExplicitValence(idx, _nodes[i].valence);
            _bmol->setAtomXyz(idx, (float)_nodes[i].x * COORD_COEF, (float)_nodes[i].y * COORD_COEF, (float)_nodes[i].z * COORD_COEF);
            _nodes[i].index = idx;
            _atom_mapping.emplace(_nodes[i].id, i);
        }
        else if (_nodes[i].type == kCDXNodeType_ExternalConnectionPoint)
        {
            _atom_mapping.emplace(_nodes[i].id, i);
        }
        else
        {
            _atom_mapping.emplace(_nodes[i].id, i);
        }
    }

    for (int i = 0; i < _bonds.size(); i++)
    {
        if ((_nodes[_atom_mapping.at(_bonds[i].beg)].type == kCDXNodeType_Element) && (_nodes[_atom_mapping.at(_bonds[i].end)].type == kCDXNodeType_Element))
        {
            if (_bonds[i].swap_bond)
                _bonds[i].index =
                    _mol->addBond_Silent(_nodes[_atom_mapping.at(_bonds[i].end)].index, _nodes[_atom_mapping.at(_bonds[i].beg)].index, _bonds[i].type);
            else
                _bonds[i].index =
                    _mol->addBond_Silent(_nodes[_atom_mapping.at(_bonds[i].beg)].index, _nodes[_atom_mapping.at(_bonds[i].end)].index, _bonds[i].type);

            if (_bonds[i].dir > 0)
                _bmol->setBondDirection(_bonds[i].index, _bonds[i].dir);
        }
        else if (_nodes[_atom_mapping.at(_bonds[i].beg)].type == kCDXNodeType_ExternalConnectionPoint)
        {
            _updateConnectionPoint(_bonds[i].beg, _bonds[i].end);
        }
        else if (_nodes[_atom_mapping.at(_bonds[i].end)].type == kCDXNodeType_ExternalConnectionPoint)
        {
            _updateConnectionPoint(_bonds[i].end, _bonds[i].beg);
        }
        else if (_nodes[_atom_mapping.at(_bonds[i].beg)].type == kCDXNodeType_Fragment)
        {
            int beg = 0;
            int end = 0;

            for (int j = 0; j < _nodes[_atom_mapping.at(_bonds[i].beg)].connections.size(); j++)
            {
                if (_nodes[_atom_mapping.at(_bonds[i].beg)].connections[j].bond_id == _bonds[i].id)
                {
                    beg = _nodes[_atom_mapping.at(_bonds[i].beg)].connections[j].atom_id;
                    break;
                }
            }
            for (int j = 0; j < _nodes[_atom_mapping.at(_bonds[i].end)].connections.size(); j++)
            {
                if (_nodes[_atom_mapping.at(_bonds[i].end)].connections[j].bond_id == _bonds[i].id)
                {
                    end = _nodes[_atom_mapping.at(_bonds[i].end)].connections[j].atom_id;
                    break;
                }
            }

            if (beg != 0 && end != 0)
            {
                _bonds[i].index = _mol->addBond_Silent(_nodes[_atom_mapping.at(beg)].index, _nodes[_atom_mapping.at(end)].index, _bonds[i].type);
                if (_bonds[i].dir > 0)
                    _bmol->setBondDirection(_bonds[i].index, _bonds[i].dir);
            }
        }
    }

    _postLoad();
}

void MoleculeCdxLoader::_updateConnectionPoint(int point_id, int atom_id)
{
    for (int i = 0; i < _nodes.size(); i++)
    {
        if (_nodes[i].type == kCDXNodeType_Fragment)
        {
            for (int j = 0; j < _nodes[i].connections.size(); j++)
            {
                if (_nodes[i].connections[j].point_id == point_id)
                {
                    _nodes[i].connections[j].atom_id = atom_id;
                    break;
                }
            }
        }
    }
}

void MoleculeCdxLoader::_postLoad()
{
    _sensible_bond_directions.clear_resize(_bonds.size());
    _sensible_bond_directions.zerofill();
    _ignore_cistrans.clear_resize(_bonds.size());
    _ignore_cistrans.zerofill();

    _bmol->buildFromBondsStereocenters(stereochemistry_options, _sensible_bond_directions.ptr());
    _bmol->buildFromBondsAlleneStereo(stereochemistry_options.ignore_errors, _sensible_bond_directions.ptr());
    _bmol->buildCisTrans(_ignore_cistrans.ptr());
    _bmol->have_xyz = true;
}

void MoleculeCdxLoader::_readFragment(UINT32 fragment_id)
{
    UINT16 tag;
    UINT16 size;
    UINT32 id;

    int level = 1;

    while (!_scanner->isEOF())
    {
        tag = _scanner->readBinaryWord();

        if (tag & kCDXTag_Object)
        {
            id = _scanner->readBinaryDword();

            if (tag == kCDXObj_Fragment)
            {
                _readFragment(id);
            }
            else if (tag == kCDXObj_Node)
            {
                _readNode(id);
            }
            else if (tag == kCDXObj_Bond)
            {
                _readBond(id);
            }
            else if ((tag == kCDXObj_Graphic) || (tag == kCDXObj_Text))
            {
                _skipObject();
            }
            else
            {
                _skipObject();
            }
        }
        else if (tag == 0)
        {
            level--;
        }
        else
        {
            size = _scanner->readBinaryWord();
            _NodeDesc* node = 0;
            switch (tag)
            {
            case kCDXProp_Frag_ConnectionOrder:
                node = &_nodes.top();
                _getConnectionOrder(size, node->connections);
                break;
            default:
                _scanner->seek(size, SEEK_CUR);
                break;
            }
        }
        if (level == 0)
            return;
    }
}

void MoleculeCdxLoader::_readNode(UINT32 node_id)
{
    UINT16 tag;
    UINT16 size;
    UINT32 id;

    int level = 1;

    _NodeDesc& node = _nodes.push();
    memset(&node, 0, sizeof(_NodeDesc));
    node.id = node_id;
    node.type = kCDXNodeType_Element;
    node.label = ELEM_C;
    node.hydrogens = -1;
    node.valence = -1;
    node.radical = -1;

    while (!_scanner->isEOF())
    {
        tag = _scanner->readBinaryWord();
        if (tag & kCDXTag_Object)
        {
            id = _scanner->readBinaryDword();
            if (tag == kCDXObj_Fragment)
            {
                _readFragment(id);
            }
            else if (tag == kCDXObj_Group)
            {
            }
            else if ((tag == kCDXObj_Graphic) || (tag == kCDXObj_Text) || (tag == kCDXObj_ObjectTag))
            {
                _skipObject();
            }
            else
            {
                _skipObject();
            }
        }
        else if (tag == 0)
        {
            level--;
        }
        else
        {
            size = _scanner->readBinaryWord();
            switch (tag)
            {
            case kCDXProp_Atom_NumHydrogens:
                node.hydrogens = _scanner->readBinaryWord();
                break;
            case kCDXProp_2DPosition:
                _read2DPosition(node.x, node.y);
                break;
            case kCDXProp_3DPosition:
                _read3DPosition(node.x, node.y, node.z);
                break;
            case kCDXProp_Node_Element:
                node.label = _getElement();
                break;
            case kCDXProp_Atom_Charge:
                node.charge = _getCharge(size);
                break;
            case kCDXProp_Node_Type:
                node.type = _scanner->readBinaryWord();
                break;
            case kCDXProp_Atom_Isotope:
                node.isotope = _scanner->readBinaryWord();
                break;
            case kCDXProp_Atom_Radical:
                node.radical = _getRadical();
                break;
            case kCDXProp_Atom_BondOrdering:
                _getBondOrdering(size, node.connections);
                break;
                //            case kCDXProp_Atom_EnhancedStereoType:
                //              node.enchanced_stereo = _scanner->readByte();
                //            case kCDXProp_Atom_EnhancedStereoGroupNum:
                //              node.stereo_group = _scanner->readBinaryWord();
            case kCDXProp_Atom_CIPStereochemistry:
                node.stereo = _scanner->readByte();
                break;
            case kCDXProp_Atom_ElementList:
            case kCDXProp_Atom_AbnormalValence:
            case kCDXProp_Name:
            case kCDXProp_IgnoreWarnings:
            case kCDXProp_ChemicalWarning:
                _scanner->seek(size, SEEK_CUR);
                break;
            case kCDXProp_MarginWidth:
            case kCDXProp_ZOrder:
            case kCDXProp_Atom_GenericNickname:
            case kCDXProp_Atom_Geometry:
            case kCDXProp_Node_LabelDisplay:
            case kCDXProp_LabelStyle:
            case kCDXProp_ForegroundColor:
            case kCDXProp_BackgroundColor:
                _scanner->seek(size, SEEK_CUR);
                break;
            default:
                _scanner->seek(size, SEEK_CUR);
                break;
            }
        }
        if (level == 0)
        {
            switch (node.type)
            {
            case kCDXNodeType_Fragment:
                break;
            case kCDXNodeType_Element:
                break;
            default:
                break;
            }
            return;
        }
    }
}

void MoleculeCdxLoader::_read2DPosition(int& x, int& y)
{
    y = _scanner->readBinaryDword();
    x = _scanner->readBinaryDword();
}

void MoleculeCdxLoader::_read3DPosition(int& x, int& y, int& z)
{
    z = _scanner->readBinaryDword();
    y = _scanner->readBinaryDword();
    x = _scanner->readBinaryDword();
}

int MoleculeCdxLoader::_getElement()
{
    return _scanner->readBinaryWord();
}

int MoleculeCdxLoader::_getCharge(int size)
{
    if (size == 4)
        return _scanner->readBinaryDword();
    else
        return _scanner->readByte();
}

int MoleculeCdxLoader::_getRadical()
{
    return _scanner->readByte();
}

void MoleculeCdxLoader::_getBondOrdering(int size, Array<_ExtConnection>& connections)
{
    int nbonds = size / sizeof(UINT32);
    connections.clear();

    for (int i = 0; i < nbonds; i++)
    {
        _ExtConnection& conn = connections.push();
        conn.bond_id = _scanner->readBinaryDword();
        conn.point_id = 0;
        conn.atom_id = 0;
    }
}

void MoleculeCdxLoader::_getConnectionOrder(int size, Array<_ExtConnection>& connections)
{
    int npoints = size / sizeof(UINT32);

    if (npoints != connections.size())
    {
        _scanner->seek(size, SEEK_CUR);
        return;
    }

    for (int i = 0; i < npoints; i++)
    {
        connections[i].point_id = _scanner->readBinaryDword();
    }
}

void MoleculeCdxLoader::_readBond(UINT32 bond_id)
{
    UINT16 tag;
    UINT16 size;
    UINT32 id;

    int level = 1;

    _BondDesc& bond = _bonds.push();
    memset(&bond, 0, sizeof(_BondDesc));
    bond.id = bond_id;
    bond.type = BOND_SINGLE;

    while (!_scanner->isEOF())
    {
        tag = _scanner->readBinaryWord();

        if (tag & kCDXTag_Object)
        {
            id = _scanner->readBinaryDword();
            _skipObject();
        }
        else if (tag == 0)
        {
            level--;
        }
        else
        {
            size = _scanner->readBinaryWord();
            switch (tag)
            {
            case kCDXProp_Bond_Begin:
                bond.beg = _scanner->readBinaryDword();
                break;
            case kCDXProp_Bond_End:
                bond.end = _scanner->readBinaryDword();
                break;
            case kCDXProp_Bond_Order:
                bond.type = _getBondType();
                break;
            case kCDXProp_Bond_Display:
                bond.dir = _getBondDirection(bond.swap_bond);
                break;
            case kCDXProp_BondLength:
            case kCDXProp_Bond_CIPStereochemistry:
                bond.stereo = _scanner->readByte();
                break;
            case kCDXProp_Bond_BeginAttach:
            case kCDXProp_Bond_EndAttach:
            case kCDXProp_ChemicalWarning:
                _scanner->seek(size, SEEK_CUR);
                break;
            default:
                _scanner->seek(size, SEEK_CUR);
                break;
            }
        }
        if (level == 0)
        {
            return;
        }
    }
}

int MoleculeCdxLoader::_getBondType()
{
    UINT16 order;
    int type = BOND_SINGLE;

    order = _scanner->readBinaryWord();
    switch (order)
    {
    case kCDXBondOrder_Single:
        type = BOND_SINGLE;
        break;
    case kCDXBondOrder_Double:
        type = BOND_DOUBLE;
        break;
    case kCDXBondOrder_Triple:
        type = BOND_TRIPLE;
        break;
    case kCDXBondOrder_OneHalf:
        type = BOND_AROMATIC;
        break;
    default:
        type = BOND_SINGLE;
        break;
    }
    return type;
}

int MoleculeCdxLoader::_getBondDirection(bool& swap_bond)
{
    UINT16 display;
    int direction = 0;

    display = _scanner->readBinaryWord();
    switch (display)
    {
    case kCDXBondDisplay_WedgedHashBegin:
        direction = BOND_DOWN;
        swap_bond = false;
        break;
    case kCDXBondDisplay_WedgedHashEnd:
        direction = BOND_DOWN;
        swap_bond = true;
        break;
    case kCDXBondDisplay_WedgeBegin:
        direction = BOND_UP;
        swap_bond = false;
        break;
    case kCDXBondDisplay_WedgeEnd:
        direction = BOND_UP;
        swap_bond = true;
        break;
    case kCDXBondDisplay_Wavy:
        direction = BOND_EITHER;
        break;
    default:
        direction = 0;
        break;
    }
    return direction;
}

void MoleculeCdxLoader::_skipObject()
{
    UINT16 tag;
    UINT16 size;
    UINT32 id;

    int level = 1;

    while (!_scanner->isEOF())
    {
        tag = _scanner->readBinaryWord();
        if (tag & kCDXTag_Object)
        {
            id = _scanner->readBinaryDword();
            _skipObject();
        }
        else if (tag == 0)
        {
            level--;
        }
        else
        {
            size = _scanner->readBinaryWord();
            _scanner->seek(size, SEEK_CUR);
        }
        if (level == 0)
            return;
    }
}
