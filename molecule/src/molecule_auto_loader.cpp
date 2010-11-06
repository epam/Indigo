/****************************************************************************
 * Copyright (C) 2009-2010 GGA Software Services LLC
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

#include "molecule/molecule_auto_loader.h"

#include "base_cpp/scanner.h"
#include "molecule/smiles_loader.h"
#include "molecule/molfile_loader.h"
#include "molecule/icm_loader.h"
#include "molecule/molecule.h"
#include "molecule/query_molecule.h"
#include "gzip/gzip_scanner.h"

using namespace indigo;

void MoleculeAutoLoader::_init ()
{
   highlighting = 0;
   ignore_stereocenter_errors = false;
   treat_x_as_pseudoatom = false;
   ignore_closing_bond_direction_mismatch = false;
}

MoleculeAutoLoader::MoleculeAutoLoader (Scanner &scanner)
{
   _scanner = &scanner;
   _own_scanner = false;
   _init();
}

MoleculeAutoLoader::MoleculeAutoLoader (const Array<char> &arr)
{
   _scanner = new BufferScanner(arr);
   _own_scanner = true;
   _init();
}

MoleculeAutoLoader::MoleculeAutoLoader (const char *str)
{
   _scanner = new BufferScanner(str);
   _own_scanner = true;
   _init();
}

MoleculeAutoLoader::~MoleculeAutoLoader ()
{
   if (_own_scanner)
      delete _scanner;
}

void MoleculeAutoLoader::loadMolecule (Molecule &mol)
{
   _loadMolecule(mol, false);
}

void MoleculeAutoLoader::loadQueryMolecule (QueryMolecule &qmol)
{
   _loadMolecule(qmol, true);
}

void MoleculeAutoLoader::_loadMolecule (BaseMolecule &mol, bool query)
{
   if (highlighting != 0)
      highlighting->clear();

   // check for GZip format
   if (!query && _scanner->length() >= 2)
   {
      byte id[2];
      int pos = _scanner->tell();

      _scanner->readCharsFix(2, (char *)id);
      _scanner->seek(pos, SEEK_SET);

      if (id[0] == 0x1f && id[1] == 0x8b)
      {
         GZipScanner gzscanner(*_scanner);
         QS_DEF(Array<char>, buf);

         gzscanner.readAll(buf);
         MoleculeAutoLoader loader2(buf);

         loader2.highlighting = highlighting;
         loader2.ignore_stereocenter_errors = ignore_stereocenter_errors;
         loader2.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
         loader2.loadMolecule((Molecule &)mol);
         return;
      }
   }

   // check for ICM format
   if (!query && _scanner->length() >= 4)
   {
      char id[3];
      int pos = _scanner->tell();

      _scanner->readCharsFix(3, id);
      _scanner->seek(pos, SEEK_SET);
      if (strncmp(id, "ICM", 3) == 0)
      {
         if (query)
            throw Error("cannot load query molecule from ICM format");

         IcmLoader loader(*_scanner);
         loader.loadMolecule((Molecule &)mol);
         return;
      }
   }

   // check for SMILES format
   if (Scanner::isSingleLine(*_scanner))
   {
      SmilesLoader loader(*_scanner);

      loader.ignore_closing_bond_direction_mismatch =
             ignore_closing_bond_direction_mismatch;
      loader.highlighting = highlighting;
      if (query)
         loader.loadQueryMolecule((QueryMolecule &)mol);
      else
         loader.loadMolecule((Molecule &)mol);
   }

   // default is Molfile format
   else
   {
      MolfileLoader loader(*_scanner);
      loader.ignore_stereocenter_errors = ignore_stereocenter_errors;
      loader.treat_x_as_pseudoatom = treat_x_as_pseudoatom;
      loader.highlighting = highlighting;

      if (query)
         loader.loadQueryMolecule((QueryMolecule &)mol);
      else
         loader.loadMolecule((Molecule &)mol);
   }
}
