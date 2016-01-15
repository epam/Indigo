/****************************************************************************
 * Copyright (C) 2009-2015 EPAM Systems
 * 
 * This file is part of Indigo toolkit.
 * 
 * This file may be distributed and/or modified under the terms of the
 * GNU General Public License version 3 as published by the Free Software
 * Foundation and appearing in the file LICENSE.GPL included in the
 * packaging of this file.
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 ***************************************************************************/

#ifndef __molecule_rgroups_composition__
#define __molecule_rgroups_composition__

#include "molecule/molecule.h"
#include "molecule/molecule_attachments_search.h"

#ifdef _WIN32
#pragma warning(push)
#pragma warning(disable:4251)
#endif

namespace indigo {

class Molecule;

class MoleculeRGroupsComposition {
public:
    MoleculeRGroupsComposition(BaseMolecule &mol) : _mol(mol) {};
    ~MoleculeRGroupsComposition () {};

    Iterator<Attachment*>*   refine();
    BaseMolecule*            decorate(Attachment &at);
    Iterator<BaseMolecule*>* combinations();

    DECL_ERROR;

private:
    BaseMolecule& _mol;
};

}

#ifdef _WIN32
#endif

#endif