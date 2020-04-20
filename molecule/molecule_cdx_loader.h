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

#ifndef __molecule_cdx_loader__
#define __molecule_cdx_loader__

#include "base_cpp/exception.h"
#include "base_cpp/obj.h"
#include "molecule/base_molecule.h"
#include "molecule/molecule_stereocenter_options.h"

typedef unsigned short int UINT16;
typedef int INT32;
typedef unsigned int UINT32;
#include "molecule/CDXConstants.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable : 4251)
#endif

namespace indigo
{

    class Scanner;
    class Molecule;

    class DLLEXPORT MoleculeCdxLoader
    {
    public:
        DECL_ERROR;

        explicit MoleculeCdxLoader(Scanner& scanner);

        void loadMolecule(Molecule& mol);

        StereocentersOptions stereochemistry_options;
        bool ignore_bad_valence;

        const float COORD_COEF;

        CP_DECL;
        TL_CP_DECL(RedBlackStringObjMap<Array<char>>, properties);

    protected:
        struct _ExtConnection
        {
            int bond_id;
            int point_id;
            int atom_id;
        };

        struct _NodeDesc
        {
            int id;
            int type;
            int label;
            int isotope;
            int charge;
            int radical;
            int valence;
            int hydrogens;
            int stereo;
            int enchanced_stereo;
            int stereo_group;
            int x;
            int y;
            int z;
            int index;
            Array<_ExtConnection> connections;
        };

        struct _BondDesc
        {
            int id;
            int beg;
            int end;
            int type;
            int stereo;
            int dir;
            int index;
            bool swap_bond;
        };

        Scanner* _scanner;

        TL_CP_DECL(Array<_NodeDesc>, _nodes);
        TL_CP_DECL(Array<_BondDesc>, _bonds);

        TL_CP_DECL(Array<int>, _stereo_care_atoms);
        TL_CP_DECL(Array<int>, _stereo_care_bonds);
        TL_CP_DECL(Array<int>, _stereocenter_types);
        TL_CP_DECL(Array<int>, _stereocenter_groups);
        TL_CP_DECL(Array<int>, _sensible_bond_directions);
        TL_CP_DECL(Array<int>, _ignore_cistrans);

        void _checkHeader();
        void _loadMolecule();
        void _updateConnectionPoint(int point_id, int atom_id);
        void _postLoad();
        void _readFragment(UINT32 fragmment_id);
        void _readNode(UINT32 node_id);
        void _readBond(UINT32 bond_id);
        void _skipObject();
        void _read2DPosition(int& x, int& y);
        void _read3DPosition(int& x, int& y, int& z);
        int _getElement();
        int _getCharge(int size);
        int _getRadical();
        int _getBondType();
        int _getBondDirection(bool& swap_bond);
        void _getBondOrdering(int size, Array<_ExtConnection>& cons);
        void _getConnectionOrder(int size, Array<_ExtConnection>& cons);

        Molecule* _mol;
        BaseMolecule* _bmol;

    private:
        MoleculeCdxLoader(const MoleculeCdxLoader&); // no implicit copy
    };

} // namespace indigo

#ifdef _WIN32
#pragma warning(pop)
#endif

#endif
