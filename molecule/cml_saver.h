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

#ifndef __cml_saver_h__
#define __cml_saver_h__

#include "molecule/base_molecule.h"

class TiXmlDocument;
class TiXmlElement;

namespace indigo {

class Molecule;
class QueryMolecule;
class Output;
class SGroup;

class CmlSaver
{
public:
   explicit CmlSaver (Output &output);

   void saveMolecule (Molecule &mol);
   void saveQueryMolecule (QueryMolecule &mol);
   bool skip_cml_tag; // skips <?xml> and <cml> tags

   DECL_ERROR;

protected:
   void _saveMolecule(BaseMolecule &mol, bool query);
   void _addMoleculeElement (TiXmlElement *elem, BaseMolecule &mol, bool query);
   void _addSgroupElement (TiXmlElement *elem, BaseMolecule &mol, SGroup &sgroup);
   void _addRgroups (TiXmlElement *elem, BaseMolecule &mol, bool query);
   void _addRgroupElement (TiXmlElement *elem, RGroup &rgroup, bool query);

   bool _getRingBondCountFlagValue (QueryMolecule &qmol, int idx, int &value);
   bool _getSubstitutionCountFlagValue (QueryMolecule &qmol, int idx, int &value);
   void _writeOccurrenceRanges (Output &out, const Array<int> &occurrences);

   Output   &_output;

   TiXmlDocument * _doc;
   TiXmlElement * _root;

private:

   CmlSaver (const CmlSaver &); // no implicit copy
};

}

#endif

