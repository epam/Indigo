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

#ifndef __molecule_cml_saver_h__
#define __molecule_cml_saver_h__

class TiXmlDocument;
class TiXmlElement;

namespace indigo {

class Molecule;
class Output;
class SGroup;

class MoleculeCmlSaver
{
public:
   explicit MoleculeCmlSaver (Output &output);

   void saveMolecule (Molecule &mol);
   bool skip_cml_tag; // skips <?xml> and <cml> tags

   DECL_ERROR;

protected:
   void _addSgroupElement (TiXmlElement *elem, SGroup &sgroup);

   Molecule *_mol;
   Output   &_output;

   TiXmlDocument * _doc;
   TiXmlElement * _root;

private:

   MoleculeCmlSaver (const MoleculeCmlSaver &); // no implicit copy
};

}

#endif

